/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

#include <node_api.h>

#include "ani.h"
#include "libarkbase/os/thread.h"
#include "plugins/ets/runtime/ets_ani_env.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/execution/coroutines/stackful/stackful_coroutine_worker.h"
#include "runtime/execution/dfx/async_stack_helper.h"
#include "runtime/execution/job_manager.h"

namespace {

constexpr const char *RUNNER_MODULE = "ETS_ANI_HYBRID_RUNNER";
constexpr const char *TEST_CLASS = "exclusive_scope_plan.ExclusiveScopePlanRunner";
constexpr const char *TEST_HOOKS_CLASS = "exclusive_scope_plan.ExclusiveScopeTestHooks";
constexpr const char *TEST_METHOD = "main";
constexpr const char *EA_WORKER_CLASS = "std.core.EAWorker";
constexpr const char *EXCLUSIVE_SCOPE_METHOD = "exclusiveScope";
constexpr const char *EXCLUSIVE_SCOPE_SIGNATURE = "C{std.core.Function0}:C{std.core.Object}";
constexpr const char *NULL_POINTER_ERROR_CLASS = "std.core.NullPointerError";
constexpr const char *NULL_CALLBACK_MESSAGE = "EAWorker:: exclusiveScope callback must not be null";
constexpr ani_int EXPECTED_RUNNER_CASE_COUNT = 20;
constexpr auto OS_STACK_READY_TIMEOUT = std::chrono::seconds(10);
constexpr uint32_t OS_STACK_READY_POLL_INTERVAL_MS = 1U;

std::atomic<bool> g_asyncStackTestEnabled {false};
thread_local uint64_t g_asyncStackTestId = 0U;

class TestAsyncStackHelper final : public ark::dfx::AsyncStackHelper {
public:
    bool CheckLoadDfxAsyncStackFunc() const override
    {
        return true;
    }

    uint64_t CollectAsyncStack([[maybe_unused]] ark::dfx::StackType stackType,
                               [[maybe_unused]] size_t depth) const override
    {
        // Atomic with relaxed order reason: the flag does not publish or synchronize any other state
        return g_asyncStackTestEnabled.load(std::memory_order_relaxed) ? 1U : 0U;
    }

    void SetStackId(uint64_t id) const override
    {
        g_asyncStackTestId = id;
    }

