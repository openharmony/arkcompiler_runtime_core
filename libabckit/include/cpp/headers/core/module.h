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
#include "./namespace.h"
#include "./annotation_interface.h"

namespace abckit::core {

/**
 * @brief Module
 */
class Module : public ViewInResource<AbckitCoreModule *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class core::Class;
    /// @brief to access private constructor
    friend class core::ImportDescriptor;
    /// @brief to access private constructor
    friend class core::ExportDescriptor;
    /// @brief to access private constructor
    friend class core::AnnotationInterface;
    /// @brief to access private constructor
    friend class core::Class;
    /// @brief to access private constructor
    friend class core::Function;
    /// @brief abckit::DefaultHash<Module>
    friend class abckit::DefaultHash<Module>;
    /// @brief abckit::DynamicIsa
    friend class abckit::DynamicIsa;
    /// @brief arkts::Module
    friend class arkts::Module;

protected:
    /// @brief Core API View type
    using CoreViewT = Module;

public:
    /**
     * @brief Construct a new Module object
     * @param other
     */
    Module(const Module &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

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
    Module(Module &&other) = default;  // CC-OFF(G.CLS.07-CPP)

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
     * @brief Returns binary file that the Module is a part of.
     * @return `File` that contains Module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Returns the target that the Module was compiled for.
     * @return Target of the current module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    enum AbckitTarget GetTarget() const;

    /**
     * @brief Get the Classes name
     * @return std::string
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Tells if Module is defined in the same binary or externally in another binary.
     * @return Returns `true` if Module is defined in another binary and `false` if defined locally.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsExternal() const;

    /**
     * @brief Get the Classes object
     * @return std::vector<core::Class>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Class> GetClasses() const;

    /**
     * @brief Get the Top Level Functions object
     * @return std::vector<core::Function>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Function> GetTopLevelFunctions() const;

    /**
     * @brief Get the Annotation Interfaces object
     * @return std::vector<core::AnnotationInterface>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::AnnotationInterface> GetAnnotationInterfaces() const;

    /**
     * @brief Get the Namespaces object
     * @return std::vector<core::Namespace>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Namespace> GetNamespaces() const;

    /**
     * @brief Get the Imports object
     * @return std::vector<core::ImportDescriptor>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::ImportDescriptor> GetImports() const;

    /**
     * @brief Get the Exports object
     * @return std::vector<core::ExportDescriptor>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::ExportDescriptor> GetExports() const;

    /**
     * @brief Enumerates namespaces of the Module, invoking callback `cb` for each namespace.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateNamespaces(const std::function<bool(core::Namespace)> &cb) const;

    /**
     * @brief Enumerates top level functions of the Module, invoking callback `cb` for each top level function.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateTopLevelFunctions(const std::function<bool(core::Function)> &cb) const;

    /**
     * @brief Enumerates classes of the Module, invoking callback `cb` for each class.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateClasses(const std::function<bool(core::Class)> &cb) const;

    /**
     * @brief Enumerates imports of the Module, invoking callback `cb` for each import.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateImports(const std::function<bool(core::ImportDescriptor)> &cb) const;

    /**
     * @brief Enumerates anonymous functions of the Module, invoking callback `cb` for each anonymous function.
     * @return`false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateAnonymousFunctions(const std::function<bool(core::Function)> &cb) const;

    /**
     * @brief Enumerates exports of the Module, invoking callback `cb` for each export.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateExports(const std::function<bool(core::ExportDescriptor)> &cb) const;

    /**
     * @brief Enumerates annotation interfaces of the Module, invoking callback `cb` for each annotation interface.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'cb' is false.
     */
    bool EnumerateAnnotationInterfaces(const std::function<bool(core::AnnotationInterface)> &cb) const;

private:
    bool GetClassesInner(std::vector<core::Class> &classes) const;

    bool GetTopLevelFunctionsInner(std::vector<core::Function> &functions) const;

    bool GetAnnotationInterfacesInner(std::vector<core::AnnotationInterface> &ifaces) const;

    bool GetNamespacesInner(std::vector<core::Namespace> &namespaces) const;

    bool GetImportsInner(std::vector<core::ImportDescriptor> &imports) const;

    bool GetExportsInner(std::vector<core::ExportDescriptor> &exports) const;

    Module(AbckitCoreModule *module, const ApiConfig *conf, const File *file) : ViewInResource(module), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_MODULE_H
