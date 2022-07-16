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

#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/encode.h"
#include "optimizer/ir/graph.h"
#include "optimizer/code_generator/spill_fill_encoder.h"

namespace panda::compiler {

bool SpillFillEncoder::AreConsecutiveOps(const SpillFillData &pred, const SpillFillData &succ)
{
    bool same_src_type = pred.SrcType() == succ.SrcType();
    bool same_dst_type = pred.DstType() == succ.DstType();
    bool same_argument_type = pred.GetCommonType() == succ.GetCommonType();
    if (!same_src_type || !same_dst_type || !same_argument_type) {
        return false;
    }

    // Slots should be neighboring, note that offset from SP is decreasing when slot number is increasing,
    // so succ's slot number should be lower than pred's slot number.
    if (pred.SrcType() == LocationType::STACK && pred.SrcValue() != succ.SrcValue() + 1U) {
        return false;
    }
    if (pred.DstType() == LocationType::STACK && pred.DstValue() != succ.DstValue() + 1U) {
        return false;
    }
    return true;
}

bool SpillFillEncoder::CanCombineSpillFills(SpillFillData pred, SpillFillData succ, const Graph *graph)
{
    if (!IsCombiningEnabled(graph)) {
        return false;
    }
    // Stack slot is 64-bit wide, so we can only combine types that could be widened up to
    // 64 bit (i.e. we can' combine two floats).
    if (!DataType::Is64Bits(pred.GetCommonType(), graph->GetArch())) {
        return false;
    }

    return AreConsecutiveOps(pred, succ);
}

void SpillFillEncoder::SortSpillFillData(ArenaVector<SpillFillData> *spill_fills)
{
    constexpr size_t MAX_VECTOR_LEN = MAX_NUM_REGS + MAX_NUM_VREGS;
    // Don't sort vectors that are too large in order to reduce compilation duration.
    if (spill_fills->size() > MAX_VECTOR_LEN) {
        COMPILER_LOG(DEBUG, CODEGEN) << "Bypass spill fills sorting because corresponding vector is too large: "
                                     << spill_fills->size();
        return;
    }
    auto it = spill_fills->begin();
    while (it != spill_fills->end()) {
        // Sort spill fills only within group of consecutive SpillFillData elements sharing the same spill-fill type.
        // SpillFillData elements could not be reordered within whole spill_fills array, because some of these elements
        // may be inserted by SpillFillResolver to break cyclic dependency.
        bool is_fill = it->SrcType() == LocationType::STACK && it->GetDst().IsAnyRegister();
        bool is_spill = it->GetSrc().IsAnyRegister() && it->DstType() == LocationType::STACK;
        if (!is_spill && !is_fill) {
            ++it;
            continue;
        }
        auto next = std::next(it);
        while (next != spill_fills->end() && it->SrcType() == next->SrcType() && it->DstType() == next->DstType()) {
            ++next;
        }

        if (is_spill) {
            std::sort(it, next, [](auto sf1, auto sf2) { return sf1.DstValue() > sf2.DstValue(); });
        } else {
            ASSERT(is_fill);
            std::sort(it, next, [](auto sf1, auto sf2) { return sf1.SrcValue() > sf2.SrcValue(); });
        }

        it = next;
    }
}

SpillFillEncoder::SpillFillEncoder(Codegen *codegen, Inst *inst)
    : inst_(inst->CastToSpillFill()),
      graph_(codegen->GetGraph()),
      codegen_(codegen),
      encoder_(codegen->GetEncoder()),
      fl_(codegen->GetFrameLayout())
{
    sp_reg_ = codegen->GetTarget().GetStackReg();
}

void SpillFillEncoder::EncodeSpillFill()
{
    if (IsCombiningEnabled(graph_)) {
        SortSpillFillData(&(inst_->GetSpillFills()));
    }

    // hint on how many consecutive ops current group contain
    int consecutive_ops_hint = 0;
    for (auto it = inst_->GetSpillFills().begin(), end = inst_->GetSpillFills().end(); it != end;) {
        auto sf = *it;
        auto next_it = std::next(it);
        SpillFillData *next = next_it == end ? nullptr : &(*next_it);

        // new group started
        if (consecutive_ops_hint <= 0) {
            consecutive_ops_hint = 1;
            // find how many consecutive SpillFillData have the same type, source and destination type
            // and perform read or write from consecutive stack slots.
            for (auto group_it = it, next_group_it = std::next(it);
                 next_group_it != end && AreConsecutiveOps(*group_it, *next_group_it); ++next_group_it) {
                consecutive_ops_hint++;
                group_it = next_group_it;
            }
        }

        size_t adv = 0;
        switch (sf.SrcType()) {
            case LocationType::IMMEDIATE: {
                adv = EncodeImmToX(sf);
                break;
            }
            case LocationType::FP_REGISTER:
            case LocationType::REGISTER: {
                adv = EncodeRegisterToX(sf, next, consecutive_ops_hint);
                break;
            }
            case LocationType::STACK_PARAMETER:
            case LocationType::STACK: {
                adv = EncodeStackToX(sf, next, consecutive_ops_hint);
                break;
            }
            default:
                UNREACHABLE();
        }
        consecutive_ops_hint -= adv;
        std::advance(it, adv);
    }
}

size_t SpillFillEncoder::EncodeImmToX(const SpillFillData &sf)
{
    auto const_inst = graph_->GetSpilledConstant(sf.SrcValue());
    ASSERT(const_inst->IsConst());

    if (sf.GetDst().IsAnyRegister()) {  // imm -> register
        auto type = sf.GetType();
        if (graph_->IsDynamicMethod() && const_inst->GetType() == DataType::INT64) {
            type = DataType::UINT32;
        }
        auto imm = codegen_->ConvertImm(const_inst, type);
        auto dst_reg = GetDstReg(sf.GetDst(), imm.GetType());
        encoder_->EncodeMov(dst_reg, imm);
        return 1U;
    }

    ASSERT(sf.GetDst().IsAnyStack());  // imm -> stack
    auto dst_mem = codegen_->GetMemRefForSlot(sf.GetDst());
    auto imm = codegen_->ConvertImm(const_inst, sf.GetCommonType());
    encoder_->EncodeSti(imm, dst_mem);
    return 1U;
}

size_t SpillFillEncoder::EncodeRegisterToX(const SpillFillData &sf, const SpillFillData *next, int consecutive_ops_hint)
{
    if (sf.GetDst().IsAnyRegister()) {  // register -> register
        auto src_reg = codegen_->ConvertRegister(sf.SrcValue(), sf.GetType());
        auto dst_reg = GetDstReg(sf.GetDst(), src_reg.GetType());
        encoder_->EncodeMov(dst_reg, src_reg);
        return 1U;
    }

    ASSERT(sf.GetDst().IsAnyStack());
    auto offset = codegen_->GetStackOffset(sf.GetDst());
    auto mem_ref = MemRef(sp_reg_, offset);

    if (sf.GetDst().IsStackArgument()) {  // register -> stack_arg
        auto src_reg = codegen_->ConvertRegister(sf.SrcValue(), sf.GetType());
        // There is possible to have sequence to intrinsics with no getter/setter in interpreter:
        // compiled_code->c2i(push to frame)->interpreter(HandleCallVirtShort)->i2c(move to stack)->intrinsic
        // To do not fix it in interpreter, it is better to store 64-bits
        if (src_reg.GetSize() < DOUBLE_WORD_SIZE && !src_reg.GetType().IsFloat()) {
            src_reg = src_reg.As(Codegen::ConvertDataType(DataType::REFERENCE, codegen_->GetArch()));
        }
        encoder_->EncodeStrz(src_reg, mem_ref);
        return 1U;
    }

    // register -> stack
    auto src_reg = codegen_->ConvertRegister(sf.SrcValue(), sf.GetCommonType());
    // If address is no qword aligned and current group consist of even number of consecutive slots
    // then we can skip current operation.
    constexpr int COALESCE_OPS_LIMIT = 2;
    auto skip_coalescing = (consecutive_ops_hint % COALESCE_OPS_LIMIT == 1) && (offset % QUAD_WORD_SIZE_BYTE != 0);
    if (next != nullptr && CanCombineSpillFills(sf, *next, graph_) && !skip_coalescing) {
        auto next_reg = codegen_->ConvertRegister(next->SrcValue(), next->GetCommonType());
        encoder_->EncodeStp(src_reg, next_reg, mem_ref);
        return 2U;
    }
    encoder_->EncodeStr(src_reg, mem_ref);
    return 1U;
}

size_t SpillFillEncoder::EncodeStackToX(const SpillFillData &sf, const SpillFillData *next, int consecutive_ops_hint)
{
    auto offset = codegen_->GetStackOffset(sf.GetSrc());
    auto src_mem = MemRef(sp_reg_, offset);
    auto type_info = Codegen::ConvertDataType(sf.GetType(), codegen_->GetArch());

    if (sf.GetDst().IsAnyRegister()) {  // stack -> register
        // If address is no qword aligned and current group consist of even number of consecutive slots
        // then we can skip current operation.
        constexpr int COALESCE_OPS_LIMIT = 2;
        auto skip_coalescing = (consecutive_ops_hint % COALESCE_OPS_LIMIT == 1) && (offset % QUAD_WORD_SIZE_BYTE != 0);
        if (next != nullptr && CanCombineSpillFills(sf, *next, graph_) && !skip_coalescing) {
            auto cur_reg = codegen_->ConvertRegister(sf.DstValue(), sf.GetCommonType());
            auto next_reg = codegen_->ConvertRegister(next->DstValue(), next->GetCommonType());
            encoder_->EncodeLdp(cur_reg, next_reg, false, src_mem);
            return 2U;
        }
        auto dst_reg = GetDstReg(sf.GetDst(), type_info);
        encoder_->EncodeLdr(dst_reg, false, src_mem);
        return 1U;
    }

    // stack -> stack
    ASSERT(sf.GetDst().IsAnyStack());
    auto dst_mem = codegen_->GetMemRefForSlot(sf.GetDst());
    encoder_->EncodeMemCopy(src_mem, dst_mem, DOUBLE_WORD_SIZE);  // Stack slot is 64-bit wide
    return 1U;
}
}  // namespace panda::compiler
