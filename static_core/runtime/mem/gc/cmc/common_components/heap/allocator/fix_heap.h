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

#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_FIX_HEAP_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_ALLOCATOR_FIX_HEAP_H

#include <stack>
#include "runtime/include/mem/panda_containers.h"

namespace ark::common_vm {

class Collector;
}  // namespace ark::common_vm

namespace ark::mem {
class RegionDesc;
}  // namespace ark::mem

namespace ark::common_vm {
using ark::mem::RegionDesc;
class RegionList;
class BaseObject;

enum class FixRegionType {
    FIX_OLD_REGION,         // Fix all rset objects
    FIX_RECENT_OLD_REGION,  // Fix all rset objects before copyline
    FIX_RECENT_REGION,      // Fix all objects before copyline
    FIX_REGION,             // Fix all survived objects
    FIX_TO_REGION,          // Fix objects in to region
};

struct FixRegionResult {
    ark::PandaVector<std::tuple<RegionDesc *, BaseObject *, size_t>> monoSizeNonMovableGarbages;
    ark::PandaVector<std::pair<BaseObject *, size_t>> polySizeNonMovableGarbages;
    size_t numProcessedRegions = 0;
};

class FixTaskInfo {
public:
    FixTaskInfo(RegionDesc *region, common_vm::FixRegionType type) : region_(region), type_(type) {}

    RegionDesc *GetRegion() const
    {
        return region_;
    }

    common_vm::FixRegionType GetType() const
    {
        return type_;
    }

private:
    RegionDesc *region_ {nullptr};
    common_vm::FixRegionType type_;
};

using FixHeapTaskList = ark::PandaVector<FixTaskInfo>;

/// Worker class for parallel heap fixing operations
class FixHeap {
public:
    /// Collect fix heap tasks from a region list
    static void CollectFixHeapTasks(FixHeapTaskList &taskList, RegionList &regionList, common_vm::FixRegionType type);
};

/// Worker class for collecting garbages units after heap fixing operations
class PostFixHeap {
public:
    // During fix phase we also collect the entire empty regions into garbage list from non-movable region.
    // However, we can only do it during post-fix because those region can contains metadata for getObjectSize
    // NOTE(d.chikunov) - make it PandaStack in future
    static std::stack<std::pair<RegionList *, RegionDesc *>> emptyRegionsToCollect;
    static void AddEmptyRegionToCollectDuringPostFix(RegionList *list, RegionDesc *region);
    static size_t CollectEmptyRegions();
};

};  // namespace ark::common_vm
#endif
