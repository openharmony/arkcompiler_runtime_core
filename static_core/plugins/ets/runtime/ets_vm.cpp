/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "include/mem/panda_string.h"
#include "libpandabase/macros.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_runtime_interface.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#include "plugins/ets/runtime/napi/ets_mangle.h"
#include "plugins/ets/runtime/napi/ets_napi_invoke_interface.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/compiler.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/init_icu.h"
#include "runtime/coroutines/stackful_coroutine_manager.h"
#include "runtime/coroutines/threaded_coroutine_manager.h"
#include "runtime/mem/lock_config_helper.h"
#include "plugins/ets/stdlib/native/init_native_methods.h"
#include "plugins/ets/runtime/types/ets_finalizable_weak_ref_list.h"

#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"

namespace ark::ets {
// Create MemoryManager by RuntimeOptions
static mem::MemoryManager *CreateMM(Runtime *runtime, const RuntimeOptions &options)
{
    mem::MemoryManager::HeapOptions heapOptions {
        nullptr,                                      // is_object_finalizeble_func
        nullptr,                                      // register_finalize_reference_func
        options.GetMaxGlobalRefSize(),                // max_global_ref_size
        options.IsGlobalReferenceSizeCheckEnabled(),  // is_global_reference_size_check_enabled
        MT_MODE_TASK,                                 // multithreading mode
        options.IsUseTlabForAllocations(),            // is_use_tlab_for_allocations
        options.IsStartAsZygote(),                    // is_start_as_zygote
    };

    auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::ETS);
    auto allocator = runtime->GetInternalAllocator();

    mem::GCTriggerConfig gcTriggerConfig(options, panda_file::SourceLang::ETS);

    mem::GCSettings gcSettings(options, panda_file::SourceLang::ETS);

    auto gcType = Runtime::GetGCType(options, panda_file::SourceLang::ETS);

    return mem::MemoryManager::Create(ctx, allocator, gcType, gcSettings, gcTriggerConfig, heapOptions);
}

