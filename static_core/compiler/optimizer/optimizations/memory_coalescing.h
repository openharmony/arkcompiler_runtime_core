/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_COALESCING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_COALESCING_H

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "compiler_options.h"

namespace panda::compiler {
class MemoryCoalescing : public Optimization {
    using Optimization::Optimization;

public:
    struct CoalescedPair {
        Inst *first;
        Inst *second;
    };

    explicit MemoryCoalescing(Graph *graph, bool aligned = true) : Optimization(graph), aligned_only_(aligned) {}

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return OPTIONS.IsCompilerMemoryCoalescing();
    }

    const char *GetPassName() const override
    {
        return "MemoryCoalescing";
    }

    /// Types of memory accesses that can be coalesced
    static bool AcceptedType(DataType::Type type)
    {
        switch (type) {
            case DataType::UINT32:
            case DataType::INT32:
            case DataType::UINT64:
            case DataType::INT64:
            case DataType::FLOAT32:
            case DataType::FLOAT64:
            case DataType::ANY:
                return true;
            case DataType::REFERENCE:
                return OPTIONS.IsCompilerMemoryCoalescingObjects();
            default:
                return false;
        }
    }

    NO_MOVE_SEMANTIC(MemoryCoalescing);
    NO_COPY_SEMANTIC(MemoryCoalescing);
    ~MemoryCoalescing() override = default;

private:
    void ReplacePairs(ArenaVector<CoalescedPair> const &pairs);
    void ReplacePair(Inst *first, Inst *second, Inst *insert_after);

    Inst *ReplaceLoadArray(Inst *first, Inst *second, Inst *insert_after);
    Inst *ReplaceLoadArrayI(Inst *first, Inst *second, Inst *insert_after);
    Inst *ReplaceStoreArray(Inst *first, Inst *second, Inst *insert_after);
    Inst *ReplaceStoreArrayI(Inst *first, Inst *second, Inst *insert_after);

private:
    bool aligned_only_;
};
}  // namespace panda::compiler

#endif  //  COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_COALESCING_H
