/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifdef PANDA_USE_FFRT
#include "c/ffrt_ipc.h"
#endif
#include "ani.h"
#include "ani_options_parser.h"
#include "ani_options.h"
#include "compiler/compiler_logger.h"
#include "compiler/compiler_options.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "libarkbase/panda_gen_options/generated/logger_options.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/interop_context_api.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/pandargs.h"

// CC-OFFNXT(huge_method[C++]) solid logic
extern "C" ani_status ANI_CreateVM(const ani_options *options, uint32_t version, ani_vm **result)
{
    ANI_CHECK_RETURN_IF_EQ(result, nullptr, ANI_INVALID_ARGS);
    if (!ark::ets::ani::IsVersionSupported(version)) {
        return ANI_INVALID_VERSION;
    }

    ark::ets::ani::OptionsParser parser;
    auto errMsg = parser.Parse(options);
    auto &aniOptions = parser.GetANIOptions();
    ark::Logger::Initialize(parser.GetLoggerOptions(), aniOptions.GetLoggerCallback());
    if (errMsg) {
        LOG(ERROR, ANI) << errMsg.value();
        return ANI_ERROR;
    }
    ark::compiler::CompilerLogger::SetComponents(ark::compiler::g_options.GetCompilerLog());

    if (!ark::Runtime::Create(parser.GetRuntimeOptions())) {
        LOG(ERROR, ANI) << "Cannot create runtime";
        return ANI_ERROR;
    }

    auto *mThread = ark::ManagedThread::GetCurrent();
    ASSERT(mThread != nullptr);
#ifdef PANDA_ETS_INTEROP_JS
    if (aniOptions.IsInteropMode()) {
        bool created = ark::ets::interop::js::CreateMainInteropContext(ark::ets::EtsExecutionContext::FromMT(mThread),
                                                                       aniOptions.GetInteropEnv());
        if (!created) {
            LOG(ERROR, ANI) << "Cannot create interop context";
            ark::Runtime::Destroy();
            return ANI_ERROR;
        }
    }
#endif /* PANDA_ETS_INTEROP_JS */

    *result = ark::ets::EtsExecutionContext::FromMT(mThread)->GetPandaVM();
    ASSERT(*result != nullptr);

    // NOTE:
    //  Don't change the following log message, because it used for testing logger callback.
    //  Or change log message at the same time as the ani_option_logger_parser_success_test.cpp test.
    LOG(INFO, ANI) << "ani_vm has been created";
    return ANI_OK;
}

