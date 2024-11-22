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
#include "./core/import_descriptor.h"

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
    Instruction CreateLoadString(std::string_view str) const &&;

    /**
     * @brief Creates instruction with opcode TRYLDGLOBALBYNAME. Loads the global variable of the name `string`.
     * If the global variable `string` does not exist, an exception is thrown.
     * @param str to load
     * @return `Instruction`
     */
    Instruction CreateTryldglobalbyname(std::string_view str) const &&;

    /**
     * @brief Creates instruction with opcode CALLARG1. This instruction invokes the function object stored in `acc`
     * with `input0` argument
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing argument.
     * @return `Instruction`
     */
    Instruction CreateCallArg1(const Instruction &acc, const Instruction &input0) const &&;

    /**
     * @brief Creates instruction with opcode TRYLDGLOBALBYNAME. Loads the global variable of the name `str`.
     * If the global variable `str` does not exist, an exception is thrown.
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] str - String containing name of global variable.
     * @return `Instruction`
     */
    Instruction CreateLdObjByName(const Instruction &acc, std::string_view str) const &&;

    /**
     * @brief Creates instruction with opcode CALLTHIS0.
     * Sets the value of this as `input0`, invokes the function object stored in `acc`.
     * @return Created `Instruction`
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `Graph` is a `View` over NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is a `View` over NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is a `View` over NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `Graph` owning one of `Instruction`s and current `Graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if current `Graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    Instruction CreateCallThis0(const Instruction &acc, const Instruction &input0) const &&;

    /**
     * @brief Creates instruction with opcode CALLTHIS2.
     * Sets the value of this as `input0`,
     * invokes the function object stored in `acc` with arguments `input1`, `input2`.
     * @return Created `Instruction`.
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     * @param [ in ] input1 - First argument.
     * @param [ in ] input2 - Second argument.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `Graph` is a `View` over NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is a `View` over NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is a `View` over NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is a `View` over NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input2` is a `View` over NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `Graph` owning one of `Instruction`s and current `Graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if current `Graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    Instruction CreateCallThis2(const Instruction &acc, const Instruction &input0, const Instruction &input1,
                                const Instruction &input2) const &&;

    /**
     * @brief Creates instruction with opcode SUB2. This instruction computes the binary operation `input0 - acc`, and
     * stores the result in returned instruction.
     * @return Created `Instruction`.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @return `Instruction`
     */
    Instruction CreateSub2(const Instruction &acc, const Instruction &input0) const &&;

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
    Instruction CreateGreatereq(const Instruction &acc, const Instruction &input0) const &&;

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
    Instruction CreateIf(const Instruction &input, enum AbckitIsaApiDynamicConditionCode cc) const &&;

    /**
     * @brief Creates instruction with opcode RETURN. This instruction returns `acc`.
     * @return Created `Instruction`.
     * @param [ in ] acc - Instruction to be returned.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is false.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `File`s owning `Graph` and `acc` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `Graph`'s mode is not DYNAMIC.
     */
    Instruction CreateReturn(const Instruction &acc) const &&;

    /**
     * @brief Creates instruction with opcode NEWOBJRANGE. This instruction invokes the constructor of `instClass` with
     * arguments `instArgs...` to create a class instance, and stores the instance in returned `Instruction`.
     * @tparam Instructions - a parameter pack for abckit::Instruction
     * @param [ in ] instClass - Inst containing class object
     * @param [ in ] instArgs - Insts containing constructor arguments
     * @return `Instruction`
     */
    template <class... Instructions>
    Instruction CreateNewobjrange(const Instruction &instClass, Instructions &&...instArgs) const &&;

    /**
     * @brief Creates instruction with opcode LDEXTERNALMODULEVAR. Loads the external module variable.
     * @return Created `Instruction`.
     * @param [ in ] idesc - Import descriptor to load.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `File` owning `idesc` and `File` owning `this->GetGraph()` differs.`
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if this `Graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    Instruction CreateLdExternalModuleVar(const core::ImportDescriptor &idesc) const &&;

    /**
     * @brief Creates instruction with opcode THROW_UNDEFINEDIFHOLEWITHNAME. This instruction throws the exception that
     * `string` is `undefined` if `acc` is hole.
     * @return Created `Instruction`.
     * @param [ in ] acc - Inst containing value of the object.
     * @param [ in ] string - String for exception.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(acc)` results in `false`
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if this `Graph` and `Graph` owning `acc` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if this `Graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    Instruction CreateThrowUndefinedIfHoleWithName(const Instruction &acc, std::string_view string) const &&;

    /**
     * @brief Creates instruction with opcode LDTRUE. This instruction loads `true` into `acc`.
     * @return Created `Instruction`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if this `Graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    Instruction CreateLdTrue() const &&;

    /**
     * @brief Creates instruction with opcode LDFALSE. This instruction loads `false` into `acc`.
     * @return Created `Instruction`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if this `Graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    Instruction CreateLdFalse() const &&;
    /**
     * @brief Creates instruction with opcode STOWNBYINDEX. Stores `acc` to object `input0`'s property of the key
     * `imm0`.
     * @return Created `Instruction`.
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] imm0 - Index.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is false.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and current `Graph`
     * differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if this `Graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    Instruction CreateStownByIndex(const Instruction &acc, const Instruction &input0, uint64_t imm0) const &&;

    /**
     * @brief Creates instruction with opcode CREATEARRAYWITHBUFFER. This instruction creates an array using literal
     * array indexed by `literalArray`.
     * @return Created `Instruction`.
     * @param [ in ] literalArray - Literal array used in array creation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `literalArray` is false.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if this `Graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    Instruction CreateCreateArrayWithBuffer(const abckit::LiteralArray &literalArray) const &&;

    // Other dynamic API methods declarations

    /**
     * @brief Get the Import Descriptor object
     * @param inst
     * @return core::ImportDescriptor
     */
    core::ImportDescriptor GetImportDescriptor(const Instruction &inst) const;

private:
    explicit DynamicIsa(const Graph &graph) : graph_(graph) {};
    const Graph &graph_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_DYNAMIC_ISA_H
