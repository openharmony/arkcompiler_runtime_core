/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_CODEGEN_METHOD_PROPERTIES_H
#define COMPILER_OPTIMIZER_CODEGEN_METHOD_PROPERTIES_H

#include <cstddef>
#include <iostream>
#include "libpandabase/mem/arena_allocator.h"
#include "libpandabase/utils/bit_field.h"

namespace panda::compiler {
class Graph;

class MethodProperties {
public:
    explicit MethodProperties(const Graph *graph);
    MethodProperties(const MethodProperties &) = default;
    MethodProperties &operator=(const MethodProperties &) = default;
    MethodProperties(MethodProperties &&) = default;
    MethodProperties &operator=(MethodProperties &&) = default;
    ~MethodProperties() = default;

    static MethodProperties *Create(ArenaAllocator *arena_allocator, const Graph *graph)
    {
        return arena_allocator->New<MethodProperties>(graph);
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MPROP_GET_FIELD(name, type) \
    type Get##name() const          \
    {                               \
        return name::Get(fields_);  \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MPROP_SET_FIELD(name, type) \
    void Set##name(type val)        \
    {                               \
        name::Set(val, &fields_);   \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MPROP_FIELD(name, type) \
    MPROP_GET_FIELD(name, type) \
    MPROP_SET_FIELD(name, type)

    using FieldsTy = uint32_t;

    MPROP_FIELD(CanThrow, bool);
    MPROP_FIELD(HasCalls, bool);
    MPROP_FIELD(HasLibCalls, bool);
    MPROP_FIELD(HasRuntimeCalls, bool);
    MPROP_FIELD(HasSafepoints, bool);
    MPROP_FIELD(HasRequireState, bool);
    MPROP_FIELD(HasDeopt, bool);
    MPROP_FIELD(HasParamsOnStack, bool);
    MPROP_FIELD(CompactPrologueAllowed, bool);
    MPROP_FIELD(RequireFrameSetup, bool);

    using CanThrow = BitField<bool, 0, 1>;
    using HasCalls = CanThrow::NextFlag;
    using HasLibCalls = HasCalls::NextFlag;
    using HasRuntimeCalls = HasLibCalls::NextFlag;
    using HasSafepoints = HasRuntimeCalls::NextFlag;
    using HasRequireState = HasSafepoints::NextFlag;
    using HasDeopt = HasRequireState::NextFlag;
    using HasParamsOnStack = HasDeopt::NextFlag;
    using CompactPrologueAllowed = HasParamsOnStack::NextFlag;
    using RequireFrameSetup = CompactPrologueAllowed::NextFlag;

    bool IsLeaf() const
    {
        return !GetHasCalls() && !GetHasRuntimeCalls() && !GetHasLibCalls() && !GetHasRequireState() && !GetCanThrow();
    }

    Inst *GetLastReturn() const
    {
        return last_return_;
    }

#undef MPROP_GET_FIELD
#undef MPROP_SET_FIELD
#undef MPROP_FIELD

private:
    Inst *last_return_ {nullptr};
    FieldsTy fields_ {0};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_CODEGEN_METHOD_PROPERTIES_H
