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
#ifndef COMPILER_OPTIMIZER_CODEGEN_SPILL_FILL_ENCODER_H_
#define COMPILER_OPTIMIZER_CODEGEN_SPILL_FILL_ENCODER_H_

#include "optimizer/ir/inst.h"
#include "optimizer/ir/graph_visitor.h"
#include "utils/cframe_layout.h"

namespace panda::compiler {

class Graph;
class Codegen;
class Encoder;

// Helper class for SpillFill encoding
class SpillFillEncoder {
public:
    SpillFillEncoder(Codegen *codegen, Inst *inst);
    ~SpillFillEncoder() = default;
    NO_COPY_SEMANTIC(SpillFillEncoder);
    NO_MOVE_SEMANTIC(SpillFillEncoder);

    void EncodeSpillFill();
    static bool CanCombineSpillFills(SpillFillData pred, SpillFillData succ, const Graph *graph);
    static void SortSpillFillData(ArenaVector<SpillFillData> *spill_fills);

    static bool IsCombiningEnabled(const Graph *graph)
    {
        return graph->GetArch() == Arch::AARCH64 && options.IsCompilerSpillFillPair();
    }

    inline Reg GetDstReg(Location dst, TypeInfo type)
    {
        if (graph_->GetArch() == Arch::AARCH32) {
            // ARM32 ABI SOFT calling convention
            if (type.IsFloat() && dst.IsRegister()) {
                type = type.GetSize() < DOUBLE_WORD_SIZE ? INT32_TYPE : INT64_TYPE;
            } else if (type.IsScalar() && dst.IsFpRegister()) {
                type = type.GetSize() < DOUBLE_WORD_SIZE ? FLOAT32_TYPE : FLOAT64_TYPE;
            }
        }
        return Reg(dst.GetValue(), type);
    }

private:
    size_t EncodeImmToX(const SpillFillData &sf);
    size_t EncodeRegisterToX(const SpillFillData &sf, const SpillFillData *next, int consecutive_ops_hint = 0);
    size_t EncodeStackToX(const SpillFillData &sf, const SpillFillData *next, int consecutive_ops_hint = 0);
    static bool AreConsecutiveOps(const SpillFillData &pred, const SpillFillData &succ);

    SpillFillInst *inst_;
    Graph *graph_;
    Codegen *codegen_;
    Encoder *encoder_;
    CFrameLayout fl_;
    Reg sp_reg_;
};
}  // namespace panda::compiler
#endif  // COMPILER_OPTIMIZER_CODEGEN_SPILL_FILL_ENCODER_H_
