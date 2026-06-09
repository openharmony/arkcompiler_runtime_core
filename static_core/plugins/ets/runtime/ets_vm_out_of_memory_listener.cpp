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

#include "plugins/ets/runtime/ets_vm_out_of_memory_listener.h"

#include "plugins/ets/runtime/ets_vm.h"
#include "runtime/include/oom_stats.h"
#include "runtime/include/runtime.h"

#include <string>

namespace ark::ets {

void EtsVmOutOfMemoryListener::OutOfMemory(size_t size, SpaceType spaceType)
{
    mem::HeapManager *heapManager = vm_->GetHeapManager();
    auto *memStats = vm_->GetMemStats();
    const size_t activeMemory = (memStats != nullptr) ? memStats->GetFootprintHeap() : 0U;
    ark::oom_stats::OomNotifier::NotifyBeforeManagedOom(heapManager->GetMaxMemory(), activeMemory, size,
                                                        Runtime::GetOptions().GetProcessPackageName(), spaceType);
}

}  // namespace ark::ets
