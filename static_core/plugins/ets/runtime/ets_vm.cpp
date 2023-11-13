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

#include "plugins/ets/runtime/ets_vm.h"
#include <atomic>

#include "compiler/optimizer/ir/runtime_interface.h"
#include "include/runtime.h"
#include "macros.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_runtime_interface.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#include "plugins/ets/runtime/napi/ets_mangle.h"
#include "plugins/ets/runtime/napi/ets_napi_invoke_interface.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/compiler.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/init_icu.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"
#include "runtime/coroutines/threaded_coroutine_manager.h"
#include "plugins/ets/runtime/types/ets_promise.h"

namespace panda::ets {
// Create MemoryManager by RuntimeOptions
static mem::MemoryManager *CreateMM(Runtime *runtime, const RuntimeOptions &options)
{
    mem::MemoryManager::HeapOptions heap_options {
        nullptr,                                      // is_object_finalizeble_func
        nullptr,                                      // register_finalize_reference_func
        options.GetMaxGlobalRefSize(),                // max_global_ref_size
        options.IsGlobalReferenceSizeCheckEnabled(),  // is_global_reference_size_check_enabled
        // NOTE(konstanting, #I67QXC): implement MT_MODE_TASK in allocators and HeapOptions as is_single_thread is not
        // enough
        false,                              // is_single_thread
        options.IsUseTlabForAllocations(),  // is_use_tlab_for_allocations
        options.IsStartAsZygote(),          // is_start_as_zygote
    };

    auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::ETS);
    auto allocator = runtime->GetInternalAllocator();

    mem::GCTriggerConfig gc_trigger_config(options, panda_file::SourceLang::ETS);

    mem::GCSettings gc_settings(options, panda_file::SourceLang::ETS);

    auto gc_type = Runtime::GetGCType(options, panda_file::SourceLang::ETS);

    return mem::MemoryManager::Create(ctx, allocator, gc_type, gc_settings, gc_trigger_config, heap_options);
}

void PandaEtsVM::PromiseListenerInfo::UpdateRefToMovedObject()
{
    if (promise_->IsForwarded()) {
        promise_ = EtsPromise::FromCoreType(panda::mem::GetForwardAddress(promise_));
    }
}

void PandaEtsVM::PromiseListenerInfo::OnPromiseStateChanged(EtsHandle<EtsPromise> &promise)
{
    listener_->OnPromiseStateChanged(promise);
}

/* static */
bool PandaEtsVM::CreateTaskManagerIfNeeded(const RuntimeOptions &options)
{
    auto lang_str = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);
    if (options.GetWorkersType(lang_str) == "taskmanager" && Runtime::GetTaskScheduler() == nullptr) {
        auto *task_scheduler = taskmanager::TaskScheduler::Create(options.GetTaskmanagerWorkersCount(lang_str));
        if (task_scheduler == nullptr) {
            return false;
        }
        Runtime::SetTaskScheduler(task_scheduler);
    }
    return true;
}

/* static */
Expected<PandaEtsVM *, PandaString> PandaEtsVM::Create(Runtime *runtime, const RuntimeOptions &options)
{
    ASSERT(runtime != nullptr);

    if (!PandaEtsVM::CreateTaskManagerIfNeeded(options)) {
        return Unexpected(PandaString("Cannot create TaskManager"));
    }

    auto mm = CreateMM(runtime, options);
    if (mm == nullptr) {
        return Unexpected(PandaString("Cannot create MemoryManager"));
    }

    auto allocator = mm->GetHeapManager()->GetInternalAllocator();
    auto vm = allocator->New<PandaEtsVM>(runtime, options, mm);
    if (vm == nullptr) {
        return Unexpected(PandaString("Cannot create PandaCoreVM"));
    }

    auto class_linker = EtsClassLinker::Create(runtime->GetClassLinker());
    if (!class_linker) {
        allocator->Delete(vm);
        mem::MemoryManager::Destroy(mm);
        return Unexpected(class_linker.Error());
    }
    vm->class_linker_ = std::move(class_linker.Value());

    vm->InitializeGC();

    std::string icu_path = options.GetIcuDataPath();
    if (icu_path == "default") {
        SetIcuDirectory();
    } else {
        u_setDataDirectory(icu_path.c_str());
    }

    CoroutineManagerConfig cfg {
        // emulate_js
        Runtime::GetOptions().IsCoroutineJsMode(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // workers_count
        Runtime::GetOptions().GetCoroutineWorkersCount(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))};
    vm->coroutine_manager_->Initialize(cfg, runtime, vm);

    return vm;
}

