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

#include "intrinsics.h"

namespace ark::ets::intrinsics {

void AsyncWorkNativeInvoke(int64_t nativeCbPtr, int64_t dataPtr)
{
    ani_env *env = EtsExecutionContext::GetCurrent()->GetPandaAniEnv();
    auto *nativeCb = reinterpret_cast<void (*)(ani_env *, void *)>(nativeCbPtr);

    ScopedNativeCodeThread sn(ManagedThread::GetCurrent());

    constexpr ani_size MAX_LOCAL_REF = 4096;
    if (env->CreateLocalScope(MAX_LOCAL_REF) != ANI_OK) {
        LOG(FATAL, RUNTIME) << "AsyncWorkNativeInvoke CreateLocalScope failed";
    }

    nativeCb(env, reinterpret_cast<void *>(dataPtr));

    env->DestroyLocalScope();
}

void EventNativeInvoke(int64_t nativeCbPtr, int64_t dataPtr)
{
    auto *nativeCb = reinterpret_cast<void (*)(void *)>(nativeCbPtr);
    PandaAniEnv *env = EtsExecutionContext::GetCurrent()->GetPandaAniEnv();

    ScopedNativeCodeThread sn(ManagedThread::GetCurrent());

    constexpr ani_size MAX_LOCAL_REF = 4096;
    if (env->CreateLocalScope(MAX_LOCAL_REF) != ANI_OK) {
        LOG(FATAL, RUNTIME) << "EventNativeInvoke CreateLocalScope failed";
    }

    nativeCb(reinterpret_cast<void *>(dataPtr));

    env->DestroyLocalScope();
}

}  // namespace ark::ets::intrinsics
