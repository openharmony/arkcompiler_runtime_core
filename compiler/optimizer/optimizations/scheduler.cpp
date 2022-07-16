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

#include "scheduler.h"

#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/ir/basicblock.h"
#include "compiler_logger.h"

namespace panda::compiler {
/* Instruction scheduling.
 * Current decisions/limitations
 *
 * 1. Scheduler pass is placed immediately before register allocator.
 * 2. It rearranges instructions only inside the basic block, but not between them.
 * 3. No liveness analysis, only calculating dependencies using barrier/users/alias information.
 * 4. No CPU pipeline/resource modeling, only having dependency costs.
 * 5. Forward list scheduling algorithm with standart critical-path-based priority.
 */
bool Scheduler::RunImpl()
{
    COMPILER_LOG(DEBUG, SCHEDULER) << "Run " << GetPassName();
    bool result = false;
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (!bb->IsEmpty() && !bb->IsStartBlock()) {
            result |= ScheduleBasicBlock(bb);
        }
    }
    COMPILER_LOG(DEBUG, SCHEDULER) << GetPassName() << " complete";
#ifndef NDEBUG
    GetGraph()->SetSchedulerComplete();
#endif  // NDEBUG
    return result;
}

// Dependency helper function
void Scheduler::AddDep(uint32_t *prio, Inst *from, Inst *to, uint32_t latency, Inst *barrier)
{
    if (from == to) {
        return;
    }
    COMPILER_LOG(DEBUG, SCHEDULER) << "Found dependency " << from->GetId() << " -> " << to->GetId() << " latency "
                                   << latency;
    // Estimate old cycle (without scheduling)
    ocycle_[from] = std::max(ocycle_[from], latency + ocycle_[to]);

    // Update instruction priority - "how high instruction is in dependency tree"
    *prio = std::max(*prio, latency + prio_[to]);

    // Do not add cross-barrier depenedencies into deps_
    if (barrier == nullptr || old_[to] > old_[barrier]) {
        if (deps_.at(from).count(to) == 1) {
            uint32_t old_latency = deps_.at(from).at(to);
            if (old_latency >= latency) {
                return;
            }
        } else {
            num_deps_[to]++;
        }
        deps_.at(from)[to] = latency;
    }
}

// Calculate priority and dependencies
bool Scheduler::BuildAllDeps(BasicBlock *bb)
{
    auto marker_holder = MarkerHolder(GetGraph());
    auto mrk = marker_holder.GetMarker();

    oprev_ = 0;
    num_barriers_ = 0;
    max_prio_ = 0;

    static constexpr uint32_t TOO_LONG_BB = 256;
    uint32_t num_inst = 0;
    uint32_t num_between = 0;
    uint32_t num_special = 0;
    Inst *last_barrier = nullptr;
    for (auto inst : bb->InstsSafeReverse()) {
        ProcessInst(inst, mrk, &num_inst, &num_between, &num_special, &last_barrier);

        if (num_special > TOO_LONG_BB || num_between > TOO_LONG_BB) {
            COMPILER_LOG(DEBUG, SCHEDULER) << "Block " << bb->GetId() << " seems too big for scheduling, skipping";
            Cleanup();
            return false;
        }
    }
    return true;
}

// One instruction deps
void Scheduler::ProcessInst(Inst *inst, Marker mrk, uint32_t *num_inst, uint32_t *num_between, uint32_t *num_special,
                            Inst **last_barrier)
{
    uint32_t prio = 0;
    uint32_t inst_latency = inst->Latency();
    bool barrier = inst->IsBarrier();

    (*num_between)++;
    old_.insert({inst, (*num_inst)++});
    ocycle_.insert({inst, ++oprev_});
    num_deps_.insert({inst, 0U});
    deps_.emplace(inst, GetGraph()->GetLocalAllocator()->Adapter());

    // Dependency to the barrier
    if (*last_barrier != nullptr) {
        AddDep(&prio, inst, *last_barrier, 1U, *last_barrier);
    }

    // Dependency from barrier
    if (barrier) {
        Inst *old_last_barrier = *last_barrier;
        *last_barrier = inst;
        num_barriers_++;
        *num_between = 0;
        for (auto user = inst->GetNext(); user != old_last_barrier; user = user->GetNext()) {
            AddDep(&prio, inst, user, 1U, *last_barrier);
        }
    }

    // Users
    for (auto &user_item : inst->GetUsers()) {
        auto user = user_item.GetInst();
        if (user->IsMarked(mrk)) {
            AddDep(&prio, inst, user, inst_latency, *last_barrier);
        }
    }

    if (inst->IsMemory() || inst->IsRefSpecial()) {
        ProcessMemory(inst, &prio, *last_barrier);
        (*num_special)++;
    }

    if (inst->CanThrow() || inst->IsRuntimeCall() || inst->IsSaveState()) {
        ProcessSpecial(inst, &prio, *last_barrier);
        (*num_special)++;
    }

    inst->SetMarker(mrk);
    prio_.insert({inst, prio});
    max_prio_ = std::max(max_prio_, prio);
    oprev_ = ocycle_[inst];
}

