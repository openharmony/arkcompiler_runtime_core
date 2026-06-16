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

#include "runtime/include/oom_stats.h"

#include "libarkbase/utils/logger.h"

namespace ark::oom_stats {

void OomNotifier::NotifyBeforeManagedOom(size_t heapLimitBytes, size_t activeMemoryBytes, size_t failedAllocBytes,
                                         [[maybe_unused]] const std::string &processName,
                                         [[maybe_unused]] SpaceType spaceType)
{
    LOG(INFO, RUNTIME) << "OOM notify (stub): heapLimit=" << heapLimitBytes << ", activeMemory=" << activeMemoryBytes
                       << ", failedAlloc=" << failedAllocBytes;
}

}  // namespace ark::oom_stats
