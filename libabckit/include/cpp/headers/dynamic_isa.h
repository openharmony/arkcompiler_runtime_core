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

#ifndef CPP_ABCKIT_DYNAMIC_ISA_H
#define CPP_ABCKIT_DYNAMIC_ISA_H

#include "./instruction.h"

#include <string>
#include <string_view>

namespace abckit {

class Graph;

// Third type of Entity? Or just a view?
/**
 * @brief DynamicIsa
 */
class DynamicIsa final {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /**
     * Field to access private constructor
     */
    friend class Graph;

public:
    /**
     * @brief Deleted constructor
     * @param other
     */
    DynamicIsa(const DynamicIsa &other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     * @return DynamicIsa
     */
    DynamicIsa &operator=(const DynamicIsa &other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     */
    DynamicIsa(DynamicIsa &&other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     * @return DynamicIsa
     */
    DynamicIsa &operator=(DynamicIsa &&other) = delete;

    /**
     * @brief Destructor
     */
    ~DynamicIsa() = default;

    // Rvalue annotated so we can call it only in callchain context

    /**
     * @brief Creates instruction with opcode LOAD_STRING. This instruction loads the string `str` into `acc`.
     * @param str to load
     * @return `Instruction`
     */
    Instruction CreateLoadString(std::string_view str) &&;

    /**
     * @brief Creates instruction with opcode TRYLDGLOBALBYNAME. Loads the global variable of the name `string`.
     * If the global variable `string` does not exist, an exception is thrown.
     * @param str to load
     * @return `Instruction`
     */
    Instruction CreateTryldglobalbyname(std::string_view str) &&;

    /**
     * @brief Creates instruction with opcode CALLARG1. This instruction invokes the function object stored in `acc`
     * with `input0` argument
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing argument.
     * @return `Instruction`
     */
    Instruction CreateCallArg1(const Instruction &acc, const Instruction &input0) &&;

    /**
     * @brief Creates instruction with opcode TRYLDGLOBALBYNAME. Loads the global variable of the name `str`.
     * If the global variable `str` does not exist, an exception is thrown.
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] str - String containing name of global variable.
     * @return `Instruction`
     */
    Instruction CreateLdObjByName(const Instruction &acc, std::string_view str) &&;

    /**
     * @brief Creates instruction with opcode CALLTHIS0.
     * Sets the value of this as `input0`, invokes the function object stored in returned instruction.
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     * @return `Instruction`
     */
    Instruction CreateCallThis0(const Instruction &acc, const Instruction &input0) &&;

    /**
     * @brief Creates instruction with opcode SUB2. This instruction computes the binary operation `input0 - acc`, and
     * stores the result in returned instruction.
     * @return Created `Instruction`.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @return `Instruction`
     */
    Instruction CreateSub2(const Instruction &acc, const Instruction &input0) &&;

    /**
     * @brief Creates instruction with opcode GREATEREQ. This instruction computes the binary operation `input0 >= acc`.
     * @return Created `Instruction`.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is false.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `Graph` owning `acc` and current `Graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `Graph` owning `input0` and current `Graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `Graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    Instruction CreateGreatereq(const Instruction &acc, const Instruction &input0) &&;

    /**
     * @brief Creates instruction with opcode IF.
     * @return Created `Instruction`.
     * @param [ in ] input - Instruction that will be compared to zero.
     * @param [ in ] cc - Condition code.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cc` is `ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE` or
     * if `cc` is not `ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ` or `ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE`.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `File`s owning `Graph` and `input` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `Graph`'s mode is not DYNAMIC.
     */
    Instruction CreateIf(const Instruction &input, enum AbckitIsaApiDynamicConditionCode cc) &&;

    /**
     * @brief Creates instruction with opcode RETURN. This instruction returns `acc`.
     * @return Created `Instruction`.
     * @param [ in ] acc - Instruction to be returned.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is false.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `File`s owning `Graph` and `acc` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `Graph`'s mode is not DYNAMIC.
     */
    Instruction CreateReturn(const Instruction &acc) &&;

    /**
     * @brief Creates instruction with opcode NEWOBJRANGE. This instruction invokes the constructor of `instClass` with
     * arguments `instArgs...` to create a class instance, and stores the instance in returned `Instruction`.
     * @tparam Instructions - a parameter pack for abckit::Instruction
     * @param [ in ] instClass - Inst containing class object
     * @param [ in ] instArgs - Insts containing constructor arguments
     * @return `Instruction`
     */
    template <class... Instructions>
    Instruction CreateNewobjrange(const Instruction &instClass, Instructions &&...instArgs) &&;

    // Other dynamic API methods declarations

    /**
     * @brief Get the Import Descriptor object
     * @param inst
     * @return core::ImportDescriptor
     */
    core::ImportDescriptor GetImportDescriptor(const Instruction &inst);

private:
    explicit DynamicIsa(const Graph &graph) : graph_(graph) {};
    const Graph &graph_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_DYNAMIC_ISA_H
