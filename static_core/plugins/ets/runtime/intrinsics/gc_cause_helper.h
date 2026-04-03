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

// Helper for GC cause integer conversion, shared between Default GC and CMC GC intrinsics

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_GC_CAUSE_HELPER_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_GC_CAUSE_HELPER_H

#include "runtime/include/gc_task.h"
#include "libarkbase/utils/utils.h"

namespace ark::ets::intrinsics {
// GCCauseFromInt is used in std_core_gc.cpp and std_core_default_gc.cpp
static GCTaskCause GCCauseFromInt(EtsInt cause)
{
    if (cause == 0_I) {
        return GCTaskCause::YOUNG_GC_CAUSE;
    }
    if (cause == 1_I) {
        return GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE;
    }
    if (cause == 2_I) {
        return GCTaskCause::OOM_CAUSE;
    }
    UNREACHABLE();
    return GCTaskCause::INVALID_CAUSE;
}

}  // namespace ark::ets::intrinsics

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTRINSICS_GC_CAUSE_HELPER_H