    uint64_t GetStackId() const override
    {
        return g_asyncStackTestId;
    }
};

ani_boolean WaitUntilOsStackWorkReady(ani_env *aniEnv, [[maybe_unused]] ani_class klass, ani_int workerId)
{
    auto *executionCtx = ark::ets::PandaAniEnv::FromAniEnv(aniEnv)->GetExecutionContext();
    auto *jobManager = executionCtx->GetPandaVM()->GetJobManager();
    auto deadline = std::chrono::steady_clock::now() + OS_STACK_READY_TIMEOUT;
    do {
        bool ready = false;
        jobManager->EnumerateWorkers([workerId, &ready](ark::JobWorkerThread *worker) {
            if (worker->GetId() != workerId) {
                return true;
            }
            ready = worker->InExclusiveMode() &&
                    ark::StackfulCoroutineWorker::FromJobWorkerThread(worker)->IsServingOsStackWork();
            return false;
        });
        if (ready) {
            return ANI_TRUE;
        }
        ark::os::thread::NativeSleep(OS_STACK_READY_POLL_INTERVAL_MS);
    } while (std::chrono::steady_clock::now() < deadline);
    return ANI_FALSE;
}

bool BindTestHooks(ani_env *aniEnv, std::string *failure)
{
    ani_class hooksClass = nullptr;
    ani_status status = aniEnv->FindClass(TEST_HOOKS_CLASS, &hooksClass);
    if (status != ANI_OK || hooksClass == nullptr) {
        *failure = "cannot find exclusiveScope test hooks class, status " + std::to_string(status);
        return false;
    }
    ani_native_function hook {"waitUntilOsStackWorkReady", "i:z", reinterpret_cast<void *>(WaitUntilOsStackWorkReady)};
    status = aniEnv->Class_BindStaticNativeMethods(hooksClass, &hook, 1U);
    if (status != ANI_OK) {
        *failure = "cannot bind exclusiveScope test hooks, status " + std::to_string(status);
        return false;
    }
    return true;
}

struct NullCallbackSymbols {
    ani_class workerClass {nullptr};
    ani_static_method exclusiveScope {nullptr};
    ani_class nullPointerErrorClass {nullptr};
};

std::string GetErrorStringProperty(ani_env *aniEnv, ani_error error, const char *getter)
{
    ani_ref value = nullptr;
    ani_value noArguments {};
    ani_status status =
        aniEnv->Object_CallMethodByName_Ref_A(error, getter, ":C{std.core.String}", &value, &noArguments);
    if (status != ANI_OK || value == nullptr) {
        return "<unavailable>";
    }
    auto string = reinterpret_cast<ani_string>(value);
    ani_size size = 0;
    if (aniEnv->String_GetUTF8Size(string, &size) != ANI_OK) {
        return "<unavailable>";
    }
    std::string result(size + 1, '\0');
    ani_size written = 0;
    if (aniEnv->String_GetUTF8SubString(string, 0, size, result.data(), result.size(), &written) != ANI_OK) {
        return "<unavailable>";
    }
    result.resize(written);
    return result;
}

bool ResolveNullCallbackSymbols(ani_env *aniEnv, NullCallbackSymbols *symbols, std::string *failure)
{
    ani_status status = aniEnv->FindClass(EA_WORKER_CLASS, &symbols->workerClass);
    if (status != ANI_OK || symbols->workerClass == nullptr) {
        *failure = "cannot find std.core.EAWorker, status " + std::to_string(status);
        return false;
    }

    status = aniEnv->Class_FindStaticMethod(symbols->workerClass, EXCLUSIVE_SCOPE_METHOD, EXCLUSIVE_SCOPE_SIGNATURE,
                                            &symbols->exclusiveScope);
    if (status != ANI_OK || symbols->exclusiveScope == nullptr) {
        *failure = "cannot find EAWorker.exclusiveScope, status " + std::to_string(status);
        return false;
    }

    status = aniEnv->FindClass(NULL_POINTER_ERROR_CLASS, &symbols->nullPointerErrorClass);
    if (status != ANI_OK || symbols->nullPointerErrorClass == nullptr) {
        *failure = "cannot find std.core.NullPointerError, status " + std::to_string(status);
        return false;
    }
    return true;
}

bool InvokeNullCallback(ani_env *aniEnv, const NullCallbackSymbols &symbols, ani_error *error, std::string *failure)
{
    ani_ref nullCallback = nullptr;
    ani_status status = aniEnv->GetNull(&nullCallback);
    if (status != ANI_OK) {
        *failure = "cannot create ANI null reference, status " + std::to_string(status);
        return false;
    }

    std::array<ani_value, 1> args {};
    args[0].r = nullCallback;
    ani_ref unusedResult = nullptr;
    status =
        aniEnv->Class_CallStaticMethod_Ref_A(symbols.workerClass, symbols.exclusiveScope, &unusedResult, args.data());
    if (status != ANI_PENDING_ERROR) {
        *failure = "EAWorker.exclusiveScope(null) expected ANI_PENDING_ERROR, got " + std::to_string(status);
        return false;
    }

    ani_boolean hasError = ANI_FALSE;
    if (aniEnv->ExistUnhandledError(&hasError) != ANI_OK || hasError != ANI_TRUE) {
        *failure = "EAWorker.exclusiveScope(null) did not leave a pending error";
        return false;
    }

    if (aniEnv->GetUnhandledError(error) != ANI_OK || *error == nullptr) {
        *failure = "cannot retrieve the pending null callback error";
        return false;
    }
    if (aniEnv->ResetError() != ANI_OK) {
        *failure = "cannot clear the pending null callback error";
        return false;
    }
    return true;
}

bool ValidateNullCallbackError(ani_env *aniEnv, const NullCallbackSymbols &symbols, ani_error error,
                               std::string *failure)
{
    ani_boolean isNullPointerError = ANI_FALSE;
    ani_status status = aniEnv->Object_InstanceOf(error, symbols.nullPointerErrorClass, &isNullPointerError);
    if (status != ANI_OK || isNullPointerError != ANI_TRUE) {
        *failure = "EAWorker.exclusiveScope(null) did not throw std.core.NullPointerError; actual " +
                   GetErrorStringProperty(aniEnv, error, "%%get-name") + ": " +
                   GetErrorStringProperty(aniEnv, error, "%%get-message");
        return false;
    }

    auto message = GetErrorStringProperty(aniEnv, error, "%%get-message");
    if (message != NULL_CALLBACK_MESSAGE) {
        *failure = "unexpected null callback NullPointerError message: " + message;
        return false;
    }

    ani_boolean hasError = ANI_TRUE;
    if (aniEnv->ExistUnhandledError(&hasError) != ANI_OK || hasError != ANI_FALSE) {
        *failure = "null callback error remained pending after ANI validation";
        return false;
    }
    return true;
}

bool CheckNullCallbackViaAni(ani_env *aniEnv, std::string *failure)
{
    NullCallbackSymbols symbols;
    ani_error error = nullptr;
    if (!ResolveNullCallbackSymbols(aniEnv, &symbols, failure) ||
        !InvokeNullCallback(aniEnv, symbols, &error, failure) ||
        !ValidateNullCallbackError(aniEnv, symbols, error, failure)) {
        return false;
    }
    std::cout << "PASS null callback rejected (ANI direct call)" << std::endl;
    return true;
}

napi_value ThrowRunnerError(napi_env env, const std::string &message)
{
    std::cerr << "[" << RUNNER_MODULE << "] " << message << std::endl;
    napi_throw_error(env, nullptr, message.c_str());
    return nullptr;
}

void DescribeAndResetPendingError(ani_env *aniEnv)
{
    ani_boolean hasError = ANI_FALSE;
    if (aniEnv != nullptr && aniEnv->ExistUnhandledError(&hasError) == ANI_OK && hasError == ANI_TRUE) {
        aniEnv->DescribeError();
        aniEnv->ResetError();
    }
}

napi_value DestroyVmAndThrowRunnerError(napi_env env, ani_vm *vm, const std::string &message)
{
    ani_status status = vm->DestroyVM();
    if (status != ANI_OK) {
        return ThrowRunnerError(env, message + "; ani_vm::DestroyVM failed with status " + std::to_string(status));
    }
    return ThrowRunnerError(env, message);
}

ani_status CreateTestVm(napi_env env, const char *stdlib, const char *testAbc, ani_vm **vm)
{
    std::string bootFiles = std::string("--ext:boot-panda-files=") + stdlib + ":" + testAbc;
    const std::array optionsArray = {
        ani_option {bootFiles.c_str(), nullptr},
        ani_option {"--ext:gc-type=g1-gc", nullptr},
        ani_option {"--ext:gc-trigger-type=heap-trigger", nullptr},
        ani_option {"--ext:compiler-enable-jit=false", nullptr},
        ani_option {"--ext:run-gc-in-place=true", nullptr},
        ani_option {"--ext:coroutine-workers-count=1", nullptr},
        ani_option {"--ext:taskpool-support-interop=true", nullptr},
        ani_option {"--ext:interop", env},
    };
    ani_options options {optionsArray.size(), optionsArray.data()};
    return ANI_CreateVM(&options, ANI_VERSION_1, vm);
}

bool CallVoidTest(ani_env *aniEnv, ani_class testClass, const char *methodName, std::string *failure)
{
    ani_value noArguments {};
    ani_status status = aniEnv->Class_CallStaticMethodByName_Void_A(testClass, methodName, ":", &noArguments);
    if (status != ANI_OK) {
        *failure = std::string(methodName) + " failed with status " + std::to_string(status);
        return false;
    }
    return true;
}

bool RunEtsTests(ani_env *aniEnv, std::string *failure)
{
    if (!CheckNullCallbackViaAni(aniEnv, failure)) {
        *failure = "ANI null callback test failed: " + *failure;
        return false;
    }

    ani_class testClass = nullptr;
    ani_status status = aniEnv->FindClass(TEST_CLASS, &testClass);
    if (status != ANI_OK || testClass == nullptr) {
        *failure = "cannot find ETS test class, status " + std::to_string(status);
        return false;
    }
    if (!BindTestHooks(aniEnv, failure)) {
        return false;
    }

    // The Host runtime has no OHOS DFX provider. This helper models the same
    // zero/non-zero stack-ID switch used by the production implementation.
    auto *executionCtx = ark::ets::PandaAniEnv::FromAniEnv(aniEnv)->GetExecutionContext();
    executionCtx->GetPandaVM()->GetJobManager()->SetAsyncStackHelper(ark::MakePandaUnique<TestAsyncStackHelper>());

    // Atomic with relaxed order reason: the flag does not publish or synchronize any other state
    g_asyncStackTestEnabled.store(false, std::memory_order_relaxed);
    if (!CallVoidTest(aniEnv, testClass, "testManagedStackStitchingDisabled", failure)) {
        return false;
    }
    std::cout << "PASS managed Error.stack stitching disabled by async-stack switch" << std::endl;

    // Atomic with relaxed order reason: the flag does not publish or synchronize any other state
    g_asyncStackTestEnabled.store(true, std::memory_order_relaxed);
    return CallVoidTest(aniEnv, testClass, TEST_METHOD, failure);
}

napi_value RunEtsMain(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    if (napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr) != napi_ok || argc != 0) {
        return ThrowRunnerError(env, "runEtsMain expects no arguments");
    }

