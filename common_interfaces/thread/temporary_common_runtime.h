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

#ifndef COMMON_INTERFACES_THREAD_TEMPORARY_COMMON_RUNTIME_H
#define COMMON_INTERFACES_THREAD_TEMPORARY_COMMON_RUNTIME_H

#include "thread/thread_holder_manager.h"

#include "libpandabase/macros.h"

namespace panda::commoninterfaces {

class TemporaryCommonRuntime final {
public:
    static void CreateInstance();
    static void DestroyInstance();
    static TemporaryCommonRuntime *GetInstance();

    ThreadHolderManager *GetThreadHolderManager() const
    {
        return threadHolderManager_;
    }

private:
    TemporaryCommonRuntime();
    ~TemporaryCommonRuntime() = default;

    static TemporaryCommonRuntime *instance_;

    ThreadHolderManager *threadHolderManager_ {nullptr};

    NO_COPY_SEMANTIC(TemporaryCommonRuntime);
    NO_MOVE_SEMANTIC(TemporaryCommonRuntime);
};

}  // namespace panda::commoninterfaces
#endif  // COMMON_INTERFACES_THREAD_TEMPORARY_COMMON_RUNTIME_H
