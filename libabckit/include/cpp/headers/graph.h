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

#ifndef CPP_ABCKIT_GRAPH_H
#define CPP_ABCKIT_GRAPH_H

#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/isa/isa_dynamic.h"
#include "libabckit/src/include_v2/c/isa/isa_static.h"
#include "libabckit/include/cpp/headers/base_classes.h"
#include "libabckit/include/cpp/headers/config.h"
#include "libabckit/include/cpp/headers/declarations.h"
#include "libabckit/include/cpp/headers/basic_block.h"
#include "libabckit/include/cpp/headers/dynamic_isa.h"
#include "libabckit/src/include_v2/cpp/headers/static_isa.h"

#include <memory>
#include <vector>

namespace abckit {

class Graph final : public Resource<AbckitGraph *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class core::Function;
    friend class DynamicIsa;
    friend class StaticIsa;

public:
    Graph(const Graph &other) = delete;
    Graph &operator=(const Graph &other) = delete;
    Graph(Graph &&other) = default;
    Graph &operator=(Graph &&other) = default;
    ~Graph() override = default;

    BasicBlock GetStartBb() const;
    std::vector<BasicBlock> GetBlocksRPO() const;

    DynamicIsa DynIsa()
    {
        return DynamicIsa(*this);
    }

    StaticIsa StatIsa()
    {
        return StaticIsa(*this);
    }

    // Other Graph API's
    // ...

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    class GraphDeleter final : public IResourceDeleter {
    public:
        GraphDeleter(const ApiConfig *conf, const Graph &graph) : conf_(conf), deleterGraph_(graph) {};
        GraphDeleter(const GraphDeleter &other) = delete;
        GraphDeleter &operator=(const GraphDeleter &other) = delete;
        GraphDeleter(GraphDeleter &&other) = delete;
        GraphDeleter &operator=(GraphDeleter &&other) = delete;
        ~GraphDeleter() override = default;

        void DeleteResource() override
        {
            // NOTE(nsizov): add Graph destroying when C API will stop delete graph on functionSetGraph
            (void)conf_;
            (void)deleterGraph_;
        }

    private:
        const ApiConfig *conf_;
        const Graph &deleterGraph_;
    };

    inline void GetBlocksRPOInner(std::vector<BasicBlock> &blocks) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<BasicBlock> *, const ApiConfig *>;
        EnumerateData enumerateData(&blocks, conf);

        conf->cGapi_->gVisitBlocksRpo(GetResource(), (void *)&enumerateData, [](AbckitBasicBlock *bb, void *data) {
            auto *vec = static_cast<EnumerateData *>(data)->first;
            auto *config = static_cast<EnumerateData *>(data)->second;
            vec->push_back(BasicBlock(bb, config));
        });
    }

    Graph(AbckitGraph *graph, const ApiConfig *conf) : Resource(graph), conf_(conf)
    {
        SetDeleter(std::make_unique<GraphDeleter>(conf_, *this));
    };
    // for interop with API
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_GRAPH_H
