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

#include "plugins/ets/runtime/dfx/async_stack_helper.h"

#include <dlfcn.h>

#include "runtime/include/runtime.h"

namespace ark::ets::dfx {
namespace {

constexpr uint64_t ARKTS_STA_LAUNCH_DFX_TYPE = 1ULL << 26ULL;

uint64_t MapToDfxType([[maybe_unused]] ark::dfx::StackType stackType)
{
    ASSERT(stackType == ark::dfx::StackType::STACK_TYPE_LAUNCH);
    return ARKTS_STA_LAUNCH_DFX_TYPE;
}

}  // namespace

void OhosAsyncStackHelper::Initialize()
{
    // libasync_stack.z.so has dlopen in appspawn, so dlsym directly.
    collectAsyncStack_ = reinterpret_cast<CollectAsyncStackFunc>(dlsym(RTLD_DEFAULT, "DfxCollectStackWithDepth"));
    setStackId_ = reinterpret_cast<SetStackIdFunc>(dlsym(RTLD_DEFAULT, "DfxSetSubmitterStackId"));
    getStackId_ = reinterpret_cast<GetStackIdFunc>(dlsym(RTLD_DEFAULT, "DfxGetSubmitterStackId"));
}

bool OhosAsyncStackHelper::IsLoaded() const
{
    return collectAsyncStack_ != nullptr && setStackId_ != nullptr && getStackId_ != nullptr;
}

bool OhosAsyncStackHelper::CheckLoadDfxAsyncStackFunc() const
{
    if (!IsLoaded()) {
        LOG(DEBUG, DFX) << "DfxAsyncStackFunc failed to load.";
        return false;
    }
    return true;
}

uint64_t OhosAsyncStackHelper::CollectAsyncStack(ark::dfx::StackType stackType, size_t depth) const
{
    if (!IsLoaded()) {
        LOG(DEBUG, DFX) << "DfxCollectStackWithDepth is not loaded.";
        return 0U;
    }
    auto type = MapToDfxType(stackType);
    uint64_t stackId = collectAsyncStack_(type, depth);
    LOG(DEBUG, DFX) << "CollectAsyncStack: type=" << type << " depth=" << depth << " stackId=" << stackId;
    return stackId;
}

void OhosAsyncStackHelper::SetStackId(uint64_t id) const
{
    if (setStackId_ != nullptr) {
        setStackId_(id);
    }
    LOG(DEBUG, DFX) << "SetStackId: " << id;
}

uint64_t OhosAsyncStackHelper::GetStackId() const
{
    uint64_t id = getStackId_ == nullptr ? 0U : getStackId_();
    LOG(DEBUG, DFX) << "GetStackId: " << id;
    return id;
}

}  // namespace ark::ets::dfx
