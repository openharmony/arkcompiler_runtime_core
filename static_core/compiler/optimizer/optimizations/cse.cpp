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

#include "cse.h"
#include "utils/logger.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/dominators_tree.h"
#include "algorithm"
#include "compiler_logger.h"

namespace panda::compiler {
void Cse::LocalCse()
{
    deleted_insts_.clear();
    candidates_.clear();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        candidates_.clear();
        for (auto inst : bb->Insts()) {
            if (!IsLegalExp(inst)) {
                continue;
            }
            // match the insts in candidates
            Exp exp = GetExp(inst);
            if (AllNotIn(candidates_, inst)) {
                InstVector empty_tmp(GetGraph()->GetLocalAllocator()->Adapter());
                empty_tmp.emplace_back(inst);
                candidates_.emplace(exp, std::move(empty_tmp));
                continue;
            }
            if (NotIn(candidates_, exp)) {
                exp = GetExpCommutative(inst);
            }

            auto &cntainer = candidates_.at(exp);
            auto iter = std::find_if(cntainer.begin(), cntainer.end(), Finder(inst));
            if (iter != cntainer.end()) {
                inst->ReplaceUsers(*iter);
                deleted_insts_.emplace_back(inst);
            } else {
                cntainer.emplace_back(inst);
            }
        }
    }
    RemoveInstsIn(&deleted_insts_);
}

void Cse::CollectTreeForest()
{
    replace_pair_.clear();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->IsEndBlock()) {
            continue;
        }
        candidates_.clear();
        for (auto inst : bb->Insts()) {
            if (!IsLegalExp(inst)) {
                continue;
            }
            auto exp = GetExp(inst);
            if (AllNotIn(candidates_, inst)) {
                InstVector vectmp(GetGraph()->GetLocalAllocator()->Adapter());
                vectmp.emplace_back(inst);
                candidates_.emplace(exp, std::move(vectmp));
            } else if (!NotIn(candidates_, exp)) {
                candidates_.at(exp).emplace_back(inst);
            } else {
                candidates_.at(GetExpCommutative(inst)).emplace_back(inst);
            }
        }
        if (candidates_.empty()) {
            continue;
        }
        for (auto domed : bb->GetDominatedBlocks()) {
            for (auto inst : domed->Insts()) {
                if (!IsLegalExp(inst) || AllNotIn(candidates_, inst)) {
                    continue;
                }
                Exp exp = NotIn(candidates_, GetExp(inst)) ? GetExpCommutative(inst) : GetExp(inst);
                auto &cntainer = candidates_.at(exp);
                auto iter = std::find_if(cntainer.begin(), cntainer.end(), Finder(inst));
                if (iter != cntainer.end()) {
                    replace_pair_.emplace_back(inst, *iter);
                }
            }
        }
    }
}

/// Convert the tree-forest structure of replace_pair into star-forest structure
void Cse::ConvertTreeForestToStarForest()
{
    min_replace_star_.clear();
    for (auto rpair : replace_pair_) {
        Inst *temp = rpair.second;
        auto it = replace_pair_.begin();
        do {
            it = replace_pair_.begin();
            while (it != replace_pair_.end() && it->first != temp) {
                it++;
            }
            if (it != replace_pair_.end()) {
                temp = it->second;
            }
        } while (it != replace_pair_.end());

        min_replace_star_.emplace_back(temp, rpair.first);
    }
}

void Cse::EliminateAlongDomTree()
{
    // Eliminate along Dominator tree (based on star structure)
    deleted_insts_.clear();
    for (auto pair : min_replace_star_) {
        pair.second->ReplaceUsers(pair.first);
        deleted_insts_.emplace_back(pair.second);
    }
    RemoveInstsIn(&deleted_insts_);
}

void Cse::BuildSetOfPairs(BasicBlock *block)
{
    same_exp_pair_.clear();
    auto bbl = block->GetPredsBlocks()[0];
    auto bbr = block->GetPredsBlocks()[1];
    for (auto instl : bbl->Insts()) {
        if (!IsLegalExp(instl) || InVector(deleted_insts_, instl)) {
            continue;
        }
        Exp expl = GetExp(instl);
        if (!NotIn(same_exp_pair_, expl)) {
            auto &pair = same_exp_pair_.at(expl);
            pair.first.emplace_back(instl);
            continue;
        }
        if (!AllNotIn(same_exp_pair_, instl)) {
            auto &pair = same_exp_pair_.at(GetExpCommutative(instl));
            pair.first.emplace_back(instl);
            continue;
        }
        for (auto instr : bbr->Insts()) {
            if (!IsLegalExp(instr) || (NotSameExp(GetExp(instr), expl) && NotSameExp(GetExpCommutative(instr), expl)) ||
                InVector(deleted_insts_, instr)) {
                continue;
            }
            if (!NotIn(same_exp_pair_, expl)) {
                auto &pair = same_exp_pair_.at(expl);
                pair.second.emplace_back(instr);
                continue;
            }
            InstVector emptyl(GetGraph()->GetLocalAllocator()->Adapter());
            emptyl.emplace_back(instl);
            InstVector emptyr(GetGraph()->GetLocalAllocator()->Adapter());
            emptyr.emplace_back(instr);
            same_exp_pair_.emplace(expl, std::make_pair(std::move(emptyl), std::move(emptyr)));
        }
    }
}

void Cse::DomTreeCse()
{
    CollectTreeForest();
    ConvertTreeForestToStarForest();
    EliminateAlongDomTree();
}

void Cse::GlobalCse()
{
    // Cse should not be used in OSR mode.
    if (GetGraph()->IsOsrMode()) {
        return;
    }

    deleted_insts_.clear();
    matched_tuple_.clear();
    static constexpr size_t TWO_PREDS = 2;
    // Find out redundant insts
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->GetPredsBlocks().size() != TWO_PREDS) {
            continue;
        }
        BuildSetOfPairs(bb);
        if (same_exp_pair_.empty()) {
            continue;
        }
        // Build the set of matched insts
        for (auto inst : bb->Insts()) {
            if (!IsLegalExp(inst) || AllNotIn(same_exp_pair_, inst)) {
                continue;
            }
            Exp exp = NotIn(same_exp_pair_, GetExp(inst)) ? GetExpCommutative(inst) : GetExp(inst);
            auto &pair = same_exp_pair_.find(exp)->second;
            for (auto instl : pair.first) {
                // If one decides to enable Cse in OSR mode then
                // ensure that inst's basic block is not OsrEntry.
                deleted_insts_.emplace_back(inst);
                auto lrpair = std::make_pair(instl, *(pair.second.begin()));
                matched_tuple_.emplace_back(inst, std::move(lrpair));
                break;
            }
        }
    }
    // Add phi instruction
    matched_tuple_.shrink_to_fit();
    for (auto tuple : matched_tuple_) {
        auto inst = tuple.first;
        auto phi = GetGraph()->CreateInstPhi(inst->GetType(), inst->GetPc());
        inst->ReplaceUsers(phi);
        inst->GetBasicBlock()->AppendPhi(phi);
        auto &pair = tuple.second;
        phi->AppendInput(pair.first);
        phi->AppendInput(pair.second);
    }
    RemoveInstsIn(&deleted_insts_);
}

bool Cse::RunImpl()
{
    LocalCse();
    DomTreeCse();
    GlobalCse();
    if (!is_applied_) {
        COMPILER_LOG(DEBUG, CSE_OPT) << "Cse does not find duplicate insts";
    }
    return is_applied_;
}
}  // namespace panda::compiler
