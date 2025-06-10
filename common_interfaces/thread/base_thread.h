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

#ifndef COMMON_INTERFACES_THREAD_BASE_THREAD_H
#define COMMON_INTERFACES_THREAD_BASE_THREAD_H

#include "thread/thread_holder-inl.h"

#include "base/common.h"

namespace common {
// Fixme: replace with real RootVisitor
using MockRootVisitor = void (*)();

enum class BaseThreadType {
    JS_THREAD,
    STS_THREAD,
};

/**
 * BaseThread is the abstract base class for different languages
 */
class BaseThread {
public:
    explicit BaseThread(BaseThreadType type, ThreadHolder *holder) : threadType_(type), threadHolder_(holder) {}
    virtual ~BaseThread() = default;

    NO_COPY_SEMANTIC_CC(BaseThread);
    NO_MOVE_SEMANTIC_CC(BaseThread);

    BaseThreadType GetThreadType() const
    {
        return threadType_;
    }

    ThreadHolder *GetThreadHolder() const
    {
        return threadHolder_;
    }

private:
    BaseThreadType threadType_ {};
    ThreadHolder *threadHolder_ {};
};
}  // namespace common
#endif  // COMMON_INTERFACES_THREAD_BASE_THREAD_H