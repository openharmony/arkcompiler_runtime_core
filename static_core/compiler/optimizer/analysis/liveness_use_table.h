/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_USE_TABLE_H
#define COMPILER_OPTIMIZER_ANALYSIS_USE_TABLE_H

#include "utils/arena_containers.h"
#include "optimizer/ir/inst.h"

namespace ark::compiler {
using FixedUses = ArenaMap<LifeNumber, Register>;

/// For each added instruction holds its uses on the fixed locations
class UseTable {
public:
    explicit UseTable(ArenaAllocator *allocator);

    void AddUseOnFixedLocation(const Inst *inst, Location location, LifeNumber ln);
    bool HasUseOnFixedLocation(const Inst *inst, LifeNumber ln) const;
    Register GetNextUseOnFixedLocation(const Inst *inst, LifeNumber ln) const;

    void Dump(std::ostream &out, Arch arch) const;

private:
    ArenaUnorderedMap<const Inst *, FixedUses> table_;
    ArenaAllocator *allocator_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_USE_TABLE_H