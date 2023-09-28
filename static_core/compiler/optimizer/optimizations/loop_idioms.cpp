/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "compiler_logger.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/optimizations/loop_idioms.h"

namespace panda::compiler {
bool LoopIdioms::RunImpl()
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        // There is only one supported idiom and intrinsic
        // emitted for it could not be encoded on Arm32.
        return false;
    }
    GetGraph()->RunPass<LoopAnalyzer>();
    RunLoopsVisitor();
    return is_applied_;
}

void LoopIdioms::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

bool LoopIdioms::TransformLoop(Loop *loop)
{
    if (TryTransformArrayInitIdiom(loop)) {
        is_applied_ = true;
        return true;
    }
    return false;
}

StoreInst *FindStoreForArrayInit(BasicBlock *block)
{
    StoreInst *store {nullptr};
    for (auto inst : block->Insts()) {
        if (inst->GetOpcode() != Opcode::StoreArray) {
            continue;
        }
        if (store != nullptr) {
            return nullptr;
        }
        store = inst->CastToStoreArray();
    }
    // should be a loop invariant
    if (store != nullptr && store->GetStoredValue()->GetBasicBlock()->GetLoop() == block->GetLoop()) {
        return nullptr;
    }
    return store;
}

Inst *ExtractArrayInitInitialIndexValue(PhiInst *index)
{
    auto block = index->GetBasicBlock();
    BasicBlock *pred = block->GetPredsBlocks().front();
    if (pred == block) {
        pred = block->GetPredsBlocks().back();
    }
    return index->GetPhiInput(pred);
}

bool AllUsesWithinLoop(Inst *inst, Loop *loop)
{
    for (auto &user : inst->GetUsers()) {
        if (user.GetInst()->GetBasicBlock()->GetLoop() != loop) {
            return false;
        }
    }
    return true;
}

bool CanReplaceLoop(Loop *loop, Marker marker)
{
    ASSERT(loop->GetBlocks().size() == 1);
    auto block = loop->GetHeader();
    for (auto inst : block->AllInsts()) {
        if (inst->IsMarked(marker)) {
            continue;
        }
        auto opcode = inst->GetOpcode();
        if (opcode != Opcode::NOP && opcode != Opcode::SaveState && opcode != Opcode::SafePoint) {
            return false;
        }
    }
    return true;
}

bool IsLoopContainsArrayInitIdiom(StoreInst *store, Loop *loop, CountableLoopInfo &loop_info)
{
    auto store_idx = store->GetIndex();

    return loop_info.const_step == 1UL && loop_info.index == store_idx &&
           loop_info.normalized_cc == ConditionCode::CC_LT && AllUsesWithinLoop(store_idx, loop) &&
           AllUsesWithinLoop(loop_info.update, loop) &&
           AllUsesWithinLoop(loop_info.if_imm->GetInput(0).GetInst(), loop);
}

bool LoopIdioms::TryTransformArrayInitIdiom(Loop *loop)
{
    ASSERT(loop->GetInnerLoops().empty());
    if (loop->GetBlocks().size() != 1) {
        return false;
    }

    auto store = FindStoreForArrayInit(loop->GetHeader());
    if (store == nullptr) {
        return false;
    }

    auto loop_info_opt = CountableLoopParser {*loop}.Parse();
    if (!loop_info_opt.has_value()) {
        return false;
    }
    auto loop_info = *loop_info_opt;
    if (!IsLoopContainsArrayInitIdiom(store, loop, loop_info)) {
        return false;
    }
    ASSERT(loop_info.is_inc);

    MarkerHolder holder {GetGraph()};
    Marker marker = holder.GetMarker();
    store->SetMarker(marker);
    loop_info.update->SetMarker(marker);
    loop_info.index->SetMarker(marker);
    loop_info.if_imm->SetMarker(marker);
    loop_info.if_imm->GetInput(0).GetInst()->SetMarker(marker);

    if (!CanReplaceLoop(loop, marker)) {
        return false;
    }

    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Array init idiom found in loop: " << loop->GetId()
                                        << "\n\tarray: " << *store->GetArray()
                                        << "\n\tvalue: " << *store->GetStoredValue()
                                        << "\n\tinitial index: " << *loop_info.init << "\n\ttest: " << *loop_info.test
                                        << "\n\tupdate: " << *loop_info.update << "\n\tstep: " << loop_info.const_step
                                        << "\n\tindex: " << *loop_info.index;

    bool always_jump = false;
    if (loop_info.init->IsConst() && loop_info.test->IsConst()) {
        auto iterations =
            loop_info.test->CastToConstant()->GetIntValue() - loop_info.init->CastToConstant()->GetIntValue();
        if (iterations <= ITERATIONS_THRESHOLD) {
            COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
                << "Loop will have " << iterations << " iterations, so intrinsics will not be generated";
            return false;
        }
        always_jump = true;
    }

    return ReplaceArrayInitLoop(loop, &loop_info, store, always_jump);
}

