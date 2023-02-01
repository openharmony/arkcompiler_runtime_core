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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_TRY_CATCH_RESOLVING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_TRY_CATCH_RESOLVING_H

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

namespace panda::compiler {
class TryCatchResolving : public Optimization {
public:
    explicit TryCatchResolving(Graph *graph);
    NO_MOVE_SEMANTIC(TryCatchResolving);
    NO_COPY_SEMANTIC(TryCatchResolving);
    ~TryCatchResolving() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "TryCatchResolving";
    }
    void InvalidateAnalyses() override;

private:
    void VisitTry(TryInst *try_inst);
    BasicBlock *TryFindResolvedCatchHandler(BasicBlock *try_begin, BasicBlock *try_end);
    void DeleteTryCatchEdges(BasicBlock *try_begin, BasicBlock *try_end);
    void ConnectCatchHandlerAfterThrow(BasicBlock *try_end, BasicBlock *catch_block);
    void RemoveCatchPhis(BasicBlock *block, Inst *throw_inst);
    std::optional<uint32_t> TryGetObjectId(const Inst *inst);
    bool DFS(BasicBlock *block, Marker marker, uint32_t try_id);

private:
    Marker marker_ {UNDEF_MARKER};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_TRY_CATCH_RESOLVING_H
