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
#ifndef PHI_RESOLVER_H
#define PHI_RESOLVER_H

#include "inst_builder.h"
#include "compiler_logger.h"

namespace panda::compiler {
/**
 * Resolve phi instructions in the given graph. Resolving phi is:
 *  - remove phi if it has only SafePoint in users
 *  - if phi has no type, then set type determined from its inputs
 */
class PhiResolver {
public:
    explicit PhiResolver(Graph *graph)
        : graph_(graph),
          real_inputs_(graph->GetLocalAllocator()->Adapter()),
          phi_users_(graph->GetLocalAllocator()->Adapter()),
          real_users_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(PhiResolver);
    NO_COPY_SEMANTIC(PhiResolver);
    ~PhiResolver() = default;

    void Run()
    {
        MarkerHolder marker_holder {graph_};
        has_real_user_marker_ = marker_holder.GetMarker();
        for (auto bb : graph_->GetBlocksRPO()) {
            for (auto inst : bb->AllInstsSafe()) {
                if (!inst->IsPhi() && !inst->IsCatchPhi()) {
                    continue;
                }
                MarkPhiWithRealUsers(inst);
            }
        }

        for (auto bb : graph_->GetBlocksRPO()) {
            for (auto inst : bb->AllInstsSafe()) {
                if (!inst->IsPhi() && !inst->IsCatchPhi()) {
                    TryRemoveFromSaveStates(inst);
                    continue;
                }
                CheckPhiInputs(inst);
                // Do not try to remove typed phi with users, otherwise e.g. could be deleted `phi(obj, phi)` from the
                // loop-header and `obj` will be missed in the SaveStates inside the loop.
                // Or environment could be  deleted from SaveState.
                if ((inst->IsPhi() || (inst->GetType() == DataType::ANY && inst->IsCatchPhi())) && inst->HasType() &&
                    !inst->GetUsers().Empty()) {
                    TryRemoveFromSaveStates(inst);
                    continue;
                }

                if (!inst->IsMarked(has_real_user_marker_)) {
                    // inst has only SaveState users
                    RemovePhiInst(inst);
                } else if (!inst->HasType()) {
                    SetTypeByInputs(inst);
                    ASSERT(inst->HasType() || (inst->IsCatchPhi() && !inst->CastToCatchPhi()->IsAcc()));
                }
            }
        }

        RemoveDeadPhi();
    }

private:
    void CleanUp()
    {
        phi_users_.clear();
        real_users_.clear();
        has_save_state_inst_only_ = true;
        marker_ = graph_->NewMarker();
    }
    static void SetTypeByInputs(Inst *inst)
    {
        if (inst->IsCatchPhi() && inst->CastToCatchPhi()->IsAcc()) {
            inst->SetType(DataType::REFERENCE);
            return;
        }
        for (auto input : inst->GetInputs()) {
            auto input_type = input.GetInst()->GetType();
            if (input_type != DataType::NO_TYPE) {
                inst->SetType(input_type);
                break;
            }
        }
    }
    void FindUsersRec(Inst *inst)
    {
        for (auto &user : inst->GetUsers()) {
            if (user.GetInst()->SetMarker(marker_)) {
                continue;
            }
            if (user.GetInst()->IsPhi() || user.GetInst()->IsCatchPhi()) {
                phi_users_.push_back(user.GetInst());
                FindUsersRec(user.GetInst());
            } else {
                if (!user.GetInst()->IsSaveState()) {
                    has_save_state_inst_only_ = false;
                    break;
                }
                real_users_.push_back(user.GetInst());
            }
        }
    }

    void FindInputsRec(Inst *inst)
    {
        ASSERT(inst->IsPhi() || inst->IsCatchPhi());
        // We can't set real type if there aren't inputs from Phi/CathPhi
        // We add the Phi/CathPhi in the list and return false from CheckPhiInputs
        if (inst->GetInputs().Empty()) {
            real_inputs_.push_back(inst);
            return;
        }
        for (auto &input : inst->GetInputs()) {
            auto input_inst = input.GetInst();
            if (input_inst->SetMarker(marker_)) {
                continue;
            }
            if (input_inst->IsPhi() || input_inst->IsCatchPhi()) {
                if (input_inst->GetType() != DataType::NO_TYPE) {
                    real_inputs_.push_back(input_inst);
                    continue;
                }
                FindInputsRec(input_inst);
            } else {
                real_inputs_.push_back(input_inst);
            }
        }
    }

