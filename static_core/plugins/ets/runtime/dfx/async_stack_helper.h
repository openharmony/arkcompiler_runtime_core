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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_DFX_ASYNC_STACK_HELPER_H
#define PANDA_PLUGINS_ETS_RUNTIME_DFX_ASYNC_STACK_HELPER_H

#include "runtime/execution/dfx/async_stack_helper.h"

namespace ark::ets::dfx {

class OhosAsyncStackHelper final : public ark::dfx::AsyncStackHelper {
public:
    OhosAsyncStackHelper() = default;
    ~OhosAsyncStackHelper() override = default;
    NO_COPY_SEMANTIC(OhosAsyncStackHelper);
    NO_MOVE_SEMANTIC(OhosAsyncStackHelper);

    void Initialize() override;
    bool CheckLoadDfxAsyncStackFunc() const override;
    uint64_t CollectAsyncStack(ark::dfx::StackType stackType, size_t depth) const override;
    void SetStackId(uint64_t id) const override;
    uint64_t GetStackId() const override;

private:
    using CollectAsyncStackFunc = uint64_t (*)(uint64_t, size_t);
    using SetStackIdFunc = void (*)(uint64_t);
    using GetStackIdFunc = uint64_t (*)();

    bool IsLoaded() const;

    CollectAsyncStackFunc collectAsyncStack_ {nullptr};
    SetStackIdFunc setStackId_ {nullptr};
    GetStackIdFunc getStackId_ {nullptr};
};

}  // namespace ark::ets::dfx

#endif  // PANDA_PLUGINS_ETS_RUNTIME_DFX_ASYNC_STACK_HELPER_H