bool PandaEtsVM::Destroy(PandaEtsVM *vm)
{
    if (vm == nullptr) {
        return false;
    }

    vm->SaveProfileInfo();
    vm->UninitializeThreads();
    vm->StopGC();

    auto runtime = Runtime::GetCurrent();
    runtime->GetInternalAllocator()->Delete(vm);

    return true;
}

PandaEtsVM::PandaEtsVM(Runtime *runtime, const RuntimeOptions &options, mem::MemoryManager *mm)
    : EtsVM {napi::GetInvokeInterface()}, runtime_(runtime), mm_(mm)
{
    ASSERT(runtime_ != nullptr);
    ASSERT(mm_ != nullptr);

    auto heap_manager = mm_->GetHeapManager();
    auto allocator = heap_manager->GetInternalAllocator();

    runtime_iface_ = allocator->New<EtsRuntimeInterface>();
    compiler_ = allocator->New<Compiler>(heap_manager->GetCodeAllocator(), allocator, options,
                                         heap_manager->GetMemStats(), runtime_iface_);
    string_table_ = allocator->New<StringTable>();
    monitor_pool_ = allocator->New<MonitorPool>(allocator);
    reference_processor_ = allocator->New<mem::ets::EtsReferenceProcessor>(mm_->GetGC());

    auto lang_str = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);
    auto coro_type = options.GetCoroutineImpl(lang_str);
    if (coro_type == "stackful") {
        coroutine_manager_ = allocator->New<StackfulCoroutineManager>(EtsCoroutine::Create<Coroutine>);
    } else {
        coroutine_manager_ = allocator->New<ThreadedCoroutineManager>(EtsCoroutine::Create<Coroutine>);
    }
    rendezvous_ = allocator->New<Rendezvous>(this);

    InitializeRandomEngine();
}

PandaEtsVM::~PandaEtsVM()
{
    auto allocator = mm_->GetHeapManager()->GetInternalAllocator();
    ASSERT(allocator != nullptr);

    allocator->Delete(rendezvous_);
    allocator->Delete(runtime_iface_);
    allocator->Delete(coroutine_manager_);
    allocator->Delete(reference_processor_);
    allocator->Delete(monitor_pool_);
    allocator->Delete(string_table_);
    allocator->Delete(compiler_);

    ASSERT(mm_ != nullptr);
    mm_->Finalize();
    mem::MemoryManager::Destroy(mm_);
}

PandaEtsVM *PandaEtsVM::GetCurrent()
{
    // Use Thread class for possible to use it from native and manage threads
    return static_cast<PandaEtsVM *>(Thread::GetCurrent()->GetVM());
}

bool PandaEtsVM::Initialize()
{
    if (!intrinsics::Initialize(panda::panda_file::SourceLang::ETS)) {
        LOG(ERROR, RUNTIME) << "Failed to initialize eTS intrinsics";
        return false;
    }

    if (!class_linker_->Initialize()) {
        LOG(FATAL, ETS) << "Cannot initialize ets class linker";
    }

    if (Runtime::GetCurrent()->GetOptions().ShouldLoadBootPandaFiles()) {
        EtsClass *cls = class_linker_->GetClass(panda_file_items::class_descriptors::OUT_OF_MEMORY_ERROR.data());
        EtsObject *oom = EtsObject::Create(cls);
        if (oom == nullptr) {
            LOG(FATAL, RUNTIME) << "Cannot preallocate OOM Error object";
            return false;
        }

        oom_obj_ref_ = GetGlobalObjectStorage()->Add(oom->GetCoreType(), panda::mem::Reference::ObjectType::GLOBAL);
    }

    return true;
}

