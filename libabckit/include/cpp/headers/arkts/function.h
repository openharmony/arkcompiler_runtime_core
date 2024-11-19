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

#ifndef CPP_ABCKIT_ARKTS_FUNCTION_H
#define CPP_ABCKIT_ARKTS_FUNCTION_H

#include "../core/function.h"

namespace abckit::arkts {

/**
 * @brief Function
 */
class Function final : public core::Function {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Class;
    /// @brief abckit::DefaultHash<Function>
    friend class abckit::DefaultHash<Function>;

public:
    /**
     * @brief Construct a new Function object
     * @param other
     */
    Function(const Function &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Function&
     */
    Function &operator=(const Function &other) = default;

    /**
     * @brief Construct a new Function object
     * @param other
     */
    Function(Function &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Function&
     */
    Function &operator=(Function &&other) = default;

    // CC-OFFNXT(G.FMT.02) project code style
    /**
     * @brief Construct a new Function object
     * @param coreOther
     */
    explicit Function(const core::Function &coreOther) : core::Function(coreOther) {};

    /**
     * @brief Destroy the Function object
     */
    ~Function() override = default;

    /**
     * @brief Add annotation
     * @param iface
     * @return arkts::Function&
     */
    arkts::Function &AddAnnotation(const arkts::AnnotationInterface &iface);

    // Other API.
    // ...
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_FUNCTION_H
