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

#ifndef CPP_ABCKIT_CORE_MODULE_H
#define CPP_ABCKIT_CORE_MODULE_H

#include "../base_classes.h"
#include "./class.h"
#include "./export_descriptor.h"
#include "./import_descriptor.h"
#include "./namespace.h"

namespace abckit::core {

/**
 * @brief Module
 */
class Module : public ViewInResource<AbckitCoreModule *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class abckit::core::Class;
    /// @brief to access private constructor
    friend class abckit::core::Function;
    /// @brief to access private constructor
    friend class abckit::core::ImportDescriptor;
    /// @brief abckit::DefaultHash<Module>
    friend class abckit::DefaultHash<Module>;

protected:
    /// @brief Core API View type
    using CoreViewT = Module;

public:
    /**
     * @brief Construct a new Module object
     * @param other
     */
    Module(const Module &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Module&
     */
    Module &operator=(const Module &other) = default;

    /**
     * @brief Construct a new Module object
     * @param other
     */
    Module(Module &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Module&
     */
    Module &operator=(Module &&other) = default;

    /**
     * @brief Destroy the Module object
     */
    ~Module() override = default;

    /**
     * @brief Get the Classes name
     * @return std::string_view
     */
    std::string_view GetName() const;

    /**
     * @brief Get the Classes object
     * @return std::vector<core::Class>
     */
    std::vector<core::Class> GetClasses() const;

    /**
     * @brief Get the Top Level Functions object
     * @return std::vector<core::Function>
     */
    std::vector<core::Function> GetTopLevelFunctions() const;

    /**
     * @brief Get the Annotation Interfaces object
     * @return std::vector<core::AnnotationInterface>
     */
    std::vector<core::AnnotationInterface> GetAnnotationInterfaces() const;

    /**
     * @brief Get the Namespaces object
     *
     * @return std::vector<core::Namespace>
     */
    std::vector<core::Namespace> GetNamespaces() const;

    /**
     * @brief Get the Imports object
     * @return std::vector<core::ImportDescriptor>
     */
    std::vector<core::ImportDescriptor> GetImports() const;

    /**
     * @brief Get the Exports object
     *
     * @return std::vector<core::ExportDescriptor>
     */
    std::vector<core::ExportDescriptor> GetExports() const;

    /**
     * @brief EnumerateNamespaces
     * @param cb
     */
    void EnumerateNamespaces(const std::function<bool(core::Namespace)> &cb) const;

    /**
     * @brief EnumerateTopLevelFunctions
     * @param cb
     */
    void EnumerateTopLevelFunctions(const std::function<bool(core::Function)> &cb) const;

    /**
     * @brief EnumerateClasses
     * @param cb
     */
    void EnumerateClasses(const std::function<bool(core::Class)> &cb) const;

    /**
     * @brief EnumerateImports
     * @param cb
     */
    void EnumerateImports(const std::function<bool(core::ImportDescriptor)> &cb) const;

    // Core API's.
    // ...

private:
    inline void GetClassesInner(std::vector<core::Class> &classes) const;

    inline void GetTopLevelFunctionsInner(std::vector<core::Function> &functions) const;

    inline void GetAnnotationInterfacesInner(std::vector<core::AnnotationInterface> &ifaces) const;

    inline void GetNamespacesInner(std::vector<core::Namespace> &namespaces) const;

    inline void GetImportsInner(std::vector<core::ImportDescriptor> &imports) const;

    inline void GetExportsInner(std::vector<core::ExportDescriptor> &exports) const;

    Module(AbckitCoreModule *module, const ApiConfig *conf, const File *file);

    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_MODULE_H