bool PandaEtsVM::InitializeFinish()
{
    if (Runtime::GetOptions().ShouldLoadBootPandaFiles()) {
        // Preinitialize StackOverflowError, so we don't need to do this when stack overflow occurred
        EtsClass *cls = class_linker_->GetClass(panda_file_items::class_descriptors::STACK_OVERFLOW_ERROR.data());
        if (cls == nullptr) {
            LOG(FATAL, ETS) << "Cannot preinitialize StackOverflowError";
            return false;
        }
    }
    return true;
}

void PandaEtsVM::UninitializeThreads()
{
    // Wait until all threads finish the work
    coroutine_manager_->WaitForDeregistration();
    coroutine_manager_->DestroyMainCoroutine();
    coroutine_manager_->Finalize();
}

void PandaEtsVM::PreStartup()
{
    ASSERT(mm_ != nullptr);

    mm_->PreStartup();
}

void PandaEtsVM::PreZygoteFork()
{
    ASSERT(mm_ != nullptr);
    ASSERT(compiler_ != nullptr);

    mm_->PreZygoteFork();
    compiler_->PreZygoteFork();
}

void PandaEtsVM::PostZygoteFork()
{
    ASSERT(compiler_ != nullptr);
    ASSERT(mm_ != nullptr);

    compiler_->PostZygoteFork();
    mm_->PostZygoteFork();
}

void PandaEtsVM::InitializeGC()
{
    ASSERT(mm_ != nullptr);

    mm_->InitializeGC(this);
}

void PandaEtsVM::StartGC()
{
    ASSERT(mm_ != nullptr);

    mm_->StartGC();
}

void PandaEtsVM::StopGC()
{
    ASSERT(mm_ != nullptr);

    if (GetGC()->IsGCRunning()) {
        mm_->StopGC();
    }
}

void PandaEtsVM::HandleReferences(const GCTask &task, const mem::GC::ReferenceClearPredicateT &pred)
{
    ASSERT(mm_ != nullptr);

    auto gc = mm_->GetGC();
    ASSERT(gc != nullptr);

    LOG(DEBUG, REF_PROC) << "Start processing cleared references";
    gc->ProcessReferences(gc->GetGCPhase(), task, pred);
    LOG(DEBUG, REF_PROC) << "Finish processing cleared references";

    GetGlobalObjectStorage()->ClearUnmarkedWeakRefs(gc, pred);
}

void PandaEtsVM::HandleGCRoutineInMutator()
{
    // Handle references only in coroutine
    ASSERT(Coroutine::GetCurrent() != nullptr);
    ASSERT(GetMutatorLock()->HasLock());
    auto coroutine = Coroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> handle_scope(coroutine);
    os::memory::LockHolder lock(finalization_queue_lock_);
    if (!registered_finalization_queue_instances_.empty()) {
        EtsClass *finalization_queue_class = registered_finalization_queue_instances_.front()->GetClass();
        EtsMethod *cleanup = finalization_queue_class->GetMethod("cleanup");
        for (auto *entry : registered_finalization_queue_instances_) {
            VMHandle<ObjectHeader> handle(coroutine, entry->GetCoreType());
            Value arg(handle.GetPtr());
            cleanup->GetPandaMethod()->Invoke(coroutine, &arg);
            ASSERT(!coroutine->HasPendingException());
        }
    }
}

void PandaEtsVM::HandleGCFinished() {}

bool PandaEtsVM::CheckEntrypointSignature(Method *entrypoint)
{
    ASSERT(entrypoint != nullptr);

    if (entrypoint->GetNumArgs() == 0) {
        return true;
    }

    if (entrypoint->GetNumArgs() > 1) {
        return false;
    }

    auto *pf = entrypoint->GetPandaFile();
    ASSERT(pf != nullptr);
    panda_file::MethodDataAccessor mda(*pf, entrypoint->GetFileId());
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());

    if (pda.GetArgType(0).GetId() != panda_file::Type::TypeId::REFERENCE) {
        return false;
    }

    auto name = pf->GetStringData(pda.GetReferenceType(0));
    std::string_view expected_name(panda_file_items::class_descriptors::STRING_ARRAY);

    return utf::IsEqual({name.data, name.utf16_length},
                        {utf::CStringAsMutf8(expected_name.data()), expected_name.length()});
}

