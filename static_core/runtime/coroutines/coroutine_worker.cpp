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

#include "coroutines/coroutine.h"
#include "coroutines/coroutine_manager.h"
#include "coroutines/coroutine_worker.h"

namespace ark {

void CoroutineWorker::OnCoroBecameActive(Coroutine *co)
{
    if (co->GetType() == Coroutine::Type::MUTATOR && IsExternalSchedulingEnabled()) {
        auto *coroMan = co->GetManager();
// Note: Currently passing lambda to PostExternalCallback will cause a crash in Arm32
// detail infomation can be seen at #24085
#ifdef PANDA_TARGET_ARM32
        std::function<void()> callback = [coroMan]() { coroMan->Schedule(); };
        PostExternalCallback(std::move(callback));
#else
        PostExternalCallback([coroMan]() { coroMan->Schedule(); });
#endif
    }
}

}  // namespace ark
