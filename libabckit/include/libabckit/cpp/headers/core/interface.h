/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef CPP_ABCKIT_CORE_INTERFACE_H
#define CPP_ABCKIT_CORE_INTERFACE_H

#include "../base_classes.h"

namespace abckit::core {

/**
 * @brief Interface
 */
class Interface : public ViewInResource<AbckitCoreInterface *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend Module;
    /// @brief to access private constructor
    friend class Function;
    /// @brief to access private constructor
    friend class InterfaceField;
    /// @brief abckit::DefaultHash<Interface>
    friend abckit::DefaultHash<Interface>;
    /// @brief to access private constructor
    friend File;

protected:
    /// @brief Core API View type
    using CoreViewT = Interface;

public:
    /**
     * @brief Construct a new Interface object
     * @param other
     */
    Interface(const Interface &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
    Interface(Interface &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
     * @brief Returns binary file that the Interface is a part of.
     * @return Pointer to the `File`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Get Interface name
     * @return `std::string`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Returns module for this `Interface`.
     * @return Owning `core::Module`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `bool(*this)` results in `false`.
     */
    core::Module GetModule() const;

    /**
     * @brief Get vector with all Methods
     * @return std::vector<core::Function>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Function> GetAllMethods() const;

    /**
     * @brief Return vector with interface's fields.
     * @return std::vector<core::InterfaceField>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::InterfaceField> GetFields() const;

private:
    bool GetAllMethodsInner(std::vector<core::Function> &methods) const;

    bool GetFieldsInner(std::vector<core::InterfaceField> &fields) const;

    Interface(AbckitCoreInterface *iface, const ApiConfig *conf, const File *file) : ViewInResource(iface), conf_(conf)
    {
        SetResource(file);
    }

    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_INTERFACE_H