    bool CheckPhiRealInputs(Inst *phi_inst)
    {
        DataType::Type type = DataType::NO_TYPE;
        real_inputs_.clear();
        marker_ = graph_->NewMarker();
        phi_inst->SetMarker(marker_);
        FindInputsRec(phi_inst);
        graph_->EraseMarker(marker_);

        bool has_constant_input = false;
        for (auto input_inst : real_inputs_) {
            auto input_type = input_inst->GetType();
            if (input_type == DataType::NO_TYPE) {
                return false;
            }
            if (input_inst->IsConst() && input_type == DataType::INT64) {
                if (type != DataType::NO_TYPE && DataType::GetCommonType(type) != DataType::INT64) {
                    return false;
                }
                has_constant_input = true;
                continue;
            }
            if (type == DataType::NO_TYPE) {
                if (has_constant_input && DataType::GetCommonType(input_type) != DataType::INT64) {
                    return false;
                }
                type = input_type;
            } else if (type != input_type) {
                return false;
            }
        }

        if (type == DataType::NO_TYPE) {
            // Do not remove phi with constants-only inputs.
            if (!has_constant_input) {
                return false;
            }
            type = DataType::INT64;
        }
        phi_inst->SetType(type);
        return true;
    }

    // Returns false if block with input instruction doesn't dominate the predecessor of the  PHI block
    bool CheckPhiInputs(Inst *phi_inst)
    {
        ASSERT(phi_inst->IsPhi() || phi_inst->IsCatchPhi());
        if (phi_inst->HasType()) {
            return true;
        }
        if (phi_inst->IsPhi()) {
            if (phi_inst->GetInputsCount() != phi_inst->GetBasicBlock()->GetPredsBlocks().size()) {
                return false;
            }
            for (size_t index = 0; index < phi_inst->GetInputsCount(); ++index) {
                auto pred = phi_inst->GetBasicBlock()->GetPredBlockByIndex(index);
                auto input_bb = phi_inst->GetInput(index).GetInst()->GetBasicBlock();
                if (!input_bb->IsDominate(pred)) {
                    return false;
                }
            }
        }
        return CheckPhiRealInputs(phi_inst);
    }

    void MarkHasRealUserRec(Inst *inst)
    {
        if (inst->SetMarker(has_real_user_marker_)) {
            return;
        }
        for (auto &input : inst->GetInputs()) {
            auto input_inst = input.GetInst();
            if (input_inst->IsPhi() || input_inst->IsCatchPhi()) {
                MarkHasRealUserRec(input_inst);
            }
        }
    }

    void TryRemoveFromSaveStates(Inst *inst)
    {
        if (OPTIONS.IsCompilerNonOptimizing()) {
            return;
        }
        MarkerHolder marker_holder {graph_};
        marker_ = marker_holder.GetMarker();
        for (auto &user : inst->GetUsers()) {
            auto user_inst = user.GetInst();
            if (user_inst->IsPhi() || user_inst->IsCatchPhi()) {
                if (user_inst->IsMarked(has_real_user_marker_)) {
                    return;
                }
                continue;
            }
            if (user_inst->IsSaveState()) {
                continue;
            }
            MarkInstsOnPaths(user_inst->GetBasicBlock(), inst, user_inst);
        }
        for (auto user_it = inst->GetUsers().begin(); user_it != inst->GetUsers().end();) {
            auto user_inst = user_it->GetInst();
            if (!user_inst->IsSaveState() || user_inst->GetBasicBlock()->IsMarked(marker_) ||
                user_inst->IsMarked(marker_) || user_it->GetVirtualRegister().IsEnv()) {
                ++user_it;
            } else {
                user_inst->RemoveInput(user_it->GetIndex());
                user_it = inst->GetUsers().begin();
            }
        }
    }

