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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTRINSIC_TIMER_IMPL_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTRINSIC_TIMER_IMPL_H

#include <cstdint>
#include <variant>

#include "libarkbase/macros.h"

namespace ark::mem {
class Reference;
}  // namespace ark::mem

namespace ark::ets::intrinsics::helpers {

using TimerId = int32_t;

class ManagedEntrypoint {
public:
    explicit ManagedEntrypoint(mem::Reference *entry) : entrypoint_(entry)
    {
        ASSERT(entry != nullptr);
    }

    NO_COPY_SEMANTIC(ManagedEntrypoint);
    ManagedEntrypoint(ManagedEntrypoint &&epInfo);
    ManagedEntrypoint &operator=(ManagedEntrypoint &&epInfo);
    ~ManagedEntrypoint();

    void Invoke() const;

private:
    mem::Reference *entrypoint_;
};

class NativeEntrypoint {
public:
    using NativeEntrypointFunc = void (*)(void *);

    explicit NativeEntrypoint(NativeEntrypointFunc entry, void *data) : entrypoint_(entry), param_(data)
    {
        ASSERT(entry != nullptr);
    }

    NO_COPY_SEMANTIC(NativeEntrypoint);
    DEFAULT_MOVE_SEMANTIC(NativeEntrypoint);
    ~NativeEntrypoint() = default;

    void Invoke() const;

private:
    NativeEntrypointFunc entrypoint_;
    void *param_;
};

using EntrypointInfo = std::variant<ManagedEntrypoint, NativeEntrypoint>;

TimerId CreateTimer(EntrypointInfo entrypoint, int32_t delay, bool periodic);

void StopTimer(TimerId timerId);

}  // namespace ark::ets::intrinsics::helpers

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTRINSIC_TIMER_IMPL_H
