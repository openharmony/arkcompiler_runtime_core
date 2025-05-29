/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "libpandabase/utils/logger.h"
#include "runtime/mark_word.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/cmc-gc-adapter/cmc-gc-adapter.h"

namespace ark::mem {
template <class LanguageConfig>
CMCGCAdapter<LanguageConfig>::CMCGCAdapter(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
    : GCLang<LanguageConfig>(objectAllocator, settings)
{
    this->SetType(GCType::CMC_GC);
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::InitializeImpl()
{
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    auto barrierSet = allocator->New<GCDummyBarrierSet>(allocator);
    ASSERT(barrierSet != nullptr);
    this->SetGCBarrierSet(barrierSet);
    LOG(DEBUG, GC) << "CMC GC adapter initialized...";
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::RunPhasesImpl([[maybe_unused]] GCTask &task)
{
    LOG(DEBUG, GC) << "CMC GC adapter RunPhases...";
    GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats());
}

// NOLINTNEXTLINE(misc-unused-parameters)
template <class LanguageConfig>
bool CMCGCAdapter<LanguageConfig>::WaitForGC([[maybe_unused]] GCTask task)
{
    return false;
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::InitGCBits([[maybe_unused]] ObjectHeader *objHeader)
{
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::InitGCBitsForAllocationInTLAB([[maybe_unused]] ObjectHeader *objHeader)
{
    LOG(FATAL, GC) << "TLABs are not supported by this GC";
}

template <class LanguageConfig>
bool CMCGCAdapter<LanguageConfig>::Trigger([[maybe_unused]] PandaUniquePtr<GCTask> task)
{
    return false;
}

template <class LanguageConfig>
bool CMCGCAdapter<LanguageConfig>::IsPostponeGCSupported() const
{
    return true;
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::MarkObject([[maybe_unused]] ObjectHeader *object)
{
}

template <class LanguageConfig>
bool CMCGCAdapter<LanguageConfig>::IsMarked([[maybe_unused]] const ObjectHeader *object) const
{
    return false;
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::MarkReferences([[maybe_unused]] GCMarkingStackType *references,
                                                  [[maybe_unused]] GCPhase gcPhase)
{
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(CMCGCAdapter);

}  // namespace ark::mem
