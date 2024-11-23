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

#ifndef CPP_ABCKIT_INSTRUCTION_H
#define CPP_ABCKIT_INSTRUCTION_H

#include "./base_classes.h"

#include <functional>

namespace abckit {

/**
 * @brief Instruction
 */
class Instruction final : public View<AbckitInst *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.

    /// @brief To access private constructor
    friend class BasicBlock;
    /// @brief To access private constructor
    friend class StaticIsa;
    /// @brief To access private constructor
    friend class DynamicIsa;
    /// @brief abckit::DefaultHash<Instruction>
    friend class abckit::DefaultHash<Instruction>;

public:
    /**
     * @brief Construct a new Instruction object
     * @param other
     */
    Instruction(const Instruction &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Instruction
     */
    Instruction &operator=(const Instruction &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    Instruction(Instruction &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Instruction
     */
    Instruction &operator=(Instruction &&other) = default;

    /**
     * @brief Destructor
     */
    ~Instruction() override = default;

    /**
     * @brief Inserts `newInst` instruction after `ref` instruction into `ref`'s basic block.
     * @param inst
     * @return Instruction&
     */
    Instruction &InsertAfter(const Instruction &inst);

    /**
     * @brief Inserts `newInst` instruction before `ref` instruction into `ref`'s basic block.
     * @param inst
     * @return Instruction&
     */
    Instruction &InsertBefore(const Instruction &inst);

    /**
     * @brief Get the Opcode Dyn object
     * @return AbckitIsaApiDynamicOpcode
     */
    AbckitIsaApiDynamicOpcode GetOpcodeDyn() const;

    /**
     * @brief Get the Opcode Stat object
     * @return AbckitIsaApiStaticOpcode
     */
    AbckitIsaApiStaticOpcode GetOpcodeStat() const;

    /**
     * @brief Get the String object
     * @return std::string_view
     */
    std::string_view GetString() const;

    /**
     * @brief Get the Next object
     * @return Instruction
     */
    Instruction GetNext() const;

    /**
     * @brief Get the Prev object
     * @return Instruction
     */
    Instruction GetPrev() const;

    /**
     * @brief Get the Function object
     * @return core::Function
     */
    core::Function GetFunction() const;

    /**
     * @brief Returns number of inputs.
     * @return Number of inputs.
     */
    uint32_t GetInputCount() const;

    /**
     * @brief Returns `inst` input under given `index`.
     * @param [ in ] index - Index of input to be returned.
     * @return input `Instruction`
     */
    Instruction GetInput(uint32_t index) const;

    /**
     * @brief Sets input, overwrites existing input.
     * @param [ in ] index - Index of input to be set.
     * @param [ in ] input - Input instruction to be set.
     */
    void SetInput(uint32_t index, const Instruction &input);

    /**
     * @brief Enumerates `insts` user instructions, invoking callback `cb` for each user instruction.
     * @param cb - Callback that will be invoked.
     */
    void VisitUsers(const std::function<void(Instruction)> &cb) const;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    /**
     * @brief Construct a new Instruction object
     * @param inst
     * @param conf
     */
    Instruction(AbckitInst *inst, const ApiConfig *conf) : View(inst), conf_(conf) {};
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_INSTRUCTION_H