static EtsObjectArray *CreateArgumentsArray(const std::vector<std::string> &args, PandaEtsVM *ets_vm)
{
    ASSERT(ets_vm != nullptr);

    const char *class_descripor = panda_file_items::class_descriptors::STRING_ARRAY.data();
    EtsClass *array_klass = ets_vm->GetClassLinker()->GetClass(class_descripor);
    if (array_klass == nullptr) {
        LOG(FATAL, RUNTIME) << "Class " << class_descripor << " not found";
        return nullptr;
    }

    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsObjectArray *ets_array = EtsObjectArray::Create(array_klass, args.size());
    EtsHandle<EtsObjectArray> array_handle(coroutine, ets_array);

    for (size_t i = 0; i < args.size(); i++) {
        EtsString *str = EtsString::CreateFromMUtf8(args[i].data(), args[i].length());
        array_handle.GetPtr()->Set(i, str->AsObject());
    }

    return array_handle.GetPtr();
}

coretypes::String *PandaEtsVM::CreateString(Method *ctor, ObjectHeader *obj)
{
    EtsString *str = nullptr;
    ASSERT(ctor->GetNumArgs() > 0);  // must be at list this argument
    if (ctor->GetNumArgs() == 1) {
        str = EtsString::CreateNewEmptyString();
    } else if (ctor->GetNumArgs() == 2) {
        ASSERT(ctor->GetArgType(1).GetId() == panda_file::Type::TypeId::REFERENCE);
        auto *str_data = utf::Mutf8AsCString(ctor->GetRefArgType(1).data);
        if (std::strcmp("[C", str_data) == 0) {
            auto *array = reinterpret_cast<EtsArray *>(obj);
            str = EtsString::CreateNewStringFromChars(0, array->GetLength(), array);
        } else if ((std::strcmp("Lstd/core/String;", str_data) == 0)) {
            str = EtsString::CreateNewStringFromString(reinterpret_cast<EtsString *>(obj));
        } else {
            LOG(FATAL, ETS) << "Non-existent ctor";
        }
    } else {
        LOG(FATAL, ETS) << "Must be 1 or 2 ctor args";
    }
    return str->GetCoreType();
}

Expected<int, Runtime::Error> PandaEtsVM::InvokeEntrypointImpl(Method *entrypoint, const std::vector<std::string> &args)
{
    ASSERT(Runtime::GetCurrent()->GetLanguageContext(*entrypoint).GetLanguage() == panda_file::SourceLang::ETS);

    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);

    ScopedManagedCodeThread sj(coroutine);
    if (!class_linker_->InitializeClass(coroutine, EtsClass::FromRuntimeClass(entrypoint->GetClass()))) {
        LOG(ERROR, RUNTIME) << "Cannot initialize class '" << entrypoint->GetClass()->GetName() << "'";
        return Unexpected(Runtime::Error::CLASS_NOT_INITIALIZED);
    }

    [[maybe_unused]] EtsHandleScope scope(coroutine);
    if (entrypoint->GetNumArgs() == 0) {
        auto v = entrypoint->Invoke(coroutine, nullptr);
        return v.GetAs<int>();
    }

    if (entrypoint->GetNumArgs() == 1) {
        EtsObjectArray *ets_object_array = CreateArgumentsArray(args, this);
        EtsHandle<EtsObjectArray> args_handle(coroutine, ets_object_array);
        Value arg_val(args_handle.GetPtr()->AsObject()->GetCoreType());
        auto v = entrypoint->Invoke(coroutine, &arg_val);

        return v.GetAs<int>();
    }

    // What if entrypoint->GetNumArgs() > 1 ?
    LOG(ERROR, RUNTIME) << "ets entrypoint has args count more than 1 : " << entrypoint->GetNumArgs();
    return Unexpected(Runtime::Error::INVALID_ENTRY_POINT);
}

ObjectHeader *PandaEtsVM::GetOOMErrorObject()
{
    auto obj = GetGlobalObjectStorage()->Get(oom_obj_ref_);
    ASSERT(obj != nullptr);
    return obj;
}

