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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_LEGACY_ATOMICS_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_LEGACY_ATOMICS_H

#include "plugins/ets/runtime/types/ets_arraybuffer.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_promise.h"

namespace ark::ets {

class LegacyAtomics {
public:
    static constexpr uint32_t CONTENTION_TABLE_SIZE = (1U << 8U);

    LegacyAtomics() = delete;
    ~LegacyAtomics() = delete;

    NO_COPY_SEMANTIC(LegacyAtomics);
    NO_MOVE_SEMANTIC(LegacyAtomics);

    static EtsString *WaitInt32(EtsStdCoreArrayBuffer *buffer, int32_t index, int32_t value, int32_t timeout);

    static EtsString *WaitInt64(EtsStdCoreArrayBuffer *buffer, int32_t index, int64_t value, int32_t timeout);

    static EtsPromise *AsyncWaitInt32(EtsStdCoreArrayBuffer *buffer, int32_t index, int32_t value, int32_t timeout);

    static EtsPromise *AsyncWaitInt64(EtsStdCoreArrayBuffer *buffer, int32_t index, int64_t value, int32_t timeout);

    static int32_t Notify(EtsStdCoreArrayBuffer *buffer, int32_t index, int32_t count);
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_LEGACY_ATOMICS_H