// CC-OFFNXT(G.FUN.02-CPP) project code stytle
extern "C" ani_status ANI_GetCreatedVMs(ani_vm **vmsBuffer, ani_size vmsBufferLength, ani_size *result)
{
    ANI_CHECK_RETURN_IF_EQ(vmsBuffer, nullptr, ANI_INVALID_ARGS);
    ANI_CHECK_RETURN_IF_EQ(result, nullptr, ANI_INVALID_ARGS);

    auto *mutator = ark::Mutator::GetCurrent();
    if (mutator == nullptr) {
        *result = 0;
        return ANI_OK;
    }
    // After verifying that the current thread is attached to VM, it is valid to get current ManagedThread
    auto *mThread = ark::ManagedThread::GetCurrent();
    if (mThread != nullptr) {
        if (vmsBufferLength < 1) {
            return ANI_INVALID_ARGS;
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        vmsBuffer[0] = ark::ets::EtsExecutionContext::FromMT(mThread)->GetPandaVM();
        *result = 1;
    } else {
        *result = 0;
    }
    return ANI_OK;
}

namespace ark::ets::ani {

static ani_status DestroyVM(ani_vm *vm)
{
    ANI_DEBUG_TRACE(vm);
    ANI_CHECK_RETURN_IF_EQ(vm, nullptr, ANI_INVALID_ARGS);

    auto runtime = Runtime::GetCurrent();
    if (runtime == nullptr) {
        LOG(ERROR, ANI) << "Cannot destroy ANI VM, there is no current runtime";
        return ANI_ERROR;
    }

    auto pandaVm = PandaEtsVM::FromAniVM(vm);
    auto mainVm = runtime->GetPandaVM();
    if (pandaVm == mainVm) {
        Runtime::Destroy();
    } else {
        PandaEtsVM::Destroy(pandaVm);
    }

    return ANI_OK;
}

NO_UB_SANITIZE static ani_status GetEnv(ani_vm *vm, uint32_t version, ani_env **result)
{
    ANI_DEBUG_TRACE(vm);
    ANI_CHECK_RETURN_IF_EQ(vm, nullptr, ANI_INVALID_ARGS);
    ANI_CHECK_RETURN_IF_EQ(result, nullptr, ANI_INVALID_ARGS);

    if (!IsVersionSupported(version)) {
        return ANI_INVALID_VERSION;
    }

    Mutator *mutator = Mutator::GetCurrent();
    if (mutator == nullptr) {
        LOG(ERROR, ANI) << "Cannot get environment, thread is not attached to VM";
        return ANI_ERROR;
    }
    // After verifying that the current thread is attached to VM, it is valid to get current EtsExecutionContext
    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    if (UNLIKELY(executionCtx == nullptr)) {
        LOG(ERROR, ANI) << "Cannot get environment, no current execution context exists";
        return ANI_ERROR;
    }
    ani_env *env = executionCtx->GetPandaAniEnv();
    // Each EtsExecutionContext must have a valid ani_env
    ASSERT(env != nullptr);
    *result = env;
    return ANI_OK;
}

/**
 * This function attaches the current OS thread to the VM, thus allowing for ArkTS-Sta bytecode execution
 * in the arbitrary thread created by an ANI user.
 *
 * For a real OS thread, no additional environment setup is performed (except JS runtime creation in the interop mode).
 *
 * For a FFRT "thread", this function disables the FFRT task migration between threads by turning on the
 * legacy task mode to ensure correct operation.
 */

static ani_status AttachCurrentThread(ani_vm *vm, const ani_options *options, uint32_t version, ani_env **result)
{
    ANI_DEBUG_TRACE(vm);
    ANI_CHECK_RETURN_IF_EQ(vm, nullptr, ANI_INVALID_ARGS);
    ANI_CHECK_RETURN_IF_EQ(result, nullptr, ANI_INVALID_ARGS);
    ANI_CHECK_RETURN_IF_EQ(IsVersionSupported(version), false, ANI_INVALID_VERSION);

    if (Mutator::GetCurrent() != nullptr) {
        LOG(ERROR, ANI) << "Cannot attach current thread, thread has already been attached";
        return ANI_ERROR;
    }

    bool interopEnabled = false;
    void *jsEnv = nullptr;
    if (options != nullptr) {
        for (auto option : Span(options->options, options->nr_options)) {
            PandaString opt(option.option);
            if (opt == "--interop=enable") {
                interopEnabled = true;
                jsEnv = option.extra;
            } else if (opt == "--interop=disable") {
                interopEnabled = false;
            }
        }
    }

    auto *runtime = Runtime::GetCurrent();
    auto *etsVM = PandaEtsVM::FromAniVM(vm);
    auto *jobMan = etsVM->GetJobManager();
    auto *exclusiveCoro = jobMan->AttachExclusiveWorker(runtime, etsVM);
    if (exclusiveCoro == nullptr) {
        LOG(ERROR, ANI) << "Cannot attach current thread, reached the limit of EAWorkers";
        return ANI_ERROR;
    }

    ASSERT(exclusiveCoro == Coroutine::GetCurrent());

    if (interopEnabled) {
        bool isJsEnvCreatedExternally = true;
        auto *ifaceTable = EtsExecutionContext::FromMT(jobMan->GetMainThread())->GetExternalIfaceTable();
        if (jsEnv == nullptr) {
            jsEnv = ifaceTable->CreateJSRuntime();
            isJsEnvCreatedExternally = false;
            ASSERT(jsEnv != nullptr);
        }
        ifaceTable->CreateInteropCtx(EtsExecutionContext::FromMT(exclusiveCoro), jsEnv, isJsEnvCreatedExternally);
    }
    *result = EtsExecutionContext::FromMT(exclusiveCoro)->GetPandaAniEnv();

#ifdef PANDA_USE_FFRT
    ffrt_this_task_set_legacy_mode(true);
#endif
    return ANI_OK;
}

/**
 * This function detaches the current OS thread from the VM, thus revoking the ability to execute
 * ArkTS-Sta bytecode in this thread.
 *
 * For a real OS thread, no additional environment cleanup is performed (except JSEnv cleanup in the interop mode).
 *
 * For a FFRT "thread", this function re-enables FFRT task migration between threads by turning off
 * the legacy task mode that was turned on by AttachCurrentThread.
 */
static ani_status DetachCurrentThread(ani_vm *vm)
{
    ANI_DEBUG_TRACE(vm);
    ANI_CHECK_RETURN_IF_EQ(vm, nullptr, ANI_INVALID_ARGS);
    if (Mutator::GetCurrent() == nullptr) {
        LOG(ERROR, ANI) << "Cannot detach current thread, thread is not attached";
        return ANI_ERROR;
    }

    auto *etsVM = PandaEtsVM::FromAniVM(vm);
    auto *jobMan = etsVM->GetJobManager();

    auto *ifaceTable = EtsExecutionContext::FromMT(jobMan->GetMainThread())->GetExternalIfaceTable();
    bool isJsEnvCreatedExternally = ifaceTable->IsJsEnvCreatedExternally();
    auto *jsEnv = ifaceTable->GetJSEnv();
    auto result = jobMan->DetachExclusiveWorker();
    if (jsEnv != nullptr) {
        if (!isJsEnvCreatedExternally) {
            ifaceTable->CleanUpJSEnv(jsEnv);
        }
    }
    if (!result) {
        LOG(ERROR, ANI) << "Cannot DetachThread, thread was not attached";
        return ANI_ERROR;
    }
    ASSERT(Mutator::GetCurrent() == nullptr);

#ifdef PANDA_USE_FFRT
    ffrt_this_task_set_legacy_mode(false);
#endif

    return ANI_OK;
}

// clang-format off
const __ani_vm_api VM_API = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    DestroyVM,
    GetEnv,
    AttachCurrentThread,
    DetachCurrentThread,
};
// clang-format on

const __ani_vm_api *GetVMAPI()
{
    return &VM_API;
}
}  // namespace ark::ets::ani