bool PandaEtsVM::LoadNativeLibrary(EtsEnv *env, const PandaString &name)
{
    ASSERT_PRINT(Coroutine::GetCurrent()->IsInNativeCode(), "LoadNativeLibrary must be called at native");

    if (auto error = native_library_provider_.LoadLibrary(env, name)) {
        LOG(ERROR, RUNTIME) << "Cannot load library " << name << ": " << error.value();
        return false;
    }

    return true;
}

void PandaEtsVM::ResolveNativeMethod(Method *method)
{
    ASSERT_PRINT(method->IsNative(), "Method should be native");
    std::string class_name = utf::Mutf8AsCString(method->GetClassName().data);
    std::string method_name = utf::Mutf8AsCString(method->GetName().data);
    std::string name = MangleMethodName(class_name, method_name);
    auto ptr = native_library_provider_.ResolveSymbol(PandaString(name));
    if (ptr == nullptr) {
        std::string signature;
        signature.append(EtsMethod::FromRuntimeMethod(method)->GetMethodSignature(false));
        name = MangleMethodNameWithSignature(name, signature);
        ptr = native_library_provider_.ResolveSymbol(PandaString(name));
        if (ptr == nullptr) {
            PandaStringStream ss;
            ss << "No implementation found for " << method->GetFullName() << ", tried " << name;

            auto coroutine = EtsCoroutine::GetCurrent();
            ThrowEtsException(coroutine, panda_file_items::class_descriptors::NO_SUCH_METHOD_ERROR, ss.str());
            return;
        }
    }

    method->SetNativePointer(ptr);
}

void PandaEtsVM::HandleUncaughtException()
{
    auto current_coro = EtsCoroutine::GetCurrent();
    ASSERT(current_coro != nullptr);
    ScopedManagedCodeThread sj(current_coro);
    [[maybe_unused]] EtsHandleScope scope(current_coro);

    EtsHandle<EtsObject> exception(current_coro, EtsObject::FromCoreType(current_coro->GetException()));

    PandaStringStream log_stream;
    log_stream << "Unhandled exception: " << exception->GetCoreType()->ClassAddr<Class>()->GetName();

    auto *exception_ets_class = exception->GetClass();
    if ((exception->GetCoreType() == current_coro->GetVM()->GetOOMErrorObject()) ||
        (GetClassLinker()->GetClass(panda_file_items::class_descriptors::STACK_OVERFLOW_ERROR.data()) ==
         exception_ets_class)) {
        LOG(ERROR, RUNTIME) << log_stream.str();
        // can't do anything more useful in case of OOM or StackOF
        // _exit guarantees a safe completion in case of multi-threading as static destructors aren't called
        _exit(1);
    }

    // We need to call Exception::toString() method and write the result to stdout.
    // NOTE(molotkovmikhail,#I7AJKF): use NAPI to describe the exception and print the stacktrace
    auto *method_to_execute = EtsMethod::ToRuntimeMethod(exception_ets_class->GetMethod("toString"));
    Value args(exception->GetCoreType());  // only 'this' is required as an argument
    current_coro->ClearException();        // required for Invoke()
    auto method_res = method_to_execute->Invoke(current_coro, &args);
    auto *str_result = coretypes::String::Cast(method_res.GetAs<ObjectHeader *>());
    if (str_result == nullptr) {
        LOG(ERROR, RUNTIME) << log_stream.str();
        // _exit guarantees a safe completion in case of multi-threading as static destructors aren't called
        _exit(1);
    }
    log_stream << "\n" << ConvertToString(str_result);
    LOG(ERROR, RUNTIME) << log_stream.str();
    // _exit guarantees a safe completion in case of multi-threading as static destructors aren't called
    _exit(1);
}

