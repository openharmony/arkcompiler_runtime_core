/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "libarkbase/utils/logger.h"
#include "runtime/mark_word.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/cmc-gc-adapter/cmc-gc-adapter.h"
#if defined(ARK_USE_COMMON_RUNTIME)
#include "common_interfaces/base_runtime.h"
#endif  // ARK_USE_COMMON_RUNTIME

namespace ark::mem {
template <class LanguageConfig>
CMCGCAdapter<LanguageConfig>::CMCGCAdapter(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
    : GCLang<LanguageConfig>(objectAllocator, settings)
{
    this->SetType(GCType::CMC_GC);
    this->SetTLABsSupported();
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::InitializeImpl()
{
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    auto barrierSet = allocator->New<GCCMCBarrierSet>(allocator);
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
#if defined(ARK_USE_COMMON_RUNTIME)
    common_vm::GCReason reason = common_vm::GCReason::GC_REASON_INVALID;
    common_vm::GCType type = common_vm::GCType::GC_TYPE_FULL;
    switch (task.reason) {
        case GCTaskCause::YOUNG_GC_CAUSE:
            reason = common_vm::GCReason::GC_REASON_YOUNG;
            type = common_vm::GCType::GC_TYPE_YOUNG;
            break;
        case GCTaskCause::PYGOTE_FORK_CAUSE:
            reason = common_vm::GCReason::GC_REASON_USER;
            break;
        case GCTaskCause::STARTUP_COMPLETE_CAUSE:
            reason = common_vm::GCReason::GC_REASON_HINT;
            break;
        case GCTaskCause::NATIVE_ALLOC_CAUSE:
            reason = common_vm::GCReason::GC_REASON_NATIVE;
            break;
        case GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE:
        case GCTaskCause::MIXED:
            reason = common_vm::GCReason::GC_REASON_HEU;
            break;
        case GCTaskCause::EXPLICIT_CAUSE:
            reason = common_vm::GCReason::GC_REASON_FORCE;
            break;
        case GCTaskCause::OOM_CAUSE:
            reason = common_vm::GCReason::GC_REASON_OOM;
            break;
        case GCTaskCause::CROSSREF_CAUSE:
            reason = common_vm::GCReason::GC_REASON_BACKUP;
            break;
        default:
            UNREACHABLE();
    }
    common_vm::BaseRuntime::RequestGC(reason, false, type);
#endif  // ARK_USE_COMMON_RUNTIME
    return false;
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::InitGCBits([[maybe_unused]] ObjectHeader *objHeader)
{
    constexpr ClassHelper::ClassWordSize STATIC_OBJECT_MASK = static_cast<ClassHelper::ClassWordSize>(
        static_cast<uint64_t>(common_vm::LanguageType::STATIC)
        << (common_vm::BaseStateWord::BASECLASS_WIDTH + common_vm::BaseStateWord::PADDING_WIDTH));
    auto *classWord =
        reinterpret_cast<ClassHelper::ClassWordSize *>(ToUintPtr(objHeader) + ObjectHeader::GetClassOffset());
    *classWord |= STATIC_OBJECT_MASK;
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::InitGCBitsForAllocationInTLAB([[maybe_unused]] ObjectHeader *objHeader)
{
    LOG(FATAL, GC) << "TLABs are not supported by this GC";
}

template <class LanguageConfig>
bool CMCGCAdapter<LanguageConfig>::Trigger([[maybe_unused]] PandaUniquePtr<GCTask> task)
{
#if defined(ARK_USE_COMMON_RUNTIME)
    common_vm::BaseRuntime::RequestGC(common_vm::GCReason::GC_REASON_OOM, false, common_vm::GCType::GC_TYPE_FULL);
#endif  // ARK_USE_COMMON_RUNTIME
    return false;
}

template <class LanguageConfig>
bool CMCGCAdapter<LanguageConfig>::IsPostponeGCSupported() const
{
    return true;
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::StopGC()
{
#if defined(ARK_USE_COMMON_RUNTIME)
    common_vm::BaseRuntime::StopGCWork();
#endif  // ARK_USE_COMMON_RUNTIME
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
