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
class BasicBlock final : public View<AbckitBasicBlock *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Graph;
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
    BasicBlock &AddInstFront(const Instruction &inst);

    /**
     * @brief
     * @param inst
     * @return BasicBlock&
     */
    BasicBlock &AddInstBack(const Instruction &inst);

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

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    void GetSuccsInner(std::vector<BasicBlock> &bBs) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<BasicBlock> *, const ApiConfig *>;
        EnumerateData enumerateData(&bBs, conf);

        conf->cGapi_->bbVisitSuccBlocks(GetView(), &enumerateData, [](AbckitBasicBlock *succ, void *data) {
            auto *vec = static_cast<EnumerateData *>(data)->first;
            auto *config = static_cast<EnumerateData *>(data)->second;
            vec->push_back(BasicBlock(succ, config));
        });
    }

    BasicBlock(AbckitBasicBlock *bb, const ApiConfig *conf) : View(bb), conf_(conf) {};
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_BASIC_BLOCK_H
