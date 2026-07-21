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

#ifndef PANDA_COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_FLAT_CHECK_ELIMINATION
#define PANDA_COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_FLAT_CHECK_ELIMINATION

#include "optimizer/pass.h"
#include "optimizer/ir/marker.h"

namespace ark::compiler {
class Graph;
class Inst;
class User;

/**
 * Some intrinsics with String arguments can work with flat strings only. But over the course of the optimizer pipeline,
 * the users of a StringFlatCheck can change (inlining, peepholes, intrinsic rewrites, etc.). A StringFlatCheck becomes
 * useless once none of its remaining users actually require a flat string at the position the check feeds.
 *
 * For example, there is a peephole optimization to replace:
 *     Intrinsic.StdCoreStringEquals arg, NullPtr
 * with
 *     Compare EQ ref             arg, NullPtr
 *
 * Elimination rule: remove a StringFlatCheck if it has no user for which both hold:
 * 1. the user is an Intrinsic, AND
 * 2. the intrinsic's flat-string mask bit for the input index that the StringFlatCheck feeds equals 1.
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API StringFlatCheckElimination : public Optimization {
public:
    static constexpr const char *NAME = "StringFlatCheckElimination";
    explicit StringFlatCheckElimination(Graph *graph) : Optimization(graph) {}

    NO_MOVE_SEMANTIC(StringFlatCheckElimination);
    NO_COPY_SEMANTIC(StringFlatCheckElimination);
    ~StringFlatCheckElimination() override = default;

    bool RunImpl() override;

    bool IsEnable() const override;

    const char *GetPassName() const override
    {
        return NAME;
    }

    [[nodiscard]] bool IsApplied() const
    {
        return applied_;
    }

    void SetApplied()
    {
        applied_ = true;
    }

private:
    void VisitStringFlatCheck(Inst *inst);
    bool UsersRequireFlatString(Inst *inst) const;
    bool UserRequiresFlatString(User &user) const;
    bool applied_ {false};
    Marker visited_ {UNDEF_MARKER};
};
}  // namespace ark::compiler

#endif  // PANDA_COMPILER_OPTIMIZER_OPTIMIZATIONS_STRING_FLAT_CHECK_ELIMINATION
