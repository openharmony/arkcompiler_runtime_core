/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <unordered_set>

#include "include/managed_thread.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/runtime_helpers.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_atomic_flag.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_typeapi_type.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "runtime/execution/job_execution_context.h"
#include "runtime/include/thread_scopes.h"

#include "runtime/include/stack_walker.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"
#include "types/ets_primitives.h"

#ifndef UINT8_MAX
#define UINT8_MAX (255)
#endif  // UINT8_MAX

namespace ark::ets::intrinsics {

extern "C" EtsInt CountInstancesOfClass(EtsClass *cls)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    if (cls == nullptr) {
        return EtsInt(0);
    }
    auto *heapMgr = executionCtx->GetPandaVM()->GetHeapManager();
    return static_cast<EtsInt>(heapMgr->CountInstancesOfClass(cls->GetRuntimeClass()));
}

extern "C" EtsArray *StdCoreStackTraceLines()
{
    auto runtime = Runtime::GetCurrent();
    auto linker = runtime->GetClassLinker();
    auto ext = linker->GetExtension(panda_file::SourceLang::ETS);
    auto klass = ext->GetClassRoot(ClassRoot::ARRAY_STRING);

    auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::ETS);

    auto executionCtx = EtsExecutionContext::GetCurrent();
    auto walker = StackWalker::Create(executionCtx->GetMT());

    std::vector<std::string> lines;

    for (auto stack = StackWalker::Create(executionCtx->GetMT()); stack.HasFrame(); stack.NextFrame()) {
        Method *method = stack.GetMethod();
        auto *source = method->GetClassSourceFile().data;
        auto lineNum = method->GetLineNumFromBytecodeOffset(stack.GetBytecodePc());

        if (source == nullptr) {
            source = utf::CStringAsMutf8("<unknown>");
        }

        std::stringstream ss;
        ss << method->GetClass()->GetName() << "." << method->GetName().data << " at " << source << ":" << lineNum;
        lines.push_back(ss.str());
    }

    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    auto *arr = ark::coretypes::Array::Create(klass, lines.size());

    VMHandle<coretypes::Array> arrayHandle(executionCtx->GetMT(), arr);

    for (ark::ArraySizeT i = 0; i < (ark::ArraySizeT)lines.size(); i++) {
        auto *str = coretypes::String::CreateFromMUtf8(utf::CStringAsMutf8(lines[i].data()), lines[i].length(), ctx,
                                                       executionCtx->GetPandaVM());
        arrayHandle.GetPtr()->Set(i, str);
    }

    return reinterpret_cast<EtsArray *>(arrayHandle.GetPtr());
}

extern "C" void StdCorePrintStackTrace()
{
    ark::PrintStackTrace();
}

#ifdef PANDA_BUILD_LINUX_PANDA_RUNTIME
static PandaString NormalizeLibraryName(const PandaString &name)
{
#if !defined(PANDA_TARGET_OHOS)
    constexpr PandaString::size_type Z_SUFFIX_LEN = 2;
    if (name.size() > Z_SUFFIX_LEN && name.compare(name.size() - Z_SUFFIX_LEN, Z_SUFFIX_LEN, ".z") == 0) {
        return name.substr(0, name.size() - Z_SUFFIX_LEN);
    }
#endif
    return name;
}

static constexpr const char *PANDA_SHARED_LIBRARY_PREFIX = "lib";

static constexpr const char *GetLibraryExtension()
{
#if defined(PANDA_TARGET_WINDOWS)
    return ".dll";
#elif defined(PANDA_TARGET_MACOS)
    return ".dylib";
#elif defined(PANDA_TARGET_UNIX)
    return ".so";
#else
    UNREACHABLE();
#endif
}

static PandaString ComposeLibraryName(const PandaString &name)
{
    return PandaString(PANDA_SHARED_LIBRARY_PREFIX) + name + GetLibraryExtension();
}

static PandaString ResolveLibraryName(const PandaString &name)
{
    return ComposeLibraryName(NormalizeLibraryName(name));
}

