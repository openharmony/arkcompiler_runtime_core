/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    friend class arkts::Class;
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

    /**
     * @brief Remove field for interface
     * @return `true` on success.
     * @param [ in ] field - Field to be remove.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool RemoveField(arkts::InterfaceField field) const;

    /**
     * @brief Creates a new Interface with name `name`.
     * @return The newly created Interface object.
     * @param [ in ] m - The module in which to create the Interface.
     * @param [ in ] name - The name of the Interface to create.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     */
    static Interface CreateInterface(Module m, const std::string &name);

    /**
     * @brief add field for interface
     * @return `true` on success.
     * @param [ in ] field - Field to be add.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool AddField(arkts::InterfaceField field);

    /**
     * @brief add function for interface
     * @return `true` on success.
     * @param [ in ] function - Function to be add.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool AddMethod(arkts::Function function);

    /**
     * @brief Add superinterface to the interface
     * @return `true` on success.
     * @param [ in ] iface - Interface to be added.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if iface is false.
     */
    bool AddSuperInterface(const Interface &iface) const;

    /**
     * @brief Remove superinterface from the interface
     * @return `true` on success.
     * @param [ in ] iface - Interface to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if iface is false.
     */
    bool RemoveSuperInterface(const Interface &iface) const;

    /**
     * @brief Remove method from the interface
     * @return `true` on success.
     * @param [ in ] method - Function to be removed.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if method is false.
     */
    bool RemoveMethod(const Function &method) const;

    /**
     * @brief Sets owning module for interface
     * @return `true` on success.
     * @param [ in ] module - Module to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if module is false.
     */
    bool SetOwningModule(const Module &module) const;

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
