/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani.h"
#include "plugins/ets/runtime/ani/ani_checkers.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets::ani {

extern "C" ani_status ANI_GetCreatedVMs(ani_vm **vmsBuffer, ani_size vmsBufferLength, ani_size *result)
{
    ANI_DEBUG_TRACE(env);
    ANI_CHECK_RETURN_IF_EQ(vmsBuffer, nullptr, ANI_INVALID_ARGS);
    ANI_CHECK_RETURN_IF_EQ(result, nullptr, ANI_INVALID_ARGS);

    Thread *thread = Thread::GetCurrent();
    if (thread == nullptr) {
        *result = 0;
        return ANI_OK;
    }
    EtsCoroutine *coroutine = EtsCoroutine::CastFromThread(thread);
    if (coroutine != nullptr) {
        if (vmsBufferLength < 1) {
            return ANI_ERROR;
        }

        vmsBuffer[0] = coroutine->GetPandaVM();  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *result = 1;
    } else {
        *result = 0;
    }
    return ANI_OK;
}

NO_UB_SANITIZE static ani_status GetEnv(ani_vm *vm, uint32_t version, ani_env **result)
{
    ANI_DEBUG_TRACE(env);
    ANI_CHECK_RETURN_IF_EQ(result, nullptr, ANI_INVALID_ARGS);

    PandaEtsVM *pandaVM = PandaEtsVM::FromAniVM(vm);
    EtsCoroutine *coroutine = EtsCoroutine::CastFromThread(pandaVM->GetAssociatedThread());
    if (coroutine == nullptr) {
        LOG(ERROR, ANI) << "Cannot get environment";
        return ANI_ERROR;
    }
    if (version != ANI_VERSION_1) {
        return ANI_ERROR;  // NOTE: Unsupported version?
    }
    *result = coroutine->GetEtsNapiEnv();
    return ANI_OK;
}

[[noreturn]] static void NotImplementedAPI(int nr)
{
    LOG(FATAL, ANI) << "Not implemented vm_api, nr=" << nr;
    UNREACHABLE();
}

template <int NR, typename R, typename... Args>
static R NotImplementedAdapter([[maybe_unused]] Args... args)
{
    NotImplementedAPI(NR);
}

// clang-format off
const __ani_vm_api VM_API = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    NotImplementedAdapter<4>,
    GetEnv,
    NotImplementedAdapter<6>,
    NotImplementedAdapter<7>,
};
// clang-format on

const __ani_vm_api *GetVMAPI()
{
    return &VM_API;
}
}  // namespace ark::ets::ani
