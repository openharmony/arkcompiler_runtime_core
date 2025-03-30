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

#ifndef CPP_ABCKIT_CORE_CLASS_H
#define CPP_ABCKIT_CORE_CLASS_H

#include "../base_classes.h"
#include "./function.h"

#include <functional>
#include <vector>

namespace abckit::core {

/**
 * @brief Class
 */
class Class : public ViewInResource<AbckitCoreClass *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend Module;
    /// @brief to access private constructor
    friend Namespace;
    /// @brief to access private constructor
    friend class Function;
    /// @brief to access private constructor
    friend class abckit::Type;
    /// @brief abckit::DefaultHash<Class>
    friend abckit::DefaultHash<Class>;
    /// @brief to access private constructor
    friend File;

protected:
    /// @brief Core API View type
    using CoreViewT = Class;

public:
    /**
     * @brief Construct a new Class object
     * @param other
     */
    Class(const Class &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    Class &operator=(const Class &other) = default;

    /**
     * @brief Construct a new Class object
     * @param other
     */
    Class(Class &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    Class &operator=(Class &&other) = default;

    /**
     * @brief Destroy the Class object
     */
    ~Class() override = default;

    /**
     * @brief Returns binary file that the Class is a part of.
     * @return Pointer to the `File`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Get Class name
     * @return `std::string`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Returns module for this `Class`.
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
     * @brief Get vector with all Annotations
     * @return std::vector<core::Annotation>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Annotation> GetAnnotations() const;

    /**
     * @brief Enumerates methods of Class, invoking callback `cb` for each method.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool EnumerateMethods(const std::function<bool(core::Function)> &cb) const;

    /**
     * @brief Enumerates annotations of Class, invoking callback `cb` for each annotation.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @return false` if was early exited. Otherwise - `true`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is false.
     */
    bool EnumerateAnnotations(const std::function<bool(core::Annotation)> &cb) const;

    /**
     * @brief Returns parent function for class.
     * @return `core::Function`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Function GetParentFunction() const;

    /**
     * @brief Returns parent namespace for class.
     * @return `core::Namespace`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Namespace GetParentNamespace() const;

private:
    inline bool GetAllMethodsInner(std::vector<core::Function> &methods) const;

    inline bool GetAllAnnotationsInner(std::vector<core::Annotation> &anns) const;

    Class(AbckitCoreClass *klass, const ApiConfig *conf, const File *file);

    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_CLASS_H
