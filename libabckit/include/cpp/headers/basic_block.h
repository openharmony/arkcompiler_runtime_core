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

#ifndef CPP_ABCKIT_BASIC_BLOCK_H
#define CPP_ABCKIT_BASIC_BLOCK_H

#include "./base_classes.h"
#include "./instruction.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace abckit {

/**
 * @brief BasicBlock
 */
class BasicBlock final : public ViewInResource<AbckitBasicBlock *, const Graph *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Graph;
    /// @brief to access private constructor
    friend class Instruction;
    /// @brief abckit::DefaultHash<BasicBlock>
    friend class abckit::DefaultHash<BasicBlock>;

public:
    /**
     * @brief Constructor
     * @param other
     */
    BasicBlock(const BasicBlock &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return BasicBlock&
     */
    BasicBlock &operator=(const BasicBlock &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    BasicBlock(BasicBlock &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return BasicBlock&
     */
    BasicBlock &operator=(BasicBlock &&other) = default;

    /**
     * @brief Destructor
     */
    ~BasicBlock() override = default;

    /**
     * @brief Get the Succ Count object
     * @return uint64_t
     */
    uint64_t GetSuccCount() const;

    /**
     * @brief Get the Succ By Idx object
     * @param idx
     * @return BasicBlock
     */
    BasicBlock GetSuccByIdx(int idx) const;

    /**
     * @brief Get the Succs object
     * @return std::vector<BasicBlock>
     */
    std::vector<BasicBlock> GetSuccs() const;

    /**
     * @brief
     * @param inst
     * @return BasicBlock&
     */
    const BasicBlock &AddInstFront(const Instruction &inst) const;

    /**
     * @brief
     * @param inst
     * @return BasicBlock&
     */
    const BasicBlock &AddInstBack(const Instruction &inst) const;

    /**
     * @brief Get the Instructions object
     * @return std::vector<Instruction>
     */
    std::vector<Instruction> GetInstructions() const;

    /**
     * @brief Get the First Inst object
     *
     * @return Instruction
     */
    Instruction GetFirstInst() const;

    /**
     * @brief Returns last instruction.
     * @return `Instruction`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Instruction GetLastInst() const;

    /**
     * @brief Enumerates basic blocks successing to the current `BasicBlock`, invoking callback `cb` for each basic
     * block.
     * @param [ in ] cb - Callback that will be invoked.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `BasicBlock` is false.
     */
    void VisitSuccBlocks(const std::function<void(BasicBlock)> &cb) const;

    /**
     * @brief Appends successor to the end of current successors list.
     * @param [ in ] succBlock - Basic block to be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `BasicBlock` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `succBlock` is false.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `Graph`s owning current `BasicBlock` and `succBlock`.
     * differs.
     */
    void AppendSuccBlock(const BasicBlock &succBlock) const;

    /**
     * @brief Deletes the successor and shifts the rest if there were successors with a larger index.
     * @param [ in ] index - Index of successor to be deleted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `BasicBlock` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `index` is larger than current `BasicBlock` successors quantity.
     */
    void EraseSuccBlock(uint32_t index) const;

    /**
     * @brief Returns graph owning current `BasicBlock`.
     * @return Pointer to `Graph`. It should be nullptr if current `Graph` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `BasicBlock` is false.
     */
    const Graph *GetGraph() const;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    void GetSuccsInner(std::vector<BasicBlock> &bBs) const
    {
        Payload<std::vector<BasicBlock> *> payload {&bBs, GetApiConfig(), GetResource()};

        GetApiConfig()->cGapi_->bbVisitSuccBlocks(GetView(), &payload, [](AbckitBasicBlock *succ, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<BasicBlock> *> *>(data);
            payload.data->push_back(BasicBlock(succ, payload.config, payload.resource));
            return true;
        });
    }

    BasicBlock(AbckitBasicBlock *bb, const ApiConfig *conf, const Graph *graph) : ViewInResource(bb), conf_(conf)
    {
        SetResource(graph);
    };
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_BASIC_BLOCK_H
