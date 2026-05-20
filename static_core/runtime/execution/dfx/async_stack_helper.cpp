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

#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
#include <dlfcn.h>
#endif

#include "runtime/include/runtime.h"

namespace ark::dfx {

#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
namespace {

constexpr uint64_t ARKTS_STA_LAUNCH_DFX_TYPE = 1ULL << 26ULL;

uint64_t MapToDfxType(StackType stackType)
{
    ASSERT(stackType == StackType::STACK_TYPE_LAUNCH);
    static_cast<void>(stackType);
    return ARKTS_STA_LAUNCH_DFX_TYPE;
}

}  // namespace
#endif

void AsyncStackHelper::Initialize()
{
#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
    // libasync_stack.z.so has dlopen in appspawn, so dlsym directly.
    collectAsyncStack_ = reinterpret_cast<CollectAsyncStackFunc>(dlsym(RTLD_DEFAULT, "DfxCollectStackWithDepth"));
    setStackId_ = reinterpret_cast<SetStackIdFunc>(dlsym(RTLD_DEFAULT, "DfxSetSubmitterStackId"));
    getStackId_ = reinterpret_cast<GetStackIdFunc>(dlsym(RTLD_DEFAULT, "DfxGetSubmitterStackId"));
#else
    LOG(DEBUG, COROUTINES) << "DFX async stack is not available in this build.";
#endif
}

bool AsyncStackHelper::IsLoaded() const
{
#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
    return collectAsyncStack_ != nullptr && setStackId_ != nullptr && getStackId_ != nullptr;
#else
    return false;
#endif
}

void AsyncStackHelper::CheckLoadDfxAsyncStackFunc() const
{
#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
    if (!IsLoaded()) {
        LOG(DEBUG, COROUTINES) << "DfxAsyncStackFunc failed to load.";
    }
#else
    LOG(DEBUG, COROUTINES) << "DfxAsyncStackFunc is not available in this build.";
#endif
}

uint64_t AsyncStackHelper::CollectAsyncStack(StackType stackType, size_t depth) const
{
    ASSERT(stackType == StackType::STACK_TYPE_LAUNCH);
#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
    if (!IsLoaded()) {
        LOG(DEBUG, COROUTINES) << "DfxCollectStackWithDepth is not loaded.";
        return 0U;
    }
    auto type = MapToDfxType(stackType);
    uint64_t stackId = collectAsyncStack_(type, depth);
    LOG(DEBUG, COROUTINES) << "CollectAsyncStack: type=" << type << " depth=" << depth << " stackId=" << stackId;
    return stackId;
#else
    static_cast<void>(stackType);
    static_cast<void>(depth);
    return 0U;
#endif
}

void AsyncStackHelper::SetStackId(uint64_t id) const
{
#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
    if (setStackId_ != nullptr) {
        setStackId_(id);
    }
    LOG(DEBUG, COROUTINES) << "SetStackId: " << id;
#else
    static_cast<void>(id);
#endif
}

uint64_t AsyncStackHelper::GetStackId() const
{
#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
    uint64_t id = getStackId_ == nullptr ? 0U : getStackId_();
    LOG(DEBUG, COROUTINES) << "GetStackId: " << id;
    return id;
#else
    return 0U;
#endif
}

}  // namespace ark::dfx