    void MarkInstsOnPaths(BasicBlock *block, Inst *object, Inst *start_from)
    {
        if (block->IsMarked(marker_)) {
            return;
        }
        if (start_from != nullptr) {
            auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, start_from);
            for (; it != block->AllInstsSafeReverse().end(); ++it) {
                auto inst = *it;
                if (inst->SetMarker(marker_) || inst == object) {
                    return;
                }
            }
        } else {
            block->SetMarker(marker_);
        }
        for (auto pred : block->GetPredsBlocks()) {
            MarkInstsOnPaths(pred, object, nullptr);
        }
    }

    // Remove virtual registers of SafePoint instructions which input phis to be removed.
    void RemovePhiInst(Inst *inst)
    {
        ASSERT(inst->IsPhi() || inst->IsCatchPhi());
        CleanUp();
        inst->SetMarker(marker_);
        FindUsersRec(inst);
        for (auto user : real_users_) {
            ASSERT(user->IsSaveState());
            auto save_state = static_cast<SaveStateInst *>(user);
            size_t idx = 0;
            size_t inputs_count = save_state->GetInputsCount();
            while (idx < inputs_count) {
                auto input_inst = save_state->GetInput(idx).GetInst();
                if (input_inst->IsMarked(marker_)) {
                    // env values should not be marked for removal
                    ASSERT(!save_state->GetVirtualRegister(idx).IsEnv());
                    save_state->RemoveInput(idx);
                    inputs_count--;
                } else {
                    idx++;
                }
            }
        }
        // Phi has only SafePoint in users, we can remove this phi and all phi in collected list.
        inst->RemoveUsers<true>();
        inst->GetBasicBlock()->RemoveInst(inst);
        for (auto phi : phi_users_) {
            phi->RemoveUsers<true>();
            phi->GetBasicBlock()->RemoveInst(phi);
        }
        graph_->EraseMarker(marker_);
    }

    void MarkPhiWithRealUsers(Inst *inst)
    {
        for (auto &user : inst->GetUsers()) {
            auto user_inst = user.GetInst();
            auto can_remove_user = user_inst->IsPhi() || user_inst->IsCatchPhi() ||
                                   (user_inst->IsSaveState() && !user.GetVirtualRegister().IsEnv());
            if (!can_remove_user) {
                // inst certainly cannot be removed from user's inputs
                MarkHasRealUserRec(inst);
                break;
            }
        }
    }

    void ReplaceDeadPhiUsers(Inst *inst, BasicBlock *bb)
    {
        if (inst->GetInputsCount() != 0) {
            auto input_inst = inst->GetInput(0).GetInst();
#ifndef NDEBUG
            if (!inst->GetUsers().Empty()) {
                for ([[maybe_unused]] auto &input : inst->GetInputs()) {
                    ASSERT(input.GetInst() == input_inst);
                }
            }
#endif
            inst->ReplaceUsers(input_inst);
        }
        bb->RemoveInst(inst);
    }

    void RemoveDeadPhi()
    {
        // If any input of phi instruction is not defined then we assume that phi is dead.
        // We move users to the input and remove the PHI
        for (auto bb : graph_->GetBlocksRPO()) {
            for (auto inst : bb->PhiInstsSafe()) {
                // Skip catch phi
                if (!inst->IsPhi()) {
                    continue;
                }
                if (inst->GetInputsCount() != bb->GetPredsBlocks().size()) {
                    // if the number of PHI inputs less than the number of block predecessor and all inputs are equal
                    // Replace users to the input
                    ReplaceDeadPhiUsers(inst, bb);
                }
            }
        }
    }

private:
    Graph *graph_ {nullptr};
    InstVector real_inputs_;
    InstVector phi_users_;
    InstVector real_users_;
    Marker marker_ {UNDEF_MARKER};
    Marker has_real_user_marker_ {UNDEF_MARKER};
    bool has_save_state_inst_only_ {true};
};
}  // namespace panda::compiler

#endif  // PHI_RESOLVER_H
