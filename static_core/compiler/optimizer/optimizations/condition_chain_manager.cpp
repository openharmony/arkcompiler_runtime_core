/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "condition_chain_manager.h"
#include "condition_chain.h"

namespace panda::compiler {
ConditionChainManager::ConditionChainManager(ArenaAllocator *allocator)
    : allocator_(allocator), condition_chain_bb_(allocator->Adapter())
{
}

ConditionChain *ConditionChainManager::FindConditionChain(BasicBlock *bb)
{
    if (bb->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
        return nullptr;
    }

    auto condition_chain = TryConditionChain(bb, bb->GetTrueSuccessor(), bb->GetFalseSuccessor());
    if (condition_chain != nullptr) {
        return condition_chain;
    }

    return TryConditionChain(bb, bb->GetFalseSuccessor(), bb->GetTrueSuccessor());
}

ConditionChain *ConditionChainManager::TryConditionChain(BasicBlock *bb, BasicBlock *multiple_preds_succ,
                                                         BasicBlock *chain_bb)
{
    auto loop = bb->GetLoop();
    if (multiple_preds_succ->GetLoop() != loop) {
        return nullptr;
    }
    if (chain_bb->GetLoop() != loop) {
        return nullptr;
    }
    auto chain_pos = condition_chain_bb_.size();
    condition_chain_bb_.push_back(bb);

    size_t single_pred_succ_index = 0;
    while (true) {
        if (IsConditionChainCandidate(chain_bb)) {
            if (chain_bb->GetTrueSuccessor() == multiple_preds_succ) {
                condition_chain_bb_.push_back(chain_bb);
                chain_bb = chain_bb->GetFalseSuccessor();
                single_pred_succ_index = BasicBlock::FALSE_SUCC_IDX;
                continue;
            }
            if (chain_bb->GetFalseSuccessor() == multiple_preds_succ) {
                condition_chain_bb_.push_back(chain_bb);
                chain_bb = chain_bb->GetTrueSuccessor();
                single_pred_succ_index = BasicBlock::TRUE_SUCC_IDX;
                continue;
            }
        }
        auto chain_size = condition_chain_bb_.size() - chain_pos;
        if (chain_size > 1) {
            // store successors indices instead of pointers to basic blocks because they can be changed during loop
            // transformation
            return allocator_->New<ConditionChain>(condition_chain_bb_.begin() + chain_pos, chain_size,
                                                   bb->GetSuccBlockIndex(multiple_preds_succ), single_pred_succ_index);
        }
        condition_chain_bb_.pop_back();
        break;
    }
    return nullptr;
}

bool ConditionChainManager::IsConditionChainCandidate(const BasicBlock *bb)
{
    auto loop = bb->GetLoop();
    return bb->GetSuccsBlocks().size() == MAX_SUCCS_NUM && bb->GetLastInst()->GetPrev() == nullptr &&
           bb->GetTrueSuccessor()->GetLoop() == loop && bb->GetFalseSuccessor()->GetLoop() == loop;
}

void ConditionChainManager::Reset()
{
    condition_chain_bb_.clear();
}
}  // namespace panda::compiler
