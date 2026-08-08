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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_OUT_OF_MEMORY_LISTENER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_OUT_OF_MEMORY_LISTENER_H_

#include "runtime/include/runtime_notification.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace ark::ets {

class PandaEtsVM;

/// Forwards OOM / allocation-failed notifications from RuntimeNotificationManager to DFX (HiSysEvent, etc.).
class EtsVmOutOfMemoryListener final : public RuntimeListener {
public:
    explicit EtsVmOutOfMemoryListener(PandaEtsVM *vm);

    void OutOfMemory(size_t size, SpaceType spaceType) override;

private:
    static constexpr size_t OOM_DUMP_RESERVE_SIZE = 1024U * 1024U;
    using OOMDumpReserve = std::array<uint8_t, OOM_DUMP_RESERVE_SIZE>;

    void TriggerOOMDump();

    PandaEtsVM *vm_;
    std::unique_ptr<OOMDumpReserve> oomDumpReserve_;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_OUT_OF_MEMORY_LISTENER_H_