static bool EndsWith(const PandaString &value, const char *suffix)
{
    PandaString suffixStr(suffix);
    if (value.size() < suffixStr.size()) {
        return false;
    }
    return value.compare(value.size() - suffixStr.size(), suffixStr.size(), suffixStr) == 0;
}

static void AddUniqueLibraryName(std::unordered_set<PandaString> *candidates, const PandaString &libraryName)
{
    ASSERT(candidates != nullptr);
    candidates->insert(libraryName);
}

static std::unordered_set<PandaString> GetLibraryLoadCandidates(const PandaString &name)
{
    std::unordered_set<PandaString> candidatesSet;
    AddUniqueLibraryName(&candidatesSet, ResolveLibraryName(name));

#if !defined(PANDA_TARGET_OHOS)
    // Keep compatibility with runtime names that still carry the legacy ".z" suffix.
    if (EndsWith(name, ".z")) {
        AddUniqueLibraryName(&candidatesSet, ComposeLibraryName(name));
    }

    // Previewer packages context ANI kits as simulator_context_ani_kit.<ext>.
    if (name.find("context_ani_kit") != PandaString::npos) {
        AddUniqueLibraryName(&candidatesSet, ComposeLibraryName("simulator_context_ani_kit"));
    }

    // Previewer uses previewer_window_napi.<ext> instead of windowstageani_module.<ext>.
    if (name.find("windowstageani_module") != PandaString::npos ||
        name.find("embeddablewindowstageani_module") != PandaString::npos) {
        AddUniqueLibraryName(&candidatesSet, ComposeLibraryName("previewer_window_napi"));
    }
#endif

    return candidatesSet;
}
#else
static PandaString ResolveLibraryName(const PandaString &name)
{
#ifdef PANDA_TARGET_UNIX
    return PandaString("lib") + name + ".so";
#else
    // Unsupported on windows platform
    UNREACHABLE();
#endif  // PANDA_TARGET_UNIX
}
#endif

void LoadNativeLibraryHandler(ark::ets::EtsString *name, bool shouldVerifyPermission,
                              ark::ets::EtsString *fileName = nullptr)
{
    ASSERT(name != nullptr);
    ASSERT(name->AsObject()->IsStringClass());
    auto executionCtx = EtsExecutionContext::GetCurrent();
    if (shouldVerifyPermission) {
        ASSERT(fileName != nullptr);
        ASSERT(fileName->AsObject()->IsStringClass());
    }

    if (name->IsUtf16()) {
        LOG(FATAL, RUNTIME) << "UTF-16 native library pathes are not supported";
        return;
    }

    auto nameStr = name->GetMutf8();
    if (nameStr.empty()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreError, "The native library path is empty");
        return;
    }
    PandaString fileNameStr = fileName != nullptr ? fileName->GetMutf8() : "";
    LOG(INFO, RUNTIME) << "load library name is " << nameStr.c_str()
                       << "; shouldVerifyPermission: " << shouldVerifyPermission
                       << "; fileName: " << fileNameStr.c_str();
    ScopedNativeCodeThread snct(executionCtx->GetMT());
    auto env = executionCtx->GetPandaAniEnv();
#ifdef PANDA_BUILD_LINUX_PANDA_RUNTIME
    bool loadSuccess = false;
    for (const auto &libraryName : GetLibraryLoadCandidates(nameStr)) {
        LOG(INFO, RUNTIME) << "try load library candidate " << libraryName.c_str();
        if (coroutine->GetPandaVM()->LoadNativeLibrary(env, libraryName, shouldVerifyPermission, fileNameStr)) {
            loadSuccess = true;
            break;
        }
    }
#else
    bool loadSuccess = executionCtx->GetPandaVM()->LoadNativeLibrary(env, ResolveLibraryName(nameStr),
                                                                     shouldVerifyPermission, fileNameStr);
#endif
    if (!loadSuccess) {
        ScopedManagedCodeThread smct(executionCtx->GetMT());

        PandaStringStream ss;
        ss << "Cannot load native library " << nameStr;

        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreExceptionInInitializerError, ss.str());
    }
}

extern "C" void LoadLibrary(ark::ets::EtsString *name)
{
    if (UNLIKELY(name == nullptr)) {
        ThrowNullPointerException();
        return;
    }
    LoadNativeLibraryHandler(name, false);
}

