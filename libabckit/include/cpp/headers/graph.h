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
#include "./instruction.h"
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
     * @brief Returns binary file that the current `Graph` is a part of.
     * @return Pointer to the `File`
     */
    const File *GetFile() const
    {
        return file_;
    }

    /**
     * @brief Get start basic block
     * @return `BasicBlock`
     */
    BasicBlock GetStartBb() const;

    /**
     * @brief Returns end basic block of current `Graph`.
     * @return End `BasicBlock`.
     */
    BasicBlock GetEndBb() const;

    /**
     * @brief Get blocks RPO
     * @return vector of `BasicBlock`
     */
    std::vector<BasicBlock> GetBlocksRPO() const;

    /**
     * @brief Get dynamic ISA
     * @return `DynamicIsa`
     */
    DynamicIsa DynIsa();

    /**
     * @brief Get static ISA
     * @return `StatIsa`
     */
    StaticIsa StatIsa();

    /**
     * @brief EnumerateBasicBlocksRpo
     * @param cb
     */
    void EnumerateBasicBlocksRpo(const std::function<bool(BasicBlock)> &cb) const;

    /**
     * @brief Creates I32 constant instruction and inserts it in start basic block of current `Graph`.
     * @return Created `Instruction`.
     * @param [ in ] val - value of created constant instruction.
     * @note Allocates
     */
    Instruction CreateConstantI32(int32_t val) const;

    /**
     * @brief Creates empty basic block.
     * @return Created `BasicBlock`.
     * @note Allocates
     */
    BasicBlock CreateEmptyBb() const;

    // Other Graph API's
    // ...

    /**
     * @brief Removes all basic blocks unreachable from start basic block.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     */
    void RunPassRemoveUnreachableBlocks() const;

protected:
    /**
     * @brief Get api config
     * @return `ApiConfig`
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

    /**
     * @brief Struct for using in callbacks
     */
    template <typename D>
    struct Payload {
        /**
         * @brief data
         */
        D data;
        /**
         * @brief config
         */
        const ApiConfig *config;
        /**
         * @brief resource
         */
        const Graph *resource;
    };

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

    Graph(AbckitGraph *graph, const ApiConfig *conf, const File *file) : Resource(graph), conf_(conf), file_(file)
    {
        SetDeleter(std::make_unique<GraphDeleter>(conf_, *this));
    };
    // for interop with API
    const ApiConfig *conf_;
    const File *file_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_GRAPH_H
