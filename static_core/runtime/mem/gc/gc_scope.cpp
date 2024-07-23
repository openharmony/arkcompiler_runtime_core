/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "libpandabase/mem/mem.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_scope.h"

namespace ark::mem {
GCScope<TRACE_TIMING>::GCScope(std::string_view name, GC *gc)
    : trace::ScopedTrace(name.data()), ScopedTiming(name, *gc->GetTiming())
{
}

GCScope<TRACE_TIMING_PHASE>::GCScope(std::string_view name, GC *gc, GCPhase phase)
    : trace::ScopedTrace(name.data()), ScopedTiming(name, *gc->GetTiming()), GCScopedPhase(gc, phase)
{
}

GCScope<TIMING_PHASE>::GCScope(std::string_view name, GC *gc, GCPhase phase)
    : ScopedTiming(name, *gc->GetTiming()), GCScopedPhase(gc, phase)
{
}

GCScope<TRACE_PHASE>::GCScope(std::string_view name, GC *gc, GCPhase phase)
    : trace::ScopedTrace(name.data()), GCScopedPhase(gc, phase)
{
}
}  // namespace ark::mem