/* static */
bool PandaEtsVM::CreateTaskManagerIfNeeded(const RuntimeOptions &options)
{
    if (options.GetWorkersType() == "taskmanager" && Runtime::GetTaskScheduler() == nullptr) {
        auto *taskScheduler = taskmanager::TaskScheduler::Create(
            options.GetTaskmanagerWorkersCount(), taskmanager::StringToTaskTimeStats(options.GetTaskStatsType()));
        if (taskScheduler == nullptr) {
            return false;
        }
        Runtime::SetTaskScheduler(taskScheduler);
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

    auto classLinker = EtsClassLinker::Create(runtime->GetClassLinker());
    if (!classLinker) {
        allocator->Delete(vm);
        mem::MemoryManager::Destroy(mm);
        return Unexpected(classLinker.Error());
    }
    vm->classLinker_ = std::move(classLinker.Value());

    vm->InitializeGC();

    std::string icuPath = options.GetIcuDataPath();
    if (icuPath == "default") {
        SetIcuDirectory();
    } else {
        u_setDataDirectory(icuPath.c_str());
    }

    CoroutineManagerConfig cfg {
        // emulate_js
        Runtime::GetOptions().IsCoroutineJsMode(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // workers_count
        Runtime::GetOptions().GetCoroutineWorkersCount(plugins::LangToRuntimeType(panda_file::SourceLang::ETS)),
        // enable perf stats
        Runtime::GetOptions().IsCoroutineDumpStats(plugins::LangToRuntimeType(panda_file::SourceLang::ETS))};
    vm->coroutineManager_->Initialize(cfg, runtime, vm);

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

    auto heapManager = mm_->GetHeapManager();
    auto allocator = heapManager->GetInternalAllocator();

    runtimeIface_ = allocator->New<EtsRuntimeInterface>();
    compiler_ = allocator->New<Compiler>(heapManager->GetCodeAllocator(), allocator, options,
                                         heapManager->GetMemStats(), runtimeIface_);
    stringTable_ = allocator->New<StringTable>();
    monitorPool_ = allocator->New<MonitorPool>(allocator);
    referenceProcessor_ = allocator->New<mem::ets::EtsReferenceProcessor>(mm_->GetGC());

    auto langStr = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);
    auto coroType = options.GetCoroutineImpl(langStr);
    if (coroType == "stackful") {
        coroutineManager_ = allocator->New<StackfulCoroutineManager>(EtsCoroutine::Create<Coroutine>);
    } else {
        coroutineManager_ = allocator->New<ThreadedCoroutineManager>(EtsCoroutine::Create<Coroutine>);
    }
    rendezvous_ = allocator->New<Rendezvous>(this);
    taskpool_ = allocator->New<Taskpool>();

    InitializeRandomEngine();
}

PandaEtsVM::~PandaEtsVM()
{
    auto allocator = mm_->GetHeapManager()->GetInternalAllocator();
    ASSERT(allocator != nullptr);

    allocator->Delete(taskpool_);
    allocator->Delete(rendezvous_);
    allocator->Delete(runtimeIface_);
    allocator->Delete(coroutineManager_);
    allocator->Delete(referenceProcessor_);
    allocator->Delete(monitorPool_);
    allocator->Delete(stringTable_);
    allocator->Delete(compiler_);

    if (destroyExternalData_) {
        destroyExternalData_(&externalData_);
    }

    ASSERT(mm_ != nullptr);
    mm_->Finalize();
    mem::MemoryManager::Destroy(mm_);
}

PandaEtsVM *PandaEtsVM::GetCurrent()
{
    // Use Thread class for possible to use it from native and manage threads
    return static_cast<PandaEtsVM *>(Thread::GetCurrent()->GetVM());
}

static void PreallocSpecialReference(PandaEtsVM *vm, mem::Reference *&ref, const char *desc, bool nonMovable = false)
{
    EtsClass *cls = vm->GetClassLinker()->GetClass(desc);
    if (cls == nullptr) {
        LOG(FATAL, RUNTIME) << "Cannot find a class for special object " << desc;
    }
    EtsObject *obj = nonMovable ? EtsObject::CreateNonMovable(cls) : EtsObject::Create(cls);
    if (obj == nullptr) {
        LOG(FATAL, RUNTIME) << "Cannot preallocate a special object " << desc;
    }
    ref = vm->GetGlobalObjectStorage()->Add(obj->GetCoreType(), ark::mem::Reference::ObjectType::GLOBAL);
}

bool PandaEtsVM::Initialize()
{
    if (!ark::intrinsics::Initialize(ark::panda_file::SourceLang::ETS)) {
        LOG(ERROR, RUNTIME) << "Failed to initialize eTS intrinsics";
        return false;
    }

    if (!classLinker_->Initialize()) {
        LOG(FATAL, ETS) << "Cannot initialize ets class linker";
    }
    classLinker_->GetEtsClassLinkerExtension()->InitializeBuiltinClasses();

    if (Runtime::GetOptions().ShouldLoadBootPandaFiles()) {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace panda_file_items::class_descriptors;

        PreallocSpecialReference(this, oomObjRef_, OUT_OF_MEMORY_ERROR.data());
        PreallocSpecialReference(this, undefinedObjRef_, INTERNAL_UNDEFINED.data(), true);
        PreallocSpecialReference(this, finalizableWeakRefList_, FINALIZABLE_WEAK_REF.data());

        if (Thread::GetCurrent() != nullptr) {
            ASSERT(GetThreadManager()->GetMainThread() == Thread::GetCurrent());
            auto *coro = EtsCoroutine::GetCurrent();
            coro->SetUndefinedObject(GetUndefinedObject());

            doubleToStringCache_ = DoubleToStringCache::Create(coro);
            floatToStringCache_ = FloatToStringCache::Create(coro);
            longToStringCache_ = LongToStringCache::Create(coro);
        }
        referenceProcessor_->Initialize();
    }
    [[maybe_unused]] bool cachesCreated =
        (doubleToStringCache_ != nullptr && floatToStringCache_ != nullptr && longToStringCache_ != nullptr);
    LOG_IF(!cachesCreated, WARNING, ETS) << "Cannot initialize number-to-string caches";
    LOG_IF(cachesCreated, DEBUG, ETS) << "Initialized number-to-string caches";

    // Check if Intrinsics/native methods should be initialized, we don't want to attempt to
    // initialize  native methods in certain scenarios where we don't have ets stdlib at our disposal
    if (Runtime::GetOptions().ShouldInitializeIntrinsics()) {
        // NOTE(ksarychev, #18135): Implement napi module registration via loading a separate
        // library
        EtsEnv *env = EtsCoroutine::GetCurrent()->GetEtsNapiEnv();
        ark::ets::stdlib::InitNativeMethods(env);
    }

    const auto lang = plugins::LangToRuntimeType(panda_file::SourceLang::ETS);
    for (const auto &path : Runtime::GetOptions().GetNativeLibraryPath(lang)) {
        nativeLibraryProvider_.AddLibraryPath(ConvertToString(path));
    }

    return true;
}

bool PandaEtsVM::InitializeFinish()
{
    if (Runtime::GetOptions().ShouldLoadBootPandaFiles()) {
        // Preinitialize StackOverflowError, so we don't need to do this when stack overflow occurred
        EtsClass *cls = classLinker_->GetClass(panda_file_items::class_descriptors::STACK_OVERFLOW_ERROR.data());
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
    coroutineManager_->WaitForDeregistration();
    coroutineManager_->DestroyMainCoroutine();
    coroutineManager_->Finalize();
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
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> handleScope(coroutine);
    os::memory::LockHolder lock(finalizationRegistryLock_);
    if (!registeredFinalizationRegistryInstances_.empty()) {
        EtsClass *finalizationRegistryClass = registeredFinalizationRegistryInstances_.front()->GetClass();
        EtsMethod *cleanup = finalizationRegistryClass->GetMethod("cleanup");
        for (auto *entry : registeredFinalizationRegistryInstances_) {
            VMHandle<ObjectHeader> handle(coroutine, entry->GetCoreType());
            Value arg(handle.GetPtr());
            finalizationRegistryLock_.Unlock();
            cleanup->GetPandaMethod()->Invoke(coroutine, &arg);
            finalizationRegistryLock_.Lock();
            ASSERT(!coroutine->HasPendingException());
        }
    }
    coroutine->GetPandaVM()->CleanFinalizableReferenceList();
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
    std::string_view expectedName(panda_file_items::class_descriptors::STRING_ARRAY);

    return utf::IsEqual({name.data, name.utf16Length},
                        {utf::CStringAsMutf8(expectedName.data()), expectedName.length()});
}

static EtsObjectArray *CreateArgumentsArray(const std::vector<std::string> &args, PandaEtsVM *etsVm)
{
    ASSERT(etsVm != nullptr);

    const char *classDescripor = panda_file_items::class_descriptors::STRING_ARRAY.data();
    EtsClass *arrayKlass = etsVm->GetClassLinker()->GetClass(classDescripor);
    if (arrayKlass == nullptr) {
        LOG(FATAL, RUNTIME) << "Class " << classDescripor << " not found";
        return nullptr;
    }

    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsObjectArray *etsArray = EtsObjectArray::Create(arrayKlass, args.size());
    EtsHandle<EtsObjectArray> arrayHandle(coroutine, etsArray);

    for (size_t i = 0; i < args.size(); i++) {
        EtsString *str = EtsString::CreateFromMUtf8(args[i].data(), args[i].length());
        arrayHandle.GetPtr()->Set(i, str->AsObject());
    }

    return arrayHandle.GetPtr();
}

coretypes::String *PandaEtsVM::CreateString(Method *ctor, ObjectHeader *obj)
{
    EtsString *str = nullptr;
    ASSERT(ctor->GetNumArgs() > 0);  // must be at list this argument
    if (ctor->GetNumArgs() == 1) {
        str = EtsString::CreateNewEmptyString();
    } else if (ctor->GetNumArgs() == 2U) {
        ASSERT(ctor->GetArgType(1).GetId() == panda_file::Type::TypeId::REFERENCE);
        auto *strData = utf::Mutf8AsCString(ctor->GetRefArgType(1).data);
        if (std::strcmp("[C", strData) == 0) {
            auto *array = reinterpret_cast<EtsArray *>(obj);
            str = EtsString::CreateNewStringFromChars(0, array->GetLength(), array);
        } else if ((std::strcmp("Lstd/core/String;", strData) == 0)) {
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
    if (!classLinker_->InitializeClass(coroutine, EtsClass::FromRuntimeClass(entrypoint->GetClass()))) {
        LOG(ERROR, RUNTIME) << "Cannot initialize class '" << entrypoint->GetClass()->GetName() << "'";
        return Unexpected(Runtime::Error::CLASS_NOT_INITIALIZED);
    }

    [[maybe_unused]] EtsHandleScope scope(coroutine);
    if (entrypoint->GetNumArgs() == 0) {
        auto v = entrypoint->Invoke(coroutine, nullptr);
        return v.GetAs<int>();
    }

    if (entrypoint->GetNumArgs() == 1) {
        EtsObjectArray *etsObjectArray = CreateArgumentsArray(args, this);
        EtsHandle<EtsObjectArray> argsHandle(coroutine, etsObjectArray);
        Value argVal(argsHandle.GetPtr()->AsObject()->GetCoreType());
        auto v = entrypoint->Invoke(coroutine, &argVal);

        return v.GetAs<int>();
    }

    // What if entrypoint->GetNumArgs() > 1 ?
    LOG(ERROR, RUNTIME) << "ets entrypoint has args count more than 1 : " << entrypoint->GetNumArgs();
    return Unexpected(Runtime::Error::INVALID_ENTRY_POINT);
}

ObjectHeader *PandaEtsVM::GetOOMErrorObject()
{
    auto obj = GetGlobalObjectStorage()->Get(oomObjRef_);
    ASSERT(obj != nullptr);
    return obj;
}

ObjectHeader *PandaEtsVM::GetUndefinedObject()
{
    auto obj = GetGlobalObjectStorage()->Get(undefinedObjRef_);
    ASSERT(obj != nullptr);
    return obj;
}

bool PandaEtsVM::LoadNativeLibrary(EtsEnv *env, const PandaString &name)
{
    ASSERT_PRINT(Coroutine::GetCurrent()->IsInNativeCode(), "LoadNativeLibrary must be called at native");

    if (auto error = nativeLibraryProvider_.LoadLibrary(env, name)) {
        LOG(ERROR, RUNTIME) << "Cannot load library " << name << ": " << error.value();
        return false;
    }

    return true;
}

void PandaEtsVM::ResolveNativeMethod(Method *method)
{
    ASSERT_PRINT(method->IsNative(), "Method should be native");
    std::string className = utf::Mutf8AsCString(method->GetClassName().data);
    std::string methodName = utf::Mutf8AsCString(method->GetName().data);
    std::string name = MangleMethodName(className, methodName);
    auto ptr = nativeLibraryProvider_.ResolveSymbol(PandaString(name));
    if (ptr == nullptr) {
        std::string signature;
        signature.append(EtsMethod::FromRuntimeMethod(method)->GetMethodSignature(false));
        name = MangleMethodNameWithSignature(name, signature);
        ptr = nativeLibraryProvider_.ResolveSymbol(PandaString(name));
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

static void PrintExceptionInfo(EtsCoroutine *coro, EtsHandle<EtsObject> exception, PandaStringStream &ss)
{
    auto extension = coro->GetPandaVM()->GetEtsClassLinkerExtension();
    auto cls = exception->GetClass();

    PandaVector<uint8_t> strBuf;
    auto const performCall = [coro, &exception, &strBuf](EtsMethod *method) -> std::optional<std::string_view> {
        ASSERT(method != nullptr);
        std::array<Value, 1> args = {Value(exception->GetCoreType())};
        EtsObject *callRes = EtsObject::FromCoreType(
            EtsMethod::ToRuntimeMethod(method)->Invoke(coro, args.data()).GetAs<ObjectHeader *>());
        if (coro->HasPendingException() || callRes == EtsObject::FromCoreType(coro->GetUndefinedObject())) {
            return std::nullopt;
        }
        return EtsString::FromEtsObject(callRes)->ConvertToStringView(&strBuf);
    };

    char const *dumperName =
        extension->GetErrorClass()->IsAssignableFrom(cls->GetRuntimeClass()) ? "<get>stack" : "toString";
    ss << std::endl << performCall(cls->GetMethod(dumperName)).value_or("exception dump failed");
}

void PandaEtsVM::HandleUncaughtException()
{
    auto coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    ScopedManagedCodeThread sj(coro);
    [[maybe_unused]] EtsHandleScope scope(coro);

    EtsHandle<EtsObject> exception(coro, EtsObject::FromCoreType(coro->GetException()));

    PandaStringStream logStream;
    logStream << "Unhandled exception: " << exception->GetCoreType()->ClassAddr<Class>()->GetName();

    auto descr = exception->GetClass()->GetDescriptor();
    if (descr != panda_file_items::class_descriptors::OUT_OF_MEMORY_ERROR &&
        descr != panda_file_items::class_descriptors::STACK_OVERFLOW_ERROR) {
        coro->ClearException();
        PrintExceptionInfo(coro, exception, logStream);
    }
    LOG(ERROR, RUNTIME) << logStream.str();
    // _exit guarantees a safe completion in case of multi-threading as static destructors aren't called
    _exit(1);
}

void PandaEtsVM::SweepVmRefs(const GCObjectVisitor &gcObjectVisitor)
{
    PandaVM::SweepVmRefs(gcObjectVisitor);
    {
        os::memory::LockHolder lock(finalizationRegistryLock_);
        auto it = registeredFinalizationRegistryInstances_.begin();
        while (it != registeredFinalizationRegistryInstances_.end()) {
            if (gcObjectVisitor((*it)->GetCoreType()) == ObjectStatus::DEAD_OBJECT) {
                auto toRemove = it++;
                registeredFinalizationRegistryInstances_.erase(toRemove);
            } else {
                ++it;
            }
        }
    }
}

void HandleEmptyArguments(const PandaVector<Value> &arguments, const GCRootVisitor &visitor,
                          const EtsCoroutine *coroutine)
{
    // arguments may be empty in the following cases:
    // 1. The entrypoint is static and doesn't accept any arguments
    // 2. The coroutine is launched.
    // 3. The entrypoint is the main method
    Method *entrypoint = coroutine->GetManagedEntrypoint();
    panda_file::ShortyIterator it(entrypoint->GetShorty());
    size_t argIdx = 0;
    ++it;  // skip return type
    if (!entrypoint->IsStatic()) {
        // handle 'this' argument
        ASSERT(arguments[argIdx].IsReference());
        ObjectHeader *arg = arguments[argIdx].GetAs<ObjectHeader *>();
        ASSERT(arg != nullptr);
        visitor(mem::GCRoot(mem::RootType::ROOT_THREAD, arg));
        ++argIdx;
    }
    while (it != panda_file::ShortyIterator()) {
        if ((*it).GetId() == panda_file::Type::TypeId::REFERENCE) {
            ASSERT(arguments[argIdx].IsReference());
            ObjectHeader *arg = arguments[argIdx].GetAs<ObjectHeader *>();
            if (arg != nullptr) {
                visitor(mem::GCRoot(mem::RootType::ROOT_THREAD, arg));
            }
        }
        ++it;
        ++argIdx;
    }
}

void PandaEtsVM::VisitVmRoots(const GCRootVisitor &visitor)
{
    GetThreadManager()->EnumerateThreads([visitor](ManagedThread *thread) {
        const auto coroutine = EtsCoroutine::CastFromThread(thread);
        if (auto etsNapiEnv = coroutine->GetEtsNapiEnv()) {
            auto etsStorage = etsNapiEnv->GetEtsReferenceStorage();
            etsStorage->GetAsReferenceStorage()->VisitObjects(visitor, mem::RootType::ROOT_NATIVE_LOCAL);
        }
        if (!coroutine->HasManagedEntrypoint()) {
            return true;
        }
        const PandaVector<Value> &arguments = coroutine->GetManagedEntrypointArguments();
        if (!arguments.empty()) {
            HandleEmptyArguments(arguments, visitor, coroutine);
        }
        return true;
    });
    visitor(mem::GCRoot(mem::RootType::ROOT_VM, doubleToStringCache_->GetCoreType()));
    visitor(mem::GCRoot(mem::RootType::ROOT_VM, floatToStringCache_->GetCoreType()));
    visitor(mem::GCRoot(mem::RootType::ROOT_VM, longToStringCache_->GetCoreType()));
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
        ObjectHeader *forwardAddress = mem::GetForwardAddress(arg);
        ref = Value(forwardAddress);
        LOG(DEBUG, GC) << "Forward root object: " << arg << " -> " << forwardAddress;
    }
}

void PandaEtsVM::UpdateVmRefs()
{
    GetThreadManager()->EnumerateThreads([](ManagedThread *thread) {
        auto coroutine = EtsCoroutine::CastFromThread(thread);
        if (auto etsNapiEnv = coroutine->GetEtsNapiEnv()) {
            auto etsStorage = etsNapiEnv->GetEtsReferenceStorage();
            etsStorage->GetAsReferenceStorage()->UpdateMovedRefs();
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
            size_t argIdx = 0;
            ++it;  // skip return type
            if (!entrypoint->IsStatic()) {
                // handle 'this' argument
                UpdateMovedVmRef<false>(arguments[argIdx]);
                ++argIdx;
            }
            while (it != panda_file::ShortyIterator()) {
                if ((*it).GetId() == panda_file::Type::TypeId::REFERENCE) {
                    UpdateMovedVmRef<true>(arguments[argIdx]);
                }
                ++it;
                ++argIdx;
            }
        }
        return true;
    });

    {
        os::memory::LockHolder lock(finalizationRegistryLock_);
        for (auto &entry : registeredFinalizationRegistryInstances_) {
            if ((entry->GetCoreType())->IsForwarded()) {
                entry = EtsObject::FromCoreType(ark::mem::GetForwardAddress(entry->GetCoreType()));
            }
        }
    }
}

static mem::Reference *EtsNapiObjectToGlobalReference(ets_object globalRef)
{
    ASSERT(globalRef != nullptr);
    auto *ref = reinterpret_cast<mem::Reference *>(globalRef);
    return ref;
}

void PandaEtsVM::DeleteGlobalRef(ets_object globalRef)
{
    auto *ref = EtsNapiObjectToGlobalReference(globalRef);
    if (!ref->IsGlobal()) {
        LOG(FATAL, ETS_NAPI) << "Try to remote non-global ref: " << std::hex << ref;
        return;
    }
    GetGlobalObjectStorage()->Remove(ref);
}

static mem::Reference *EtsNapiObjectToWeakReference(ets_weak weakRef)
{
    ASSERT(weakRef != nullptr);
    auto *ref = reinterpret_cast<mem::Reference *>(weakRef);
    return ref;
}

void PandaEtsVM::DeleteWeakGlobalRef(ets_weak weakRef)
{
    auto *ref = EtsNapiObjectToWeakReference(weakRef);
    if (!ref->IsWeak()) {
        LOG(FATAL, ETS_NAPI) << "Try to remote non-weak ref: " << std::hex << ref;
        return;
    }
    GetGlobalObjectStorage()->Remove(ref);
}

void PandaEtsVM::RegisterFinalizationRegistryInstance(EtsObject *instance)
{
    os::memory::LockHolder lock(finalizationRegistryLock_);
    registeredFinalizationRegistryInstances_.push_back(instance);
}

void PandaEtsVM::RegisterFinalizerForObject(EtsCoroutine *coro, const EtsHandle<EtsObject> &object,
                                            void (*finalizer)(void *), void *finalizerArg)
{
    ASSERT_MANAGED_CODE();
    auto *weakRef = EtsFinalizableWeakRef::Create(coro);
    weakRef->SetFinalizer(finalizer, finalizerArg);
    weakRef->SetReferent(object.GetPtr());
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    os::memory::LockHolder lh(finalizableWeakRefListLock_);
    weakRefList->Push(coro, weakRef);
}

void PandaEtsVM::CleanFinalizableReferenceList()
{
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    os::memory::LockHolder lh(finalizableWeakRefListLock_);
    weakRefList->UnlinkClearedReferences(EtsCoroutine::GetCurrent());
}

void PandaEtsVM::BeforeShutdown()
{
    ScopedManagedCodeThread managedScope(EtsCoroutine::GetCurrent());
    auto *coreList = GetGlobalObjectStorage()->Get(finalizableWeakRefList_);
    auto *weakRefList = EtsFinalizableWeakRefList::FromCoreType(coreList);
    weakRefList->TraverseAndFinalize();
}

/* static */
void PandaEtsVM::Abort(const char *message /* = nullptr */)
{
    Runtime::Abort(message);
}
}  // namespace ark::ets
