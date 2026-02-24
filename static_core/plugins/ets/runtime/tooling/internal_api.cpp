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

#include "plugins/ets/runtime/tooling/internal_api.h"

extern "C" PANDA_PUBLIC_API int ArkParseArkFrameInfoLocal(ark::tooling::LocalTrace localTracePtr, uintptr_t byteCodePc,
                                                          uintptr_t mapBase, uintptr_t loadOffset,
                                                          ark::tooling::Function *function)
{
    ASSERT(localTracePtr != nullptr);
    auto localTrace = reinterpret_cast<ark::tooling::LocalStackTrace *>(localTracePtr);
    if (localTrace->GetArkFrameInfo(byteCodePc, mapBase, loadOffset, function)) {
        return 1;
    }
    return -1;
}

extern "C" PANDA_PUBLIC_API void ArkCreateLocalStackTrace(ark::tooling::LocalTrace *localTracePtr)
{
    ASSERT(localTracePtr != nullptr);
    auto localTrace = ark::tooling::LocalStackTrace::Create();
    *localTracePtr = reinterpret_cast<ark::tooling::LocalTrace>(localTrace);
}

extern "C" PANDA_PUBLIC_API void ArkDestroyLocalStackTrace(ark::tooling::LocalTrace localTracePtr)
{
    ASSERT(localTracePtr != nullptr);
    auto localTrace = reinterpret_cast<ark::tooling::LocalStackTrace *>(localTracePtr);
    ark::tooling::LocalStackTrace::Destroy(localTrace);
}
