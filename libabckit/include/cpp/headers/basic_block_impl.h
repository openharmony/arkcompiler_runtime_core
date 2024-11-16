/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CPP_ABCKIT_BASIC_BLOCK_IMPL_H
#define CPP_ABCKIT_BASIC_BLOCK_IMPL_H

#include "./basic_block.h"

namespace abckit {

inline uint64_t BasicBlock::GetSuccCount() const
{
    uint64_t count = GetApiConfig()->cGapi_->bbGetSuccBlockCount(GetView());
    CheckError(GetApiConfig());
    return count;
}

inline BasicBlock BasicBlock::GetSuccByIdx(int idx) const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *abcBB = conf->cGapi_->bbGetSuccBlock(GetView(), idx);
    CheckError(conf);
    return BasicBlock(abcBB, conf);
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<BasicBlock> BasicBlock::GetSuccs() const
{
    const ApiConfig *conf = GetApiConfig();
    std::vector<BasicBlock> bBs;

    using EnumerateData = std::pair<std::vector<BasicBlock> *, const ApiConfig *>;
    EnumerateData enumerateData(&bBs, conf);

    conf->cGapi_->bbVisitSuccBlocks(GetView(), &enumerateData, [](AbckitBasicBlock *succ, void *data) {
        auto *vec = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        vec->push_back(BasicBlock(succ, config));
    });

    CheckError(conf);

    return bBs;
}

inline std::vector<Instruction> BasicBlock::GetInstructions() const
{
    std::vector<Instruction> insts;
    auto *conf = GetApiConfig();
    auto *inst = conf->cGapi_->bbGetFirstInst(GetView());
    CheckError(conf);
    while (inst != nullptr) {
        insts.push_back(Instruction(inst, conf));
        inst = conf->cGapi_->iGetNext(inst);
        CheckError(conf);
    }
    return insts;
}

inline Instruction BasicBlock::GetFirstInst() const
{
    auto *conf = GetApiConfig();
    auto *inst = conf->cGapi_->bbGetFirstInst(GetView());
    CheckError(conf);
    return Instruction(inst, conf);
}

inline BasicBlock &BasicBlock::AddInstFront(const Instruction &inst)
{
    GetApiConfig()->cGapi_->bbAddInstFront(GetView(), inst.GetView());
    CheckError(GetApiConfig());
    return *this;
}

inline BasicBlock &BasicBlock::AddInstBack(const Instruction &inst)
{
    GetApiConfig()->cGapi_->bbAddInstBack(GetView(), inst.GetView());
    CheckError(GetApiConfig());
    return *this;
}

}  // namespace abckit

#endif  // CPP_ABCKIT_BASIC_BLOCK_H
