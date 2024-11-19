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

namespace abckit {

inline BasicBlock Graph::GetStartBb() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitBasicBlock *bb = conf->cGapi_->gGetStartBasicBlock(GetResource());
    CheckError(conf);
    return BasicBlock(bb, conf);
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<BasicBlock> Graph::GetBlocksRPO() const
{
    std::vector<BasicBlock> blocks;
    const ApiConfig *conf = GetApiConfig();

    using EnumerateData = std::pair<std::vector<BasicBlock> *, const ApiConfig *>;
    EnumerateData enumerateData(&blocks, conf);

    conf->cGapi_->gVisitBlocksRpo(GetResource(), &enumerateData, [](AbckitBasicBlock *bb, void *data) {
        auto *vec = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        vec->push_back(BasicBlock(bb, config));
    });

    CheckError(conf);

    return blocks;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline void Graph::EnumerateBasicBlocksRpo(const std::function<void(BasicBlock)> &cb) const
{
    struct Payload {
        const std::function<void(BasicBlock)> &callback;
        const ApiConfig *config;
    };
    Payload payload {cb, GetApiConfig()};

    GetApiConfig()->cGapi_->gVisitBlocksRpo(GetResource(), &payload, [](AbckitBasicBlock *bb, void *data) -> void {
        const auto &payload = *static_cast<Payload *>(data);
        payload.callback(BasicBlock(bb, payload.config));
    });
    CheckError(GetApiConfig());
}

}  // namespace abckit

#endif  // CPP_ABCKIT_GRAPH_IMPL_H
