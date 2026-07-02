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
#ifndef COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_SPACE_REGIONAL_SPACE_H
#define COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_SPACE_REGIONAL_SPACE_H

#include <assert.h>
#include "runtime/mem/gc/cmc/heap/allocator/region_manager.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/mutator.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace ark::common_vm {
class RegionalSpace {
public:
    RegionalSpace(RegionManager &regionManager) : regionManager_(regionManager) {}

    RegionManager &GetRegionManager()
    {
        return regionManager_;
    }

protected:
    void ClearGCInfo(RegionList &list)
    {
        RegionList tmp("temp region list");
        list.CopyListTo(tmp);
        tmp.VisitAllRegions([](RegionDesc *region) {
            region->ClearMarkingCopyLine();
            region->ClearLiveInfo();
            region->ResetMarkBit();
        });
    }

    void InitRegionPhaseLine(RegionDesc *region)
    {
        if (region == nullptr) {
            return;
        }
        mem::GCPhase phase = PandaVM::GetCurrent()->GetGC()->GetGCPhase();
        if (phase == mem::GCPhase::GC_PHASE_INITIAL_MARK || phase == mem::GCPhase::GC_PHASE_MARK) {
            region->SetMarkingLine();
        } else if (phase == mem::GCPhase::GC_PHASE_PRECOPY || phase == mem::GCPhase::GC_PHASE_COPY ||
                   phase == mem::GCPhase::GC_PHASE_FIX || phase == mem::GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE) {
            ASSERT_PRINT(phase != mem::GCPhase::GC_PHASE_REMARK, "remark phase should not set copy line");
            region->SetMarkingLine();
            region->SetCopyLine();
        }
    }

    RegionManager &regionManager_;
};
}  // namespace ark::common_vm
#endif  // COMMON_RUNTIME_COMMON_COMPONENTS_HEAP_SPACE_REGIONAL_SPACE_H
