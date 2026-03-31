/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_COMPILER_OPTIMIZER_IR_USE_DEF_PATH_WALKER
#define PANDA_COMPILER_OPTIMIZER_IR_USE_DEF_PATH_WALKER

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"

namespace ark::compiler {

/**
 * @brief UseDefPathWalker is an implementation of the template method pattern in a form of CRTP that walks through all
 * the instructions on the path from a @a use and its definition @a def (so, in the backward direction) and handles
 * every instruction to implement some specific business logic.
 *
 * @tparam Derived a derived class implementing the {@code bool HandleInst(Inst *inst) const} method that should be
 * const-callable. The {@code HandleInst} method must return \c true when the instruction @c inst has been successfully
 * handled (checked, processed) and @c false otherwise. If the {@code HandleInst} returns @c false, the \ref Walk method
 * immediately stops and also returns @c false what may be considered as a fault signal.
 */
template <typename Derived>
class UseDefPathWalker {
private:
    friend Derived;

    explicit UseDefPathWalker(Inst *def) : def_(def), visited_(def->GetBasicBlock()->GetGraph()) {}

    bool Walk(User &user) const
    {
        auto [startInst, startBb] = GetStartUserFrom(user);
        return Walk(user.GetInst(), startBb, startInst);
    }

    [[nodiscard]] Inst *GetDef() const
    {
        return def_;
    }

    bool Walk(const Inst *user, const BasicBlock *block, Inst *startFrom) const
    {
        if (startFrom != nullptr) {
            ASSERT(startFrom->GetBasicBlock() == block);
            auto it = InstSafeIterator<IterationType::ALL, IterationDirection::BACKWARD>(*block, startFrom);
            for (; it != block->AllInstsSafeReverse().end(); ++it) {
                auto *inst = *it;
                if (inst == nullptr) {
                    break;
                }
                if (inst->SetMarker(visited_.GetMarker()) || inst == def_ || inst == user) {
                    return true;
                }
                if (!GetDerived().HandleInst(inst)) {
                    return false;
                }
            }
        }
        for (auto pred : block->GetPredsBlocks()) {
            // Catch-begin block has edge from try-end block, and all try-blocks should be visited from this edge.
            // `def` can be placed inside try-block - after try-begin, so that visiting try-begin is wrong
            if (block->IsCatchBegin() && pred->IsTryBegin()) {
                continue;
            }
            if (!Walk(user, pred, pred->GetLastInst())) {
                return false;
            }
        }
        return true;
    }

    /// CRTP convenience methods
    Derived &GetDerived()
    {
        return *static_cast<Derived *>(this);
    }

    const Derived &GetDerived() const
    {
        return *static_cast<const Derived *>(this);
    }

    static std::pair<Inst *, const BasicBlock *> GetStartUserFrom(User &user)
    {
        auto *inst = user.GetInst();
        if (inst->IsPhi()) {
            // handle instructions on path between inst and the end of corresponding predecessor of Phi's block
            auto *startBb = inst->GetBasicBlock()->GetPredsBlocks()[user.GetBbNum()];
            auto *startInst = startBb->GetLastInst();
            return {startInst, startBb};
        }
        // handle instructions on path between inst and user
        return {inst->GetPrev(), inst->GetBasicBlock()};
    }

    Inst *def_;
    MarkerHolder visited_;
};

}  // namespace ark::compiler

#endif  // PANDA_COMPILER_OPTIMIZER_IR_USE_DEF_PATH_WALKER