void PandaEtsVM::SweepVmRefs(const GCObjectVisitor &gc_object_visitor)
{
    PandaVM::SweepVmRefs(gc_object_visitor);
    {
        os::memory::LockHolder lock(finalization_queue_lock_);
        auto it = registered_finalization_queue_instances_.begin();
        while (it != registered_finalization_queue_instances_.end()) {
            if (gc_object_visitor((*it)->GetCoreType()) == ObjectStatus::DEAD_OBJECT) {
                auto to_remove = it++;
                registered_finalization_queue_instances_.erase(to_remove);
            } else {
                ++it;
            }
        }
    }
    {
        os::memory::LockHolder lock(promise_listeners_lock_);
        auto it = promise_listeners_.begin();
        while (it != promise_listeners_.end()) {
            if (gc_object_visitor(it->GetPromise()) == ObjectStatus::DEAD_OBJECT) {
                auto to_remove = it++;
                promise_listeners_.erase(to_remove);
            } else {
                ++it;
            }
        }
    }
}

void PandaEtsVM::VisitVmRoots(const GCRootVisitor &visitor)
{
    GetThreadManager()->EnumerateThreads([visitor](ManagedThread *thread) {
        const auto coroutine = EtsCoroutine::CastFromThread(thread);
        if (auto ets_napi_env = coroutine->GetEtsNapiEnv()) {
            auto ets_storage = ets_napi_env->GetEtsReferenceStorage();
            ets_storage->GetAsReferenceStorage()->VisitObjects(visitor, mem::RootType::ROOT_NATIVE_LOCAL);
        }
        if (!coroutine->HasManagedEntrypoint()) {
            return true;
        }
        const PandaVector<Value> &arguments = coroutine->GetManagedEntrypointArguments();
        if (!arguments.empty()) {
            // arguments may be empty in the following cases:
            // 1. The entrypoint is static and doesn't accept any arguments
            // 2. The coroutine is launched.
            // 3. The entrypoint is the main method
            Method *entrypoint = coroutine->GetManagedEntrypoint();
            panda_file::ShortyIterator it(entrypoint->GetShorty());
            size_t arg_idx = 0;
            ++it;  // skip return type
            if (!entrypoint->IsStatic()) {
                // handle 'this' argument
                ASSERT(arguments[arg_idx].IsReference());
                ObjectHeader *arg = arguments[arg_idx].GetAs<ObjectHeader *>();
                ASSERT(arg != nullptr);
                visitor(mem::GCRoot(mem::RootType::ROOT_THREAD, arg));
                ++arg_idx;
            }
            while (it != panda_file::ShortyIterator()) {
                if ((*it).GetId() == panda_file::Type::TypeId::REFERENCE) {
                    ASSERT(arguments[arg_idx].IsReference());
                    ObjectHeader *arg = arguments[arg_idx].GetAs<ObjectHeader *>();
                    if (arg != nullptr) {
                        visitor(mem::GCRoot(mem::RootType::ROOT_THREAD, arg));
                    }
                }
                ++it;
                ++arg_idx;
            }
        }
        return true;
    });
}

template <bool REF_CAN_BE_NULL>
void PandaEtsVM::UpdateMovedVmRef(Value &ref)
{
    ASSERT(ref.IsReference());
    ObjectHeader *arg = ref.GetAs<ObjectHeader *>();
    if constexpr (REF_CAN_BE_NULL) {
        if (arg == nullptr) {
            return;
        }
    } else {
        ASSERT(arg != nullptr);
    }
    if (arg->IsForwarded()) {
        ObjectHeader *forward_address = mem::GetForwardAddress(arg);
        ref = Value(forward_address);
        LOG(DEBUG, GC) << "Forward root object: " << arg << " -> " << forward_address;
    }
}

