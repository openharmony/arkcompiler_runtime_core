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

#include "./base_classes.h"
#include "./basic_block.h"
#include "./dynamic_isa.h"
#include "../../../src/include_v2/cpp/headers/static_isa.h"

#include <memory>
#include <vector>

namespace abckit {

/**
 * @brief Graph
 */
class Graph final : public Resource<AbckitGraph *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.

    /// @brief To access private constructor
    friend class core::Function;
    /// @brief To access private constructor
    friend class DynamicIsa;
    /// @brief To access private constructor
    friend class StaticIsa;

public:
    /**
     * @brief Deleted constructor
     * @param other
     */
    Graph(const Graph &other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     * @return Graph
     */
    Graph &operator=(const Graph &other) = delete;

    /**
     * @brief Constructor
     * @param other
     */
    Graph(Graph &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Graph
     */
    Graph &operator=(Graph &&other) = default;

    /**
     * @brief Destructor
     */
    ~Graph() override = default;

    /**
     * @brief Get start basic block
     * @return `BasicBlock`
     */
    BasicBlock GetStartBb() const;

    /**
     * @brief Get blocks RPO
     * @return vector of `BasicBlock`
     */
    std::vector<BasicBlock> GetBlocksRPO() const;

    /**
     * @brief Get dyn isa
     * @return `DynamicIsa`
     */
    DynamicIsa DynIsa()
    {
        return DynamicIsa(*this);
    }

    /**
     * @brief Get static isa
     * @return `StatIsa`
     */
    StaticIsa StatIsa()
    {
        return StaticIsa(*this);
    }

    /**
     * @brief EnumerateBasicBlocksRpo
     * @param cb
     */
    void EnumerateBasicBlocksRpo(const std::function<void(BasicBlock)> &cb) const;

    // Other Graph API's
    // ...

protected:
    /**
     * @brief Get api config
     * @return `ApiConfig`
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    class GraphDeleter final : public IResourceDeleter {
    public:
        /**
         * @brief Constructor
         * @param conf
         * @param graph
         */
        GraphDeleter(const ApiConfig *conf, const Graph &graph) : conf_(conf), deleterGraph_(graph) {};

        /**
         * @brief Deleted constructor
         * @param other
         */
        GraphDeleter(const GraphDeleter &other) = delete;

        /**
         * @brief Deleted constructor
         * @param other
         */
        GraphDeleter &operator=(const GraphDeleter &other) = delete;

        /**
         * @brief Deleted constructor
         * @param other
         */
        GraphDeleter(GraphDeleter &&other) = delete;

        /**
         * @brief Deleted constructor
         * @param other
         */
        GraphDeleter &operator=(GraphDeleter &&other) = delete;

        /**
         * @brief Destructor
         */
        ~GraphDeleter() override = default;

        /**
         * @brief Delete resource
         */
        void DeleteResource() override
        {
            conf_->cApi_->destroyGraph(deleterGraph_.GetResource());
        }

    private:
        const ApiConfig *conf_;
        const Graph &deleterGraph_;
    };

    Graph(AbckitGraph *graph, const ApiConfig *conf) : Resource(graph), conf_(conf)
    {
        SetDeleter(std::make_unique<GraphDeleter>(conf_, *this));
    };
    // for interop with API
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_GRAPH_H
