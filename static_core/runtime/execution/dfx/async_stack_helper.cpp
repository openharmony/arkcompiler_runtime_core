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

#include "runtime/execution/dfx/async_stack_helper.h"

#include "runtime/include/runtime.h"

namespace ark::dfx {

void AsyncStackHelper::Initialize()
{
    LOG(DEBUG, DFX) << "DFX async stack is not available in this build.";
}

bool AsyncStackHelper::CheckLoadDfxAsyncStackFunc() const
{
    LOG(DEBUG, DFX) << "DfxAsyncStackFunc is not available in this build.";
    return false;
}

uint64_t AsyncStackHelper::CollectAsyncStack([[maybe_unused]] StackType stackType, [[maybe_unused]] size_t depth) const
{
    ASSERT(stackType == StackType::STACK_TYPE_LAUNCH);
    return 0U;
}

void AsyncStackHelper::SetStackId([[maybe_unused]] uint64_t id) const {}

uint64_t AsyncStackHelper::GetStackId() const
{
    return 0U;
}

}  // namespace ark::dfx