    const char *stdlib = std::getenv("ARK_ETS_STDLIB_PATH");
    const char *testAbc = std::getenv("ARK_ETS_TEST_ABC_PATH");
    if (stdlib == nullptr || testAbc == nullptr) {
        return ThrowRunnerError(env, "ARK_ETS_STDLIB_PATH or ARK_ETS_TEST_ABC_PATH is not set");
    }

    ani_vm *vm = nullptr;
    ani_status status = CreateTestVm(env, stdlib, testAbc, &vm);
    if (status != ANI_OK || vm == nullptr) {
        return ThrowRunnerError(env, "ANI_CreateVM failed with status " + std::to_string(status));
    }

    ani_env *aniEnv = nullptr;
    status = vm->GetEnv(ANI_VERSION_1, &aniEnv);
    if (status != ANI_OK || aniEnv == nullptr) {
        return DestroyVmAndThrowRunnerError(env, vm, "ani_vm::GetEnv failed with status " + std::to_string(status));
    }

    std::string failure;
    if (!RunEtsTests(aniEnv, &failure)) {
        DescribeAndResetPendingError(aniEnv);
        return DestroyVmAndThrowRunnerError(env, vm, failure);
    }

    status = vm->DestroyVM();
    if (status != ANI_OK) {
        return ThrowRunnerError(env, "ani_vm::DestroyVM failed with status " + std::to_string(status));
    }

    napi_value jsResult = nullptr;
    if (napi_create_int32(env, EXPECTED_RUNNER_CASE_COUNT, &jsResult) != napi_ok) {
        return ThrowRunnerError(env, "cannot create JS result");
    }
    return jsResult;
}

napi_value Init(napi_env env, napi_value exports)
{
    const std::array descriptors = {
        napi_property_descriptor {"runEtsMain", nullptr, RunEtsMain, nullptr, nullptr, nullptr, napi_enumerable,
                                  nullptr},
    };
    if (napi_define_properties(env, exports, descriptors.size(), descriptors.data()) != napi_ok) {
        return ThrowRunnerError(env, "cannot define runner exports");
    }
    return exports;
}

}  // namespace

NAPI_MODULE(ETS_ANI_HYBRID_RUNNER, Init)
