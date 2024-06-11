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

#include "compiler_logger.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/countable_loop_parser.h"
#include "optimizer/optimizations/loop_idioms.h"

namespace ark::compiler {
bool LoopIdioms::RunImpl()
{
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        // There is only one supported idiom and intrinsic
        // emitted for it could not be encoded on Arm32.
        return false;
    }
    GetGraph()->RunPass<LoopAnalyzer>();
    RunLoopsVisitor();
    return isApplied_;
}

void LoopIdioms::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

bool LoopIdioms::TransformLoop(Loop *loop)
{
    if (TryTransformArrayInitIdiom(loop)) {
        isApplied_ = true;
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

bool IsLoopContainsArrayInitIdiom(StoreInst *store, Loop *loop, CountableLoopInfo &loopInfo)
{
    auto storeIdx = store->GetIndex();

    return loopInfo.constStep == 1UL && loopInfo.index == storeIdx && loopInfo.normalizedCc == ConditionCode::CC_LT &&
           AllUsesWithinLoop(storeIdx, loop) && AllUsesWithinLoop(loopInfo.update, loop) &&
           AllUsesWithinLoop(loopInfo.ifImm->GetInput(0).GetInst(), loop);
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

    auto loopInfoOpt = CountableLoopParser {*loop}.Parse();
    if (!loopInfoOpt.has_value()) {
        return false;
    }
    auto loopInfo = *loopInfoOpt;
    if (!IsLoopContainsArrayInitIdiom(store, loop, loopInfo)) {
        return false;
    }
    ASSERT(loopInfo.isInc);

    MarkerHolder holder {GetGraph()};
    Marker marker = holder.GetMarker();
    store->SetMarker(marker);
    loopInfo.update->SetMarker(marker);
    loopInfo.index->SetMarker(marker);
    loopInfo.ifImm->SetMarker(marker);
    loopInfo.ifImm->GetInput(0).GetInst()->SetMarker(marker);

    if (!CanReplaceLoop(loop, marker)) {
        return false;
    }

    COMPILER_LOG(DEBUG, LOOP_TRANSFORM) << "Array init idiom found in loop: " << loop->GetId()
                                        << "\n\tarray: " << *store->GetArray()
                                        << "\n\tvalue: " << *store->GetStoredValue()
                                        << "\n\tinitial index: " << *loopInfo.init << "\n\ttest: " << *loopInfo.test
                                        << "\n\tupdate: " << *loopInfo.update << "\n\tstep: " << loopInfo.constStep
                                        << "\n\tindex: " << *loopInfo.index;

    bool alwaysJump = false;
    if (loopInfo.init->IsConst() && loopInfo.test->IsConst()) {
        auto iterations =
            loopInfo.test->CastToConstant()->GetIntValue() - loopInfo.init->CastToConstant()->GetIntValue();
        if (iterations <= ITERATIONS_THRESHOLD) {
            COMPILER_LOG(DEBUG, LOOP_TRANSFORM)
                << "Loop will have " << iterations << " iterations, so intrinsics will not be generated";
            return false;
        }
        alwaysJump = true;
    }

    return ReplaceArrayInitLoop(loop, &loopInfo, store, alwaysJump);
}

Inst *LoopIdioms::CreateArrayInitIntrinsic(StoreInst *store, CountableLoopInfo *info)
{
    auto type = store->GetType();
    RuntimeInterface::IntrinsicId intrinsicId;
    switch (type) {
        case DataType::BOOL:
        case DataType::INT8:
        case DataType::UINT8:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_8;
            break;
        case DataType::INT16:
        case DataType::UINT16:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_16;
            break;
        case DataType::INT32:
        case DataType::UINT32:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_32;
            break;
        case DataType::INT64:
        case DataType::UINT64:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_64;
            break;
        case DataType::FLOAT32:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_F32;
            break;
        case DataType::FLOAT64:
            intrinsicId = RuntimeInterface::IntrinsicId::LIB_CALL_MEMSET_F64;
            break;
        default:
            return nullptr;
    }

    auto fillArray = GetGraph()->CreateInstIntrinsic(DataType::VOID, store->GetPc(), intrinsicId);
    fillArray->ClearFlag(inst_flags::Flags::REQUIRE_STATE);
    fillArray->ClearFlag(inst_flags::Flags::RUNTIME_CALL);
    fillArray->ClearFlag(inst_flags::Flags::CAN_THROW);
    fillArray->SetInputs(GetGraph()->GetAllocator(), {{store->GetArray(), DataType::REFERENCE},
                                                      {store->GetStoredValue(), type},
                                                      {info->init, DataType::INT32},
                                                      {info->test, DataType::INT32}});
    return fillArray;
}

bool LoopIdioms::ReplaceArrayInitLoop(Loop *loop, CountableLoopInfo *loopInfo, StoreInst *store, bool alwaysJump)
{
    auto inst = CreateArrayInitIntrinsic(store, loopInfo);
    if (inst == nullptr) {
        return false;
    }
    auto header = loop->GetHeader();
    auto preHeader = loop->GetPreHeader();

    auto loopSucc = header->GetSuccessor(0) == header ? header->GetSuccessor(1) : header->GetSuccessor(0);
    if (alwaysJump) {
        ASSERT(loop->GetBlocks().size() == 1);
        // insert block before disconnecting header to properly handle Phi in loop_succ
        auto block = header->InsertNewBlockToSuccEdge(loopSucc);
        preHeader->ReplaceSucc(header, block, true);
        GetGraph()->DisconnectBlock(header, false, false);
        block->AppendInst(inst);

        COMPILER_LOG(INFO, LOOP_TRANSFORM) << "Replaced loop " << loop->GetId() << " with the instruction " << *inst
                                           << " inserted into the new block " << block->GetId();
    } else {
        auto guardBlock = preHeader->InsertNewBlockToSuccEdge(header);
        auto sub = GetGraph()->CreateInstSub(DataType::INT32, inst->GetPc(), loopInfo->test, loopInfo->init);
        auto cmp = GetGraph()->CreateInstCompare(DataType::BOOL, inst->GetPc(), sub,
                                                 GetGraph()->FindOrCreateConstant(ITERATIONS_THRESHOLD),
                                                 DataType::INT32, ConditionCode::CC_LE);
        auto ifImm =
            GetGraph()->CreateInstIfImm(DataType::NO_TYPE, inst->GetPc(), cmp, 0, DataType::BOOL, ConditionCode::CC_NE);
        guardBlock->AppendInst(sub);
        guardBlock->AppendInst(cmp);
        guardBlock->AppendInst(ifImm);

        auto mergeBlock = header->InsertNewBlockToSuccEdge(loopSucc);
        auto intrinsicBlock = GetGraph()->CreateEmptyBlock();

        guardBlock->AddSucc(intrinsicBlock);
        intrinsicBlock->AddSucc(mergeBlock);
        intrinsicBlock->AppendInst(inst);

        COMPILER_LOG(INFO, LOOP_TRANSFORM) << "Inserted conditional jump into intinsic " << *inst << " before  loop "
                                           << loop->GetId() << ", inserted blocks: " << intrinsicBlock->GetId() << ", "
                                           << guardBlock->GetId() << ", " << mergeBlock->GetId();
    }
    return true;
}

}  // namespace ark::compiler
