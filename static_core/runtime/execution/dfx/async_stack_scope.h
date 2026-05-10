/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_EXECUTION_DFX_ASYNC_STACK_SCOPE_H
#define PANDA_RUNTIME_EXECUTION_DFX_ASYNC_STACK_SCOPE_H

#include "libarkbase/macros.h"
#include "runtime/execution/dfx/async_stack_helper.h"

namespace ark {

class Job;

namespace dfx {

class AsyncStackScope {
public:
    explicit AsyncStackScope(Job *job, AsyncStackHelper &asyncStackHelper);

    ~AsyncStackScope();

    NO_COPY_SEMANTIC(AsyncStackScope);
    NO_MOVE_SEMANTIC(AsyncStackScope);

private:
    AsyncStackHelper *asyncStackHelper_;
    bool hasSetStackId_ = false;
    uint64_t prevID_ = 0U;
};

}  // namespace dfx
}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_DFX_ASYNC_STACK_SCOPE_H
