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

#ifndef COMPILER_OPTIMIZER_IR_ANALYSIS_H
#define COMPILER_OPTIMIZER_IR_ANALYSIS_H

#include "graph.h"

#include <optional>

namespace panda::compiler {

/// The file contains small analysis functions which can be used in different passes
class Inst;
// returns Store value, for StoreArrayPair and StoreArrayPairI saved not last store value in second_value
Inst *InstStoredValue(Inst *inst, Inst **second_value);
Inst *InstStoredValue(Inst *inst);
bool HasOsrEntryBetween(Inst *dominate_inst, Inst *inst);
bool HasOsrEntryBetween(BasicBlock *dominate_bb, BasicBlock *bb);
bool HasTryBlockBetween(Inst *dominate_inst, Inst *inst);
bool IsSuitableForImplicitNullCheck(const Inst *inst);
bool IsInstNotNull(const Inst *inst);
SaveStateInst *CopySaveState(Graph *graph, SaveStateInst *inst);
bool CheckObjectRec(Inst *object, const Inst *user, const BasicBlock *block, Inst *start_from, Marker visited,
                    Inst **failed_ss = nullptr);
std::optional<bool> IsIfInverted(BasicBlock *phi_block, IfImmInst *if_imm);

// If object input has known class, return pointer to the class, else returns nullptr
RuntimeInterface::ClassPtr GetClassPtrForObject(Inst *inst, size_t input_num = 0);

inline bool IsInstInDifferentBlocks(Inst *i1, Inst *i2)
{
    return i1->GetBasicBlock() != i2->GetBasicBlock();
}

// This function bypass all blocks and delete 'SaveStateOSR' if the block is no longer the header of the loop
void CleanupGraphSaveStateOSR(Graph *graph);

class IsSaveState;
class IsSaveStateCanTriggerGc;
// returns true is there is SaveState/SafePoint between instructions
template <typename T = IsSaveState>
bool HasSaveStateBetween(Inst *dom_inst, Inst *inst);

/**
 * Functions below are using for create bridge in SaveStates between source instruction and target instruction.
 * It use in GVN etc. It inserts `source` instruction into `SaveStates` on each path between `source` and
 * `target` instructions to save the object in case GC is triggered on this path.
 * Instructions on how to use it: compiler/docs/bridges.md
 */
class SaveStateBridgesBuilder {
public:
    ArenaVector<Inst *> *SearchMissingObjInSaveStates(Graph *graph, Inst *source, Inst *target,
                                                      Inst *stop_search = nullptr, BasicBlock *target_block = nullptr);
    void CreateBridgeInSS(Inst *source, ArenaVector<Inst *> *bridges);
    void SearchAndCreateMissingObjInSaveState(Graph *graph, Inst *source, Inst *target, Inst *stop_search = nullptr,
                                              BasicBlock *target_block = nullptr);
    void FixInstUsageInSS(Graph *graph, Inst *inst);
    void FixSaveStatesInBB(BasicBlock *block);
    void FixPhisWithCheckInputs(BasicBlock *block);
    void DumpBridges(std::ostream &out, Inst *source, ArenaVector<Inst *> *bridges);

private:
    void SearchSSOnWay(BasicBlock *block, Inst *start_from, Inst *source_inst, Marker visited,
                       ArenaVector<Inst *> *bridges, Inst *stop_search);
    bool IsSaveStateForGc(Inst *inst);
    void ProcessSSUserPreds(Graph *graph, Inst *inst, Inst *target_inst);
    void SearchInSaveStateAndFillBridgeVector(Inst *inst, Inst *searched_inst, ArenaVector<Inst *> *bridges);
    void FixUsageInstInOtherBB(BasicBlock *block, Inst *inst);
    void FixUsagePhiInBB(BasicBlock *block, Inst *inst);
    void DeleteUnrealObjInSaveState(Inst *ss);
    /**
     * Pointer to moved out to class for reduce memory usage in each pair of equal instructions.
     * When using functions, it looks like we work as if every time get a new vector,
     * but one vector is always used and cleaned before use.
     */
    ArenaVector<Inst *> *bridges_ {nullptr};
};

class InstAppender {
public:
    explicit InstAppender(BasicBlock *block, Inst *insert_after = nullptr) : block_(block), prev_(insert_after) {}
    ~InstAppender() = default;
    DEFAULT_MOVE_SEMANTIC(InstAppender);
    NO_COPY_SEMANTIC(InstAppender);

    void Append(Inst *inst);
    void Append(std::initializer_list<Inst *> instructions);

private:
    BasicBlock *block_;
    Inst *prev_ {nullptr};
};

bool StoreValueCanBeObject(Inst *inst);

bool IsConditionEqual(const Inst *inst0, const Inst *inst1, bool inverted);

}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_ANALYSIS_H
