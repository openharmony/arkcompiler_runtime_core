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
    Instruction CreateLoadString(const std::string &str) &&;

    /**
     * @brief Creates instruction with opcode TRYLDGLOBALBYNAME. Loads the global variable of the name `string`.
     * If the global variable `string` does not exist, an exception is thrown.
     * @param str to load
     * @return `Instruction`
     */
    Instruction CreateTryldglobalbyname(const std::string &str) &&;

    /**
     * @brief Creates instruction with opcode CALLARG1. This instruction invokes the function object stored in `acc`
     * with `input0` argument
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing argument.
     * @return `Instruction`
     */
    Instruction CreateCallArg1(const Instruction &acc, const Instruction &input0) &&;

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
