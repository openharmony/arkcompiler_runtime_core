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

#ifndef PANDA_RUNTIME_EXECUTION_DFX_ASYNC_STACK_HELPER_H
#define PANDA_RUNTIME_EXECUTION_DFX_ASYNC_STACK_HELPER_H

#include <cstddef>
#include <cstdint>

#include "libarkbase/macros.h"

namespace ark::dfx {

enum class StackType : uint8_t {
    STACK_TYPE_LAUNCH,
};

class AsyncStackHelper {
public:
    static constexpr size_t DEFAULT_STACK_DEPTH = 16U;

    AsyncStackHelper() = default;
    NO_COPY_SEMANTIC(AsyncStackHelper);
    NO_MOVE_SEMANTIC(AsyncStackHelper);
    ~AsyncStackHelper() = default;

    void Initialize();

    void CheckLoadDfxAsyncStackFunc() const;

    uint64_t CollectAsyncStack(StackType stackType, size_t depth = DEFAULT_STACK_DEPTH) const;

    void SetStackId(uint64_t id) const;

    uint64_t GetStackId() const;

private:
#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
    using CollectAsyncStackFunc = uint64_t (*)(uint64_t, size_t);
    using SetStackIdFunc = void (*)(uint64_t);
    using GetStackIdFunc = uint64_t (*)();
#endif

    bool IsLoaded() const;

#if defined(PANDA_BUILD_IN_OHOS_TREE) || defined(PANDA_TARGET_OHOS)
    CollectAsyncStackFunc collectAsyncStack_ {nullptr};
    SetStackIdFunc setStackId_ {nullptr};
    GetStackIdFunc getStackId_ {nullptr};
#endif
};

}  // namespace ark::dfx

#endif  // PANDA_RUNTIME_EXECUTION_DFX_ASYNC_STACK_HELPER_H
