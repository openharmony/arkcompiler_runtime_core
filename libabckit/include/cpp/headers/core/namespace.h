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

#ifndef CPP_ABCKIT_CORE_NAMESPACE_H
#define CPP_ABCKIT_CORE_NAMESPACE_H

#include "../base_classes.h"

#include <functional>
#include <string_view>

namespace abckit::core {

/**
 * @brief Namespace
 */
class Namespace : public ViewInResource<AbckitCoreNamespace *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class core::Module;
    /// @brief to access private constructor
    friend class core::Class;
    /// @brief to access private constructor
    friend class core::Function;
    /// @brief abckit::DefaultHash<Namespace>
    friend class abckit::DefaultHash<Namespace>;

protected:
    /// @brief Core API View type
    using CoreViewT = Namespace;

public:
    /**
     * @brief Construct a new Namespace object
     * @param other
     */
    Namespace(const Namespace &other) = default;

    /**
     * @brief
     * @param other
     * @return Namespace&
     */
    Namespace &operator=(const Namespace &other) = default;

    /**
     * @brief Construct a new Namespace object
     * @param other
     */
    Namespace(Namespace &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief
     * @param other
     * @return Namespace&
     */
    Namespace &operator=(Namespace &&other) = default;

    /**
     * @brief Destroy the Namespace object
     */
    ~Namespace() override = default;

    /**
     * @brief Get the Name object
     * @return `std::string`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const
    {
        AbckitString *abcName = GetApiConfig()->cIapi_->namespaceGetName(GetView());
        CheckError(GetApiConfig());
        std::string name = GetApiConfig()->cIapi_->abckitStringToString(abcName);
        CheckError(GetApiConfig());
        return name;
    }

    /**
     * @brief Returns parent namespace.
     * @return `core::Namespace` or NULL if namespace has no parent namespace.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Namespace GetParentNamespace() const
    {
        AbckitCoreNamespace *parent = GetApiConfig()->cIapi_->namespaceGetParentNamespace(GetView());
        return Namespace(parent, GetApiConfig(), GetResource());
    }

    /**
     * @brief Enumerates namespaces defined inside of the Namespace, invoking callback `cb` for each inner
     * namespace.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateNamespaces(const std::function<bool(core::Namespace)> &cb) const;

    /**
     * @brief Enumerates classes of the Namespace, invoking callback `cb` for each class.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateClasses(const std::function<bool(core::Class)> &cb) const;

    /**
     * @brief Enumerates top level functions of the Namespace, invoking callback `cb` for each top level function.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateTopLevelFunctions(const std::function<bool(core::Function)> &cb) const;

private:
    Namespace(AbckitCoreNamespace *ns, const ApiConfig *conf, const File *file) : ViewInResource(ns), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_NAMESPACE_H
