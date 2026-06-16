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

#ifndef PANDA_RUNTIME_INCLUDE_OOM_STATS_H_
#define PANDA_RUNTIME_INCLUDE_OOM_STATS_H_

#include "libarkbase/macros.h"
#include "libarkbase/mem/space.h"

#include <string>

namespace ark::oom_stats {

/**
 * Notifies the host environment (e.g. OHOS HiSysEvent) before the runtime throws or surfaces a managed OOM.
 * This is not part of the garbage collector; it is VM / platform DFX only.
 */
class OomNotifier {
public:
    OomNotifier() = delete;
    ~OomNotifier() = delete;

    static void NotifyBeforeManagedOom(size_t heapLimitBytes, size_t activeMemoryBytes, size_t failedAllocBytes,
                                       [[maybe_unused]] const std::string &processName,
                                       [[maybe_unused]] SpaceType spaceType);

private:
    NO_COPY_SEMANTIC(OomNotifier);
    NO_MOVE_SEMANTIC(OomNotifier);
};

}  // namespace ark::oom_stats

#endif  // PANDA_RUNTIME_INCLUDE_OOM_STATS_H_
