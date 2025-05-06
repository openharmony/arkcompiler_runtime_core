/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACES_THREAD_THREAD_HOLDER_MANAGER_H
#define COMMON_INTERFACES_THREAD_THREAD_HOLDER_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <list>
#include <mutex>

#include "base/common.h"
#include "thread/thread_holder.h"

namespace panda {

class ThreadHolderManager final {
public:
    ThreadHolderManager() = default;
    ~ThreadHolderManager() = default;

    NO_COPY_SEMANTIC_CC(ThreadHolderManager);
    NO_MOVE_SEMANTIC_CC(ThreadHolderManager);

    void RegisterThreadHolder(ThreadHolder *holder);
    void UnregisterThreadHolder(ThreadHolder *holder);

    void SuspendAll(ThreadHolder *current);
    void ResumeAll(ThreadHolder *current);
    void IterateAll(CommonRootVisitor visitor);

private:
    void SuspendAllImpl(ThreadHolder *current);
    void ResumeAllImpl(ThreadHolder *current);
};

}  // namespace panda
#endif  // COMMON_INTERFACES_THREAD_THREAD_HOLDER_MANAGER_H