// Memory
void Scheduler::ProcessMemory(Inst *inst, uint32_t *prio, Inst *last_barrier)
{
    if (inst->IsRefSpecial()) {
        loads_.push_back(inst);
        return;
    }
    for (auto mem : stores_) {
        if (GetGraph()->CheckInstAlias(inst, mem) != AliasType::NO_ALIAS) {
            AddDep(prio, inst, mem, 1U, last_barrier);
        }
    }
    if (inst->IsStore()) {
        for (auto mem : loads_) {
            if (mem->IsLoad() && GetGraph()->CheckInstAlias(inst, mem) != AliasType::NO_ALIAS) {
                AddDep(prio, inst, mem, 1U, last_barrier);
            }
        }
        for (auto ct : special_) {
            AddDep(prio, inst, ct, 1U, last_barrier);
        }
        stores_.push_back(inst);
    } else {  // means inst->IsLoad()
        loads_.push_back(inst);
    }
}

// CanThrow or SaveState can't be rearranged, and stores can't be moved over them
void Scheduler::ProcessSpecial(Inst *inst, uint32_t *prio, Inst *last_barrier)
{
    for (auto mem : stores_) {
        AddDep(prio, inst, mem, 1U, last_barrier);
    }
    for (auto ct : special_) {
        AddDep(prio, inst, ct, 1U, last_barrier);
    }
    // 1. SafePoint also has this flag
    // 2. GC triggered inside can poison loaded reference value
    if (inst->IsRuntimeCall()) {
        for (auto mem : loads_) {
            if (mem->GetType() == DataType::REFERENCE) {
                AddDep(prio, inst, mem, 1U, last_barrier);
            }
        }
    }
    // We have to "restore" BoundsCheckI -> LoadArrayI dependency
    if (inst->GetOpcode() == Opcode::BoundsCheckI) {
        auto value = inst->CastToBoundsCheckI()->GetImm();
        // Remove loads with same immediate. No easy way to check arrays are same.
        for (auto load : loads_) {
            if (load->GetOpcode() == Opcode::LoadArrayPairI) {
                auto imm = load->CastToLoadArrayPairI()->GetImm();
                if (imm == value || imm + 1 == value) {
                    AddDep(prio, inst, load, 1U, last_barrier);
                }
            } else if (load->GetOpcode() == Opcode::LoadArrayI && load->CastToLoadArrayI()->GetImm() == value) {
                AddDep(prio, inst, load, 1U, last_barrier);
            }
        }
    }
    special_.push_back(inst);
}

// Rearranges instructions in the basic block using list scheduling algorithm.
bool Scheduler::ScheduleBasicBlock(BasicBlock *bb)
{
    COMPILER_LOG(DEBUG, SCHEDULER) << "Schedule BB " << bb->GetId();

    if (!BuildAllDeps(bb)) {
        return false;
    }

    // Schedule intervals between barriers
    uint32_t cycle = 0;
    uint32_t num_inst = 0;
    Inst *first = nullptr;
    for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
        bool barrier = inst->IsBarrier();
        num_inst++;
        inst->ClearMarkers();
        if (first == nullptr) {
            first = inst;
        }
        if (barrier || inst == bb->GetLastInst()) {
            Inst *last = nullptr;
            if (barrier) {
                last = inst->GetPrev();
                num_inst--;
            } else {
                last = inst;
            }
            if (num_inst > 1) {
                cycle += ScheduleInstsBetweenBarriers(first, last);
            } else if (num_inst == 1) {
                ASSERT(first->GetOpcode() != Opcode::LoadPairPart);
                sched_.push_back(first);
                cycle++;
            }
            if (barrier) {
                ASSERT(inst->GetOpcode() != Opcode::LoadPairPart);
                sched_.push_back(inst);
                cycle++;
            }
            num_inst = 0;
            first = nullptr;
        }
    }
    return FinalizeBB(bb, cycle);
}

bool Scheduler::FinalizeBB(BasicBlock *bb, uint32_t cycle)
{
    // Rearrange instructions in basic block according to schedule
    bool result = false;
    bool has_prev = false;
    uint32_t prev;
    for (auto inst : sched_) {
        auto cur = old_[inst];
        if (has_prev && prev - 1 != cur) {
            result = true;
        }
        prev = cur;
        has_prev = true;

        bb->EraseInst(inst);
        bb->AppendInst(inst);
    }

    if (result) {
        COMPILER_LOG(DEBUG, SCHEDULER) << "Stats for block " << bb->GetId() << ": old cycles = " << oprev_
                                       << ", num barriers = " << num_barriers_ << ", critical path = " << max_prio_
                                       << ", scheduled = " << cycle;
        GetGraph()->GetEventWriter().EventScheduler(bb->GetId(), bb->GetGuestPc(), oprev_, num_barriers_, max_prio_,
                                                    cycle);
    }

    Cleanup();
    return result;
}

