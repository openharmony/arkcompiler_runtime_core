/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "common_components/heap/collector/collector.h"

#include "common_components/heap/heap.h"
#include "common_interfaces/thread/mutator.h"

namespace ark::common_vm {

Collector::Collector() {}

Collector::~Collector() {}

void Collector::RequestGC(GCTaskCause reason, bool async, GCCollectionType gcType, bool explicitRequest)
{
    RequestGCInternal(reason, async, gcType, explicitRequest);
    return;
}

bool Collector::RegisterVM(VMInterface *vm)
{
    ark::os::memory::WriteLockHolder vmIfacesWriteLock(vmIfacesLock_);
    auto res = vmIfaces_.insert(vm);
    return res.second;
}

bool Collector::UnregisterVM(VMInterface *vm)
{
    ark::os::memory::WriteLockHolder vmIfacesWriteLock(vmIfacesLock_);
    auto erased = vmIfaces_.erase(vm);
    return erased != 0;
}

void Collector::ForEachVM(std::function<void(VMInterface *)> action)
{
    ark::os::memory::ReadLockHolder vmIfacesReadLock(vmIfacesLock_);
    std::for_each(vmIfaces_.begin(), vmIfaces_.end(), action);
}
}  // namespace ark::common_vm.