extern "C" void LoadLibraryWithPermissionCheck(ark::ets::EtsString *name, ark::ets::EtsString *fileName)
{
    if (UNLIKELY(name == nullptr || fileName == nullptr)) {
        ThrowNullPointerException();
        return;
    }
    LoadNativeLibraryHandler(name, true, fileName);
}

extern "C" void StdSystemScheduleCoroutine()
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    if (executionCtx->GetManager()->IsJobSwitchDisabled()) {
        ThrowEtsException(EtsExecutionContext::FromMT(executionCtx),
                          PlatformTypes(executionCtx)->coreInvalidJobOperationError,
                          "Cannot switch coroutines in the current context!");
        return;
    }

    ScopedNativeCodeThread nativeScope(executionCtx);
    executionCtx->GetManager()->ExecuteJobs();
}

extern "C" void StdSystemSetCoroutineSchedulingPolicy(int32_t policy)
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    if (Coroutine::MutatorIsCoroutine(executionCtx)) {
        constexpr auto POLICIES_MAPPING =
            std::array {CoroutineSchedulingPolicy::ANY_WORKER, CoroutineSchedulingPolicy::NON_MAIN_WORKER};
        ASSERT((policy >= 0) && (static_cast<size_t>(policy) < POLICIES_MAPPING.size()));
        CoroutineSchedulingPolicy newPolicy = POLICIES_MAPPING[policy];

        Coroutine *coro = Coroutine::CastFromMutator(executionCtx);
        ASSERT(coro != nullptr);
        auto *cm = static_cast<CoroutineManager *>(coro->GetVM()->GetThreadManager());
        cm->SetSchedulingPolicy(newPolicy);
    }
}

extern "C" int32_t StdSystemGetCoroutineId()
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    return coro->GetCoroutineId();
}

extern "C" EtsBoolean StdSystemIsMainWorker()
{
    return static_cast<EtsBoolean>(JobExecutionContext::GetCurrent()->GetWorker()->IsMainWorker());
}

extern "C" EtsBoolean StdSystemWorkerHasExternalScheduler()
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    return static_cast<EtsBoolean>(executionCtx->GetWorker()->IsExternalSchedulingEnabled());
}

extern "C" void StdSystemScaleWorkersPool(int32_t scaler)
{
    if (UNLIKELY(scaler == 0)) {
        return;
    }
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *vm = executionCtx->GetPandaVM();
    auto *runtime = vm->GetRuntime();
    auto *jobManager = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager();
    if (scaler > 0) {
        jobManager->CreateGeneralWorkers(scaler, runtime, vm);
        return;
    }
    ScopedNativeCodeThread nativeScope(executionCtx->GetMT());
    jobManager->FinalizeGeneralWorkers(std::abs(scaler), runtime, vm);
}

static Class *GetTaskPoolClass()
{
    auto *runtime = Runtime::GetCurrent();
    auto *classLinker = runtime->GetClassLinker();
    ClassLinkerExtension *cle = classLinker->GetExtension(SourceLanguage::ETS);
    auto mutf8Name = reinterpret_cast<const uint8_t *>("Lstd/concurrency/taskpool;");
    return cle->GetClass(mutf8Name);
}

extern "C" void StdSystemStopTaskpool()
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("stopAllWorkers", ":V");
    ASSERT(method != nullptr);
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    method->GetPandaMethod()->Invoke(executionCtx->GetMT(), nullptr);
}

extern "C" void StdSystemIncreaseTaskpoolWorkersToN(int32_t workersNum)
{
    if (UNLIKELY(workersNum == 0)) {
        return;
    }
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("increaseWorkersToN", "I:V");
    ASSERT(method != nullptr);
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    std::array args = {Value(workersNum)};
    method->GetPandaMethod()->Invoke(executionCtx->GetMT(), args.data());
}

extern "C" void StdSystemSetTaskPoolTriggerShrinkInterval(int32_t interval)
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Get taskpool class failed.";
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("setTaskPoolTriggerShrinkInterval", "I:V");
    ASSERT(method != nullptr);
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    std::array args = {Value(interval)};
    method->GetPandaMethod()->Invoke(executionCtx->GetMT(), args.data());
}

