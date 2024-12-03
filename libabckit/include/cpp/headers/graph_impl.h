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

#ifndef CPP_ABCKIT_GRAPH_IMPL_H
#define CPP_ABCKIT_GRAPH_IMPL_H

#include "./graph.h"
#include "./dynamic_isa.h"

namespace abckit {

inline DynamicIsa Graph::DynIsa()
{
    return DynamicIsa(*this);
}

inline StaticIsa Graph::StatIsa()
{
    return StaticIsa(*this);
}

inline BasicBlock Graph::GetStartBb() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *bb = conf->cGapi_->gGetStartBasicBlock(GetResource());
    CheckError(conf);
    return BasicBlock(bb, conf, this);
}

inline BasicBlock Graph::GetEndBb() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *bb = conf->cGapi_->gGetEndBasicBlock(GetResource());
    CheckError(conf);
    return BasicBlock(bb, conf, this);
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<BasicBlock> Graph::GetBlocksRPO() const
{
    std::vector<BasicBlock> blocks;
    Payload<std::vector<BasicBlock> *> payload {&blocks, GetApiConfig(), this};

    GetApiConfig()->cGapi_->gVisitBlocksRpo(GetResource(), &payload, [](AbckitBasicBlock *bb, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<BasicBlock> *> *>(data);
        payload.data->push_back(BasicBlock(bb, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return blocks;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline void Graph::EnumerateBasicBlocksRpo(const std::function<bool(BasicBlock)> &cb) const
{
    Payload<const std::function<void(BasicBlock)> &> payload {cb, GetApiConfig(), this};

    GetApiConfig()->cGapi_->gVisitBlocksRpo(GetResource(), &payload, [](AbckitBasicBlock *bb, void *data) -> bool {
        const auto &payload = *static_cast<Payload<const std::function<void(BasicBlock)> &> *>(data);
        payload.data(BasicBlock(bb, payload.config, payload.resource));
        return true;
    });
    CheckError(GetApiConfig());
}

inline Instruction Graph::CreateConstantI32(int32_t val) const
{
    AbckitInst *inst = GetApiConfig()->cGapi_->gCreateConstantI32(GetResource(), val);
    CheckError(GetApiConfig());
    return Instruction(inst, GetApiConfig(), this);
}

inline BasicBlock Graph::CreateEmptyBb() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *bb = conf->cGapi_->bbCreateEmpty(GetResource());
    CheckError(conf);
    return BasicBlock(bb, conf, this);
}

inline void Graph::RunPassRemoveUnreachableBlocks() const
{
    const ApiConfig *conf = GetApiConfig();
    conf->cGapi_->gRunPassRemoveUnreachableBlocks(GetResource());
    CheckError(conf);
}

}  // namespace abckit

#endif  // CPP_ABCKIT_GRAPH_IMPL_H
