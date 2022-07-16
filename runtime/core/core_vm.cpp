/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core_vm.h"
#include "utils/expected.h"
#include "runtime/compiler.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/thread.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/gc/reference-processor/empty_reference_processor.h"
#include "runtime/mem/refstorage/global_object_storage.h"

namespace panda::core {

// Create MemoryManager by RuntimeOptions
static mem::MemoryManager *CreateMM(const LanguageContext &ctx, mem::InternalAllocatorPtr internal_allocator,
                                    const RuntimeOptions &options)
{
    mem::MemoryManager::HeapOptions heap_options {
        nullptr,                                      // is_object_finalizeble_func
        nullptr,                                      // register_finalize_reference_func
        options.GetMaxGlobalRefSize(),                // max_global_ref_size
        options.IsGlobalReferenceSizeCheckEnabled(),  // is_global_reference_size_check_enabled
        false,                                        // is_single_thread
        options.IsUseTlabForAllocations(),            // is_use_tlab_for_allocations
        options.IsStartAsZygote(),                    // is_start_as_zygote
    };

    mem::GCTriggerConfig gc_trigger_config(options, panda_file::SourceLang::PANDA_ASSEMBLY);

    mem::GCSettings gc_settings(options, panda_file::SourceLang::PANDA_ASSEMBLY);

    mem::GCType gc_type = Runtime::GetGCType(options, panda_file::SourceLang::PANDA_ASSEMBLY);

    return mem::MemoryManager::Create(ctx, internal_allocator, gc_type, gc_settings, gc_trigger_config, heap_options);
}

/* static */
Expected<PandaCoreVM *, PandaString> PandaCoreVM::Create(Runtime *runtime, const RuntimeOptions &options)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    mem::MemoryManager *mm = CreateMM(ctx, runtime->GetInternalAllocator(), options);
    if (mm == nullptr) {
        return Unexpected(PandaString("Cannot create MemoryManager"));
    }

    auto allocator = mm->GetHeapManager()->GetInternalAllocator();
    PandaCoreVM *core_vm = allocator->New<PandaCoreVM>(runtime, options, mm);
    if (core_vm == nullptr) {
        return Unexpected(PandaString("Cannot create PandaCoreVM"));
    }

    core_vm->InitializeGC();

    // Create Main Thread
    core_vm->main_thread_ = MTManagedThread::Create(runtime, core_vm);
    core_vm->main_thread_->InitBuffers();
    ASSERT(core_vm->main_thread_ == ManagedThread::GetCurrent());
    core_vm->main_thread_->InitForStackOverflowCheck(ManagedThread::STACK_OVERFLOW_RESERVED_SIZE,
                                                     ManagedThread::STACK_OVERFLOW_PROTECTED_SIZE);

    core_vm->thread_manager_->SetMainThread(core_vm->main_thread_);

    return core_vm;
}

PandaCoreVM::PandaCoreVM(Runtime *runtime, const RuntimeOptions &options, mem::MemoryManager *mm)
    : runtime_(runtime), mm_(mm)
{
    mem::HeapManager *heap_manager = mm_->GetHeapManager();
    mem::InternalAllocatorPtr allocator = heap_manager->GetInternalAllocator();
    runtime_iface_ = allocator->New<PandaRuntimeInterface>();
    compiler_ = allocator->New<Compiler>(heap_manager->GetCodeAllocator(), allocator, options,
                                         heap_manager->GetMemStats(), runtime_iface_);
    string_table_ = allocator->New<StringTable>();
    monitor_pool_ = allocator->New<MonitorPool>(allocator);
    reference_processor_ = allocator->New<mem::EmptyReferenceProcessor>();
    thread_manager_ = allocator->New<ThreadManager>(allocator);
    rendezvous_ = allocator->New<Rendezvous>();
}

PandaCoreVM::~PandaCoreVM()
{
    delete main_thread_;

    mem::InternalAllocatorPtr allocator = mm_->GetHeapManager()->GetInternalAllocator();
    allocator->Delete(rendezvous_);
    allocator->Delete(runtime_iface_);
    allocator->Delete(thread_manager_);
    allocator->Delete(reference_processor_);
    allocator->Delete(monitor_pool_);
    allocator->Delete(string_table_);
    allocator->Delete(compiler_);
    mm_->Finalize();
    mem::MemoryManager::Destroy(mm_);
}