void Scheduler::Cleanup()
{
    sched_.clear();
    loads_.clear();
    stores_.clear();
    special_.clear();
    old_.clear();
    ocycle_.clear();
    num_deps_.clear();
    prio_.clear();
    deps_.clear();
}

// Rearranges instructions between [first..last] inclusive, none of them are barriers.
uint32_t Scheduler::ScheduleInstsBetweenBarriers(Inst *first, Inst *last)
{
    COMPILER_LOG(DEBUG, SCHEDULER) << "SchedBetween " << first->GetId() << " and " << last->GetId();

    // Compare function for 'waiting' queue
    auto cmp_asap = [this](Inst *left, Inst *right) {
        return asap_[left] > asap_[right] || (asap_[left] == asap_[right] && old_[left] < old_[right]);
    };
    // Queue of instructions, which dependencies are scheduled already, but they are still not finished yet
    SchedulerPriorityQueue waiting(cmp_asap, GetGraph()->GetLocalAllocator()->Adapter());

    // Compare function for 'ready' queue
    auto cmp_prio = [this](Inst *left, Inst *right) {
        return prio_[left] < prio_[right] || (prio_[left] == prio_[right] && old_[left] < old_[right]);
    };
    // Queue of ready instructions
    SchedulerPriorityQueue ready(cmp_prio, GetGraph()->GetLocalAllocator()->Adapter());

    // Initialization, add leafs into 'waiting' queue
    uint32_t num_inst = 0;
    for (auto inst = first; inst != last->GetNext(); inst = inst->GetNext()) {
        asap_.insert({inst, 1U});
        if (num_deps_[inst] == 0) {
            COMPILER_LOG(DEBUG, SCHEDULER) << "Queue wait add " << inst->GetId();
            waiting.push(inst);
        }
        num_inst++;
    }

    // Scheduling
    uint32_t cycle = 1;
    while (num_inst > 0) {
        if (ready.empty()) {
            ASSERT(!waiting.empty());
            uint32_t nearest = asap_[waiting.top()];
            // Skipping cycles where we can't schedule any instruction
            if (nearest > cycle) {
                cycle = nearest;
            }
        }

        // Move from 'waiting' to 'ready'
        while (!waiting.empty()) {
            Inst *soonest = waiting.top();
            if (asap_[soonest] <= cycle) {
                waiting.pop();
                COMPILER_LOG(DEBUG, SCHEDULER) << "Queue ready moving " << soonest->GetId();
                ready.push(soonest);
            } else {
                break;
            }
        }
        ASSERT(!ready.empty());

        // Schedule top 'ready' instruction (together with glued, when necessary)
        num_inst -= SchedWithGlued(ready.top(), &waiting, cycle++);
        ready.pop();
    }

    // Cleanup
    asap_.clear();
    return cycle;
}

uint32_t Scheduler::SchedWithGlued(Inst *inst, SchedulerPriorityQueue *waiting, uint32_t cycle)
{
    uint32_t amount = 0;
    // Compare function for 'now' queue
    auto cmp_old = [this](Inst *left, Inst *right) { return old_[left] < old_[right]; };
    // Queue of instructions to schedule at current cycle
    SchedulerPriorityQueue now(cmp_old, GetGraph()->GetLocalAllocator()->Adapter());
    // Add inst into 'now'
    ASSERT(now.empty());
    now.push(inst);

    // Add glued instructions into 'now'
    if (inst->GetOpcode() == Opcode::LoadArrayPair || inst->GetOpcode() == Opcode::LoadArrayPairI) {
        for (auto &user_item : inst->GetUsers()) {
            auto user = user_item.GetInst();
            ASSERT(user->GetOpcode() == Opcode::LoadPairPart);
            now.push(user);
        }
    }

    [[maybe_unused]] constexpr auto MAX_NOW_SIZE = 3;
    ASSERT(now.size() <= MAX_NOW_SIZE);

    // Schedule them
    while (!now.empty()) {
        auto cur = now.top();
        now.pop();
        COMPILER_LOG(DEBUG, SCHEDULER) << "Scheduling " << cur->GetId() << " at cycle " << cycle;

        // Adjust all dependent instructions
        for (auto pair : deps_.at(cur)) {
            // Adjust asap
            uint32_t asap = asap_[pair.first];
            asap = std::max(asap, cycle + pair.second);
            asap_[pair.first] = asap;

            // Adjust num_deps
            uint32_t num_deps = num_deps_[pair.first];
            ASSERT(num_deps > 0);
            num_deps--;
            num_deps_[pair.first] = num_deps;

            // Glued instructions scheduled immediately, so they should not go into queue
            if (num_deps == 0 && pair.first->GetOpcode() != Opcode::LoadPairPart) {
                COMPILER_LOG(DEBUG, SCHEDULER) << "Queue wait add " << pair.first->GetId();
                waiting->push(pair.first);
            }
        }

        // Add into schedule
        sched_.push_back(cur);
        amount++;
    }

    return amount;
}
}  // namespace panda::compiler
