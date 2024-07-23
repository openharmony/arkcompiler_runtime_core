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

#ifndef PANDA_RUNTIME_MEM_GC_GC_SCOPED_FUNC_H
#define PANDA_RUNTIME_MEM_GC_GC_SCOPED_FUNC_H

#include "libpandabase/trace/trace.h"
#include "runtime/mem/gc/gc_scoped_phase.h"
#include "runtime/timing.h"

namespace ark::mem {
// Forward declaration
class GC;

enum GCScopeType { TRACE_TIMING, TRACE_TIMING_PHASE, TIMING_PHASE, TRACE_PHASE };

template <GCScopeType GC_SCOPE_TYPE>
class GCScope {
public:
    GCScope() = delete;
    NO_COPY_SEMANTIC(GCScope);
    NO_MOVE_SEMANTIC(GCScope);
    ~GCScope() = default;
};

template <>
class GCScope<TRACE_TIMING> : public trace::ScopedTrace, public ScopedTiming {  // NOLINT(fuchsia-multiple-inheritance)
public:
    GCScope(std::string_view name, GC *gc);
    NO_COPY_SEMANTIC(GCScope);
    NO_MOVE_SEMANTIC(GCScope);
    ~GCScope() = default;
};

template <>
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class GCScope<TRACE_TIMING_PHASE> : public trace::ScopedTrace, public ScopedTiming, public GCScopedPhase {
public:
    GCScope(std::string_view name, GC *gc, GCPhase phase);
    NO_COPY_SEMANTIC(GCScope);
    NO_MOVE_SEMANTIC(GCScope);
    ~GCScope() = default;
};

template <>
class GCScope<TIMING_PHASE> : public ScopedTiming, public GCScopedPhase {  // NOLINT(fuchsia-multiple-inheritance)
public:
    GCScope(std::string_view name, GC *gc, GCPhase phase);
    NO_COPY_SEMANTIC(GCScope);
    NO_MOVE_SEMANTIC(GCScope);
    ~GCScope() = default;
};

template <>
class GCScope<TRACE_PHASE> : public trace::ScopedTrace, public GCScopedPhase {  // NOLINT(fuchsia-multiple-inheritance)
public:
    GCScope(std::string_view name, GC *gc, GCPhase phase);
    NO_COPY_SEMANTIC(GCScope);
    NO_MOVE_SEMANTIC(GCScope);
    ~GCScope() = default;
};
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_SCOPED_FUNC_H