Inst *LoopIdioms::CreateArrayInitIntrinsic(StoreInst *store, CountableLoopInfo *info)
{
    auto type = store->GetType();
    RuntimeInterface::IntrinsicId intrinsic_id;
    switch (type) {
        case DataType::BOOL:
        case DataType::INT8:
        case DataType::UINT8:
            intrinsic_id = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_8;
            break;
        case DataType::INT16:
        case DataType::UINT16:
            intrinsic_id = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_16;
            break;
        case DataType::INT32:
        case DataType::UINT32:
            intrinsic_id = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_32;
            break;
        case DataType::INT64:
        case DataType::UINT64:
            intrinsic_id = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_64;
            break;
        case DataType::FLOAT32:
            intrinsic_id = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_F32;
            break;
        case DataType::FLOAT64:
            intrinsic_id = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_F64;
            break;
        default:
            return nullptr;
    }

    constexpr size_t ARGS = 4;
    auto fill_array = GetGraph()->CreateInstIntrinsic(DataType::VOID, store->GetPc(), intrinsic_id);
    fill_array->ClearFlag(inst_flags::Flags::REQUIRE_STATE);
    fill_array->ClearFlag(inst_flags::Flags::RUNTIME_CALL);
    fill_array->ReserveInputs(ARGS);
    fill_array->AllocateInputTypes(GetGraph()->GetAllocator(), ARGS);
    fill_array->AppendInput(store->GetArray());
    fill_array->AddInputType(DataType::REFERENCE);
    fill_array->AppendInput(store->GetStoredValue());
    fill_array->AddInputType(store->GetStoredValue()->GetType());
    fill_array->AppendInput(info->init);
    fill_array->AddInputType(DataType::INT32);
    fill_array->AppendInput(info->test);
    fill_array->AddInputType(DataType::INT32);
    return fill_array;
}

bool LoopIdioms::ReplaceArrayInitLoop(Loop *loop, CountableLoopInfo *loop_info, StoreInst *store, bool always_jump)
{
    auto inst = CreateArrayInitIntrinsic(store, loop_info);
    if (inst == nullptr) {
        return false;
    }
    auto header = loop->GetHeader();
    auto pre_header = loop->GetPreHeader();

    auto loop_succ = header->GetSuccessor(0) == header ? header->GetSuccessor(1) : header->GetSuccessor(0);
    if (always_jump) {
        ASSERT(loop->GetBlocks().size() == 1);
        // insert block before disconnecting header to properly handle Phi in loop_succ
        auto block = header->InsertNewBlockToSuccEdge(loop_succ);
        pre_header->ReplaceSucc(header, block, true);
        GetGraph()->DisconnectBlock(header, false, false);
        block->AppendInst(inst);

        COMPILER_LOG(INFO, LOOP_TRANSFORM) << "Replaced loop " << loop->GetId() << " with the instruction " << *inst
                                           << " inserted into the new block " << block->GetId();
    } else {
        auto guard_block = pre_header->InsertNewBlockToSuccEdge(header);
        auto sub = GetGraph()->CreateInstSub(DataType::INT32, inst->GetPc());
        sub->SetInput(0, loop_info->test);
        sub->SetInput(1, loop_info->init);
        auto cmp = GetGraph()->CreateInstCompare(DataType::BOOL, inst->GetPc(), ConditionCode::CC_LE);
        cmp->SetInput(0, sub);
        cmp->SetInput(1, GetGraph()->FindOrCreateConstant(ITERATIONS_THRESHOLD));
        cmp->SetOperandsType(DataType::INT32);
        auto if_imm = GetGraph()->CreateInstIfImm(DataType::NO_TYPE, inst->GetPc(), ConditionCode::CC_NE, 0);
        if_imm->SetInput(0, cmp);
        if_imm->SetOperandsType(DataType::BOOL);
        guard_block->AppendInst(sub);
        guard_block->AppendInst(cmp);
        guard_block->AppendInst(if_imm);

        auto merge_block = header->InsertNewBlockToSuccEdge(loop_succ);
        auto intrinsic_block = GetGraph()->CreateEmptyBlock();

        guard_block->AddSucc(intrinsic_block);
        intrinsic_block->AddSucc(merge_block);
        intrinsic_block->AppendInst(inst);

        COMPILER_LOG(INFO, LOOP_TRANSFORM) << "Inserted conditional jump into intinsic " << *inst << " before  loop "
                                           << loop->GetId() << ", inserted blocks: " << intrinsic_block->GetId() << ", "
                                           << guard_block->GetId() << ", " << merge_block->GetId();
    }
    return true;
}

}  // namespace panda::compiler