bool PandaCoreVM::Initialize()
{
    if (!intrinsics::Initialize(panda::panda_file::SourceLang::PANDA_ASSEMBLY)) {
        LOG(ERROR, RUNTIME) << "Failed to initialize Core intrinsics";
        return false;
    }

    auto runtime = Runtime::GetCurrent();
    if (runtime->GetOptions().ShouldLoadBootPandaFiles()) {
        PreAllocOOMErrorObject();
    }

    return true;
}

void PandaCoreVM::PreAllocOOMErrorObject()
{
    auto global_object_storage = GetGlobalObjectStorage();
    auto runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *class_linker = runtime->GetClassLinker();
    auto cls = class_linker->GetExtension(ctx)->GetClass(ctx.GetOutOfMemoryErrorClassDescriptor());
    auto oom_obj = ObjectHeader::Create(cls);
    if (oom_obj == nullptr) {
        LOG(FATAL, RUNTIME) << "Cannot preallocate OOM Error object";
        return;
    }
    oom_obj_ref_ = global_object_storage->Add(oom_obj, panda::mem::Reference::ObjectType::GLOBAL);
}

bool PandaCoreVM::InitializeFinish()
{
    // Preinitialize StackOverflowException so we don't need to do this when stack overflow occurred
    auto runtime = Runtime::GetCurrent();
    auto *class_linker = runtime->GetClassLinker();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *extension = class_linker->GetExtension(ctx);
    class_linker->GetClass(ctx.GetStackOverflowErrorClassDescriptor(), true, extension->GetBootContext());

    return true;
}

void PandaCoreVM::UninitializeThreads()
{
    // Wait until all threads finish the work
    thread_manager_->WaitForDeregistration();
    main_thread_->Destroy();
}

void PandaCoreVM::PreStartup()
{
    mm_->PreStartup();
}

void PandaCoreVM::PreZygoteFork()
{
    mm_->PreZygoteFork();
    compiler_->PreZygoteFork();
}

void PandaCoreVM::PostZygoteFork()
{
    compiler_->PostZygoteFork();
    mm_->PostZygoteFork();
}

void PandaCoreVM::InitializeGC()
{
    mm_->InitializeGC(this);
}

void PandaCoreVM::StartGC()
{
    mm_->StartGC();
}

void PandaCoreVM::StopGC()
{
    mm_->StopGC();
}

void PandaCoreVM::HandleReferences(const GCTask &task, const mem::GC::ReferenceClearPredicateT &pred)
{
    LOG(DEBUG, REF_PROC) << "Start processing cleared references";
    mem::GC *gc = mm_->GetGC();
    gc->ProcessReferences(gc->GetGCPhase(), task, pred);
}

// TODO(alovkov): call ReferenceQueue.add method with cleared references
void PandaCoreVM::HandleEnqueueReferences()
{
    LOG(DEBUG, REF_PROC) << "Start HandleEnqueueReferences";
    mm_->GetGC()->EnqueueReferences();
    LOG(DEBUG, REF_PROC) << "Finish HandleEnqueueReferences";
}

void PandaCoreVM::HandleGCFinished() {}

bool PandaCoreVM::CheckEntrypointSignature(Method *entrypoint)
{
    if (entrypoint->GetNumArgs() == 0) {
        return true;
    }

    if (entrypoint->GetNumArgs() > 1) {
        return false;
    }

    auto *pf = entrypoint->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, entrypoint->GetFileId());
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());

    if (pda.GetArgType(0).GetId() != panda_file::Type::TypeId::REFERENCE) {
        return false;
    }

    auto type_id = pda.GetReferenceType(0);
    auto string_data = pf->GetStringData(type_id);
    const char class_name[] = "[Lpanda/String;";  // NOLINT(modernize-avoid-c-arrays)

    return utf::IsEqual({string_data.data, string_data.utf16_length},
                        {utf::CStringAsMutf8(class_name), sizeof(class_name) - 1});
}

