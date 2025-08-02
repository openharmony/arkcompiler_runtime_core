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

#ifndef CPP_ABCKIT_ARKTS_INTERFACE_H
#define CPP_ABCKIT_ARKTS_INTERFACE_H

#include "../core/interface.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief Interface
 */
class Interface final : public core::Interface {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Module;
    /// @brief to access private constructor
    friend class Namespace;
    /// @brief abckit::DefaultHash<Interface>
    friend class abckit::DefaultHash<Interface>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<Interface>;

public:
    /**
     * @brief Constructor Arkts API Interface from the Core API with compatibility check
     * @param coreOther - Core API Interface
     */
    explicit Interface(const core::Interface &coreOther);

    /**
     * @brief Construct a new interface object
     * @param other
     */
    Interface(const Interface &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Interface&
     */
    Interface &operator=(const Interface &other) = default;

    /**
     * @brief Construct a new Interface object
     * @param other
     */
    Interface(Interface &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Interface&
     */
    Interface &operator=(Interface &&other) = default;

    /**
     * @brief Destroy the Interface object
     */
    ~Interface() override = default;

    /**
     * @brief Sets name for interface
     * @return `true` on success.
     * @param [ in ] name - Name to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool SetName(const std::string &name) const;

private:
    /**
     * @brief Converts interface from Core to Arkts target
     * @return AbckitArktsInterface* - converted interface
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsInterface *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<Interface> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_INTERFACE_H