extern "C" void StdSystemSetTaskPoolIdleThreshold(int32_t threshold)
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Get taskpool class failed.";
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("setTaskPoolIdleThreshold", "I:V");
    ASSERT(method != nullptr);
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    std::array args = {Value {threshold}};
    method->GetPandaMethod()->Invoke(executionCtx->GetMT(), args.data());
}

extern "C" void StdSystemSetTaskPoolBlockedWorkerThreshold(int32_t thresholdMs)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *method = PlatformTypes(coro)->stdConcurrencyTaskpoolSetTaskPoolBlockedWorkerThreshold;
    std::array args = {Value {thresholdMs}};
    method->GetPandaMethod()->Invoke(coro, args.data());
}

extern "C" void StdSystemSetTaskPoolBlockedWorkerMonitorInterval(int32_t intervalMs)
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *method = PlatformTypes(coro)->stdConcurrencyTaskpoolSetTaskPoolBlockedWorkerMonitorInterval;
    std::array args = {Value {intervalMs}};
    method->GetPandaMethod()->Invoke(coro, args.data());
}

extern "C" EtsInt StdSystemGetTaskPoolWorkersNum()
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Get taskpool class failed.";
        return EtsInt(-1);
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("getTaskPoolWorkersNum", ":I");
    ASSERT(method != nullptr);
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ark::Value res = method->GetPandaMethod()->Invoke(executionCtx->GetMT(), nullptr);
    return res.GetAs<int>();
}

extern "C" void StdSystemSetTaskPoolWorkersLimit(int32_t threshold)
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Get taskpool class failed.";
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("setTaskPoolWorkersLimit", "I:V");
    ASSERT(method != nullptr);
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    std::array args = {Value {threshold}};
    method->GetPandaMethod()->Invoke(executionCtx->GetMT(), args.data());
}

extern "C" void StdSystemRetriggerTaskPoolShrink()
{
    auto *klass = GetTaskPoolClass();
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Get taskpool class failed.";
        return;
    }
    auto *method = EtsClass::FromRuntimeClass(klass)->GetStaticMethod("retriggerTaskPoolShrink", ":V");
    ASSERT(method != nullptr);
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    method->GetPandaMethod()->Invoke(executionCtx->GetMT(), nullptr);
}

extern "C" void StdSystemRetriggerTaskPoolBlockedExpandMonitor()
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto *method = PlatformTypes(coro)->stdConcurrencyTaskpoolRetriggerTaskPoolBlockedExpandMonitor;
    method->GetPandaMethod()->Invoke(coro, nullptr);
}

extern "C" void StdSystemAtomicFlagSet(EtsAtomicFlag *instance, EtsBoolean v)
{
    instance->SetValue(v);
}

extern "C" EtsBoolean StdSystemAtomicFlagGet(EtsAtomicFlag *instance)
{
    return instance->GetValue();
}

extern "C" EtsInt EtsStdCoreUint8ClampedArrayToUint8Clamped(EtsDouble val)
{
    // Convert the double value to uint8_t with clamping
    if (val <= 0 || std::isnan(val)) {
        return 0;
    }
    if (val > UINT8_MAX) {
        return UINT8_MAX;
    }
    return std::lrint(val);
}

extern "C" EtsBoolean StdSystemIsExternalTimerEnabled()
{
    auto *executionCtx = JobExecutionContext::GetCurrent();
    if (Coroutine::MutatorIsCoroutine(executionCtx)) {
        auto *coro = Coroutine::CastFromMutator(executionCtx);
        auto extTimerOption = ark::ets::ToEtsBoolean(coro->GetCoroutineManager()->GetConfig().enableExternalTimer);
        return ToEtsBoolean((extTimerOption != 0U) && coro->GetWorker()->IsExternalSchedulingEnabled());
    }
    return ToEtsBoolean(false);
}

extern "C" void StdSystemDumpUhandledFailedJobs()
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    ScopedNativeCodeThread snct(coro);
    coro->ProcessUnhandledFailedJobs();
}

}  // namespace ark::ets::intrinsics