static coretypes::Array *CreateArgumentsArray(const std::vector<std::string> &args, const LanguageContext &ctx,
                                              ClassLinker *class_linker, PandaVM *vm)
{
    const char class_name[] = "[Lpanda/String;";  // NOLINT(modernize-avoid-c-arrays)
    auto *array_klass = class_linker->GetExtension(ctx)->GetClass(utf::CStringAsMutf8(class_name));
    if (array_klass == nullptr) {
        LOG(FATAL, RUNTIME) << "Class " << class_name << " not found";
    }

    auto thread = MTManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    auto *array = coretypes::Array::Create(array_klass, args.size());
    VMHandle<coretypes::Array> array_handle(thread, array);

    for (size_t i = 0; i < args.size(); i++) {
        auto *str = coretypes::String::CreateFromMUtf8(utf::CStringAsMutf8(args[i].data()), args[i].length(), ctx, vm);
        array_handle.GetPtr()->Set(i, str);
    }

    return array_handle.GetPtr();
}

Expected<int, Runtime::Error> PandaCoreVM::InvokeEntrypointImpl(Method *entrypoint,
                                                                const std::vector<std::string> &args)
{
    Runtime *runtime = Runtime::GetCurrent();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(*entrypoint);
    ASSERT(ctx.GetLanguage() == panda_file::SourceLang::PANDA_ASSEMBLY);

    ScopedManagedCodeThread sj(thread);
    ClassLinker *class_linker = runtime->GetClassLinker();
    if (!class_linker->InitializeClass(thread, entrypoint->GetClass())) {
        LOG(ERROR, RUNTIME) << "Cannot initialize class '" << entrypoint->GetClass()->GetName() << "'";
        return Unexpected(Runtime::Error::CLASS_NOT_INITIALIZED);
    }

    ObjectHeader *object_header = nullptr;
    if (entrypoint->GetNumArgs() == 1) {
        coretypes::Array *arg_array = CreateArgumentsArray(args, ctx, runtime_->GetClassLinker(), thread->GetVM());
        object_header = arg_array;
    }

    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<ObjectHeader> args_handle(thread, object_header);
    Value arg_val(args_handle.GetPtr());
    Value v = entrypoint->Invoke(thread, &arg_val);

    return v.GetAs<int>();
}

ObjectHeader *PandaCoreVM::GetOOMErrorObject()
{
    auto global_object_storage = GetGlobalObjectStorage();
    auto obj = global_object_storage->Get(oom_obj_ref_);
    ASSERT(obj != nullptr);
    return obj;
}

void PandaCoreVM::HandleUncaughtException()
{
    ManagedThread *thread = ManagedThread::GetCurrent();
    LOG(ERROR, RUNTIME) << "Unhandled exception: " << thread->GetException()->ClassAddr<Class>()->GetName();
    // _exit guarantees a safe completion in case of multi-threading as static destructors aren't called
    _exit(1);
}

void PandaCoreVM::VisitVmRoots(const GCRootVisitor &visitor)
{
    // Visit PT roots
    GetThreadManager()->EnumerateThreads([visitor](ManagedThread *thread) {
        ASSERT(MTManagedThread::ThreadIsMTManagedThread(thread));
        auto mt_thread = MTManagedThread::CastFromThread(thread);
        auto pt_storage = mt_thread->GetPtReferenceStorage();
        pt_storage->VisitObjects(visitor, mem::RootType::ROOT_PT_LOCAL);
        return true;
    });
}

void PandaCoreVM::UpdateVmRefs()
{
    LOG(DEBUG, GC) << "=== PTRoots Update moved. BEGIN ===";
    GetThreadManager()->EnumerateThreads([](ManagedThread *thread) {
        ASSERT(MTManagedThread::ThreadIsMTManagedThread(thread));
        auto mt_thread = MTManagedThread::CastFromThread(thread);
        auto pt_storage = mt_thread->GetPtReferenceStorage();
        pt_storage->UpdateMovedRefs();
        return true;
    });
    LOG(DEBUG, GC) << "=== PTRoots Update moved. END ===";
}

}  // namespace panda::core