void PandaEtsVM::UpdateVmRefs()
{
    GetThreadManager()->EnumerateThreads([](ManagedThread *thread) {
        auto coroutine = EtsCoroutine::CastFromThread(thread);
        if (auto ets_napi_env = coroutine->GetEtsNapiEnv()) {
            auto ets_storage = ets_napi_env->GetEtsReferenceStorage();
            ets_storage->GetAsReferenceStorage()->UpdateMovedRefs();
        }
        if (!coroutine->HasManagedEntrypoint()) {
            return true;
        }
        PandaVector<Value> &arguments = coroutine->GetManagedEntrypointArguments();
        if (!arguments.empty()) {
            // arguments may be empty in the following cases:
            // 1. The entrypoint is static and doesn't accept any arguments
            // 2. The coroutine is launched.
            // 3. The entrypoint is the main method
            Method *entrypoint = coroutine->GetManagedEntrypoint();
            panda_file::ShortyIterator it(entrypoint->GetShorty());
            size_t arg_idx = 0;
            ++it;  // skip return type
            if (!entrypoint->IsStatic()) {
                // handle 'this' argument
                UpdateMovedVmRef<false>(arguments[arg_idx]);
                ++arg_idx;
            }
            while (it != panda_file::ShortyIterator()) {
                if ((*it).GetId() == panda_file::Type::TypeId::REFERENCE) {
                    UpdateMovedVmRef<true>(arguments[arg_idx]);
                }
                ++it;
                ++arg_idx;
            }
        }
        return true;
    });

    {
        os::memory::LockHolder lock(finalization_queue_lock_);
        for (auto &entry : registered_finalization_queue_instances_) {
            if ((entry->GetCoreType())->IsForwarded()) {
                entry = EtsObject::FromCoreType(panda::mem::GetForwardAddress(entry->GetCoreType()));
            }
        }
    }
    {
        os::memory::LockHolder lock(promise_listeners_lock_);
        std::for_each(promise_listeners_.begin(), promise_listeners_.end(),
                      [](auto &entry) { entry.UpdateRefToMovedObject(); });
    }
}

static mem::Reference *EtsNapiObjectToGlobalReference(ets_object global_ref)
{
    ASSERT(global_ref != nullptr);
    auto *ref = reinterpret_cast<mem::Reference *>(global_ref);
    return ref;
}

void PandaEtsVM::DeleteGlobalRef(ets_object global_ref)
{
    auto *ref = EtsNapiObjectToGlobalReference(global_ref);
    if (!ref->IsGlobal()) {
        LOG(FATAL, ETS_NAPI) << "Try to remote non-global ref: " << std::hex << ref;
        return;
    }
    GetGlobalObjectStorage()->Remove(ref);
}

static mem::Reference *EtsNapiObjectToWeakReference(ets_weak weak_ref)
{
    ASSERT(weak_ref != nullptr);
    auto *ref = reinterpret_cast<mem::Reference *>(weak_ref);
    return ref;
}

void PandaEtsVM::DeleteWeakGlobalRef(ets_weak weak_ref)
{
    auto *ref = EtsNapiObjectToWeakReference(weak_ref);
    if (!ref->IsWeak()) {
        LOG(FATAL, ETS_NAPI) << "Try to remote non-weak ref: " << std::hex << ref;
        return;
    }
    GetGlobalObjectStorage()->Remove(ref);
}

void PandaEtsVM::RegisterFinalizationQueueInstance(EtsObject *instance)
{
    os::memory::LockHolder lock(finalization_queue_lock_);
    registered_finalization_queue_instances_.push_back(instance);
}

/* static */
void PandaEtsVM::Abort(const char *message /* = nullptr */)
{
    Runtime::Abort(message);
}

void PandaEtsVM::AddPromiseListener(EtsPromise *promise, PandaUniquePtr<PromiseListener> &&listener)
{
    os::memory::LockHolder lock(promise_listeners_lock_);
    promise_listeners_.emplace_back(PromiseListenerInfo(promise, std::move(listener)));
}

void PandaEtsVM::FirePromiseStateChanged(EtsHandle<EtsPromise> &promise)
{
    os::memory::LockHolder lock(promise_listeners_lock_);
    auto it = promise_listeners_.begin();
    while (it != promise_listeners_.end()) {
        if (it->GetPromise() == promise.GetPtr()) {
            // In stackful coroutine implementation (with JS mode enabled), the OnPromiseStateChanged call
            // might trigger a PandaEtsVM::AddPromiseListener call within the same thread, causing a
            // double acquire of promise_listeners_lock_ and hence an assertion failure
            // NOTE(konstanting, I67QXC): handle this situation
            it->OnPromiseStateChanged(promise);
            auto to_remove = it++;
            promise_listeners_.erase(to_remove);
        } else {
            ++it;
        }
    }
}

}  // namespace panda::ets
