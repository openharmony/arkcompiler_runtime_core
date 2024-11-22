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
    return BasicBlock(abcBB, conf, GetResource());
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<BasicBlock> BasicBlock::GetSuccs() const
{
    std::vector<BasicBlock> bBs;
    Payload<std::vector<BasicBlock> *> payload {&bBs, GetApiConfig(), GetResource()};

    GetApiConfig()->cGapi_->bbVisitSuccBlocks(GetView(), &payload, [](AbckitBasicBlock *succ, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<BasicBlock> *> *>(data);
        payload.data->push_back(BasicBlock(succ, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return bBs;
}

inline std::vector<Instruction> BasicBlock::GetInstructions() const
{
    std::vector<Instruction> insts;
    auto *conf = GetApiConfig();
    auto *inst = conf->cGapi_->bbGetFirstInst(GetView());
    CheckError(conf);
    while (inst != nullptr) {
        insts.push_back(Instruction(inst, conf, GetResource()));
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
    return Instruction(inst, conf, GetResource());
}

inline Instruction BasicBlock::GetLastInst() const
{
    auto *conf = GetApiConfig();
    auto *inst = conf->cGapi_->bbGetLastInst(GetView());
    CheckError(conf);
    return Instruction(inst, conf, GetResource());
}

inline const BasicBlock &BasicBlock::AddInstFront(const Instruction &inst) const
{
    GetApiConfig()->cGapi_->bbAddInstFront(GetView(), inst.GetView());
    CheckError(GetApiConfig());
    return *this;
}

inline const BasicBlock &BasicBlock::AddInstBack(const Instruction &inst) const
{
    GetApiConfig()->cGapi_->bbAddInstBack(GetView(), inst.GetView());
    CheckError(GetApiConfig());
    return *this;
}

inline void BasicBlock::VisitSuccBlocks(const std::function<void(BasicBlock)> &cb) const
{
    Payload<const std::function<void(BasicBlock)> &> payload {cb, GetApiConfig(), GetResource()};

    GetApiConfig()->cGapi_->bbVisitSuccBlocks(GetView(), &payload, [](AbckitBasicBlock *succ, void *data) {
        const auto &payload = *static_cast<Payload<const std::function<void(BasicBlock)> &> *>(data);
        payload.data(BasicBlock(succ, payload.config, payload.resource));
        return true;
    });
    CheckError(GetApiConfig());
}

inline void BasicBlock::AppendSuccBlock(const BasicBlock &succBlock) const
{
    GetApiConfig()->cGapi_->bbAppendSuccBlock(GetView(), succBlock.GetView());
    CheckError(GetApiConfig());
}

inline void BasicBlock::EraseSuccBlock(uint32_t index) const
{
    GetApiConfig()->cGapi_->bbEraseSuccBlock(GetView(), index);
    CheckError(GetApiConfig());
}

}  // namespace abckit

#endif  // CPP_ABCKIT_BASIC_BLOCK_H
