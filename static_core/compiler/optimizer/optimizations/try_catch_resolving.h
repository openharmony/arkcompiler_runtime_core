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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_TRY_CATCH_RESOLVING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_TRY_CATCH_RESOLVING_H

#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"

namespace panda::compiler {
class TryCatchResolving : public Optimization {
public:
    explicit TryCatchResolving(Graph *graph);
    NO_MOVE_SEMANTIC(TryCatchResolving);
    NO_COPY_SEMANTIC(TryCatchResolving);
    ~TryCatchResolving() override
    {
        try_blocks_.clear();
        throw_insts_.clear();
        catch_blocks_.clear();
        phi_insts_.clear();
        cphi2phi_.clear();
        catch2cphis_.clear();
    }

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "TryCatchResolving";
    }
    void InvalidateAnalyses() override;

private:
    void CollectCandidates();
    BasicBlock *FindCatchBeginBlock(BasicBlock *bb);
    void VisitTryInst(TryInst *try_inst);
    void ConnectThrowCatch();
    void ConnectThrowCatchImpl(BasicBlock *catch_block, BasicBlock *throw_block, uint32_t catch_pc, Inst *new_obj,
                               Inst *thr0w);
    void DeleteTryCatchEdges(BasicBlock *try_begin, BasicBlock *try_end);
    void RemoveCatchPhis(BasicBlock *cphis_block, BasicBlock *catch_block, Inst *throw_inst, Inst *phi_inst);
    void RemoveCatchPhisImpl(CatchPhiInst *catch_phi, BasicBlock *catch_block, Inst *throw_inst);

private:
    Marker marker_ {UNDEF_MARKER};
    ArenaVector<BasicBlock *> try_blocks_;
    ArenaVector<Inst *> throw_insts_;
    ArenaMap<uint32_t, BasicBlock *> catch_blocks_;
    ArenaMap<uint32_t, PhiInst *> phi_insts_;
    ArenaMap<CatchPhiInst *, PhiInst *> cphi2phi_;
    ArenaMap<BasicBlock *, BasicBlock *> catch2cphis_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_TRY_CATCH_RESOLVING_H
