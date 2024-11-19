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
class Module : public View<AbckitCoreModule *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class abckit::core::Function;
    /// @brief to access private constructor
    friend class abckit::core::ImportDescriptor;
    /// @brief abckit::DefaultHash<Module>
    friend class abckit::DefaultHash<Module>;

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
    inline void GetClassesInner(std::vector<core::Class> &classes) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::Class> *, const ApiConfig *>;
        EnumerateData enumerateData(&classes, GetApiConfig());

        conf->cIapi_->moduleEnumerateClasses(GetView(), &enumerateData, [](AbckitCoreClass *klass, void *data) {
            auto *vec = static_cast<EnumerateData *>(data)->first;
            auto *config = static_cast<EnumerateData *>(data)->second;
            vec->push_back(core::Class(klass, config));
            return true;
        });
    }

    inline void GetTopLevelFunctionsInner(std::vector<core::Function> &functions) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::Function> *, const ApiConfig *>;
        EnumerateData enumerateData(&functions, conf);

        conf->cIapi_->moduleEnumerateTopLevelFunctions(GetView(), &enumerateData,
                                                       [](AbckitCoreFunction *func, void *data) {
                                                           auto *vec = static_cast<EnumerateData *>(data)->first;
                                                           auto *config = static_cast<EnumerateData *>(data)->second;
                                                           vec->push_back(core::Function(func, config));
                                                           return true;
                                                       });
    }

    inline void GetAnnotationInterfacesInner(std::vector<core::AnnotationInterface> &ifaces) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::AnnotationInterface> *, const ApiConfig *>;
        EnumerateData enumerateData(&ifaces, conf);

        conf->cIapi_->moduleEnumerateAnnotationInterfaces(GetView(), &enumerateData,
                                                          [](AbckitCoreAnnotationInterface *func, void *data) {
                                                              auto *vec = static_cast<EnumerateData *>(data)->first;
                                                              auto *config = static_cast<EnumerateData *>(data)->second;
                                                              vec->push_back(core::AnnotationInterface(func, config));
                                                              return true;
                                                          });
    }

    inline void GetNamespacesInner(std::vector<core::Namespace> &namespaces) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::Namespace> *, const ApiConfig *>;
        EnumerateData enumerateData(&namespaces, conf);

        conf->cIapi_->moduleEnumerateNamespaces(GetView(), &enumerateData, [](AbckitCoreNamespace *func, void *data) {
            auto *vec = static_cast<EnumerateData *>(data)->first;
            auto *config = static_cast<EnumerateData *>(data)->second;
            vec->push_back(core::Namespace(func, config));
            return true;
        });
    }

    inline void GetImportsInner(std::vector<core::ImportDescriptor> &imports) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::ImportDescriptor> *, const ApiConfig *>;
        EnumerateData enumerateData(&imports, conf);

        conf->cIapi_->moduleEnumerateImports(GetView(), &enumerateData,
                                             [](AbckitCoreImportDescriptor *func, void *data) {
                                                 auto *vec = static_cast<EnumerateData *>(data)->first;
                                                 auto *config = static_cast<EnumerateData *>(data)->second;
                                                 vec->push_back(core::ImportDescriptor(func, config));
                                                 return true;
                                             });
    }
    inline void GetExportsInner(std::vector<core::ExportDescriptor> &exports) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::ExportDescriptor> *, const ApiConfig *>;
        EnumerateData enumerateData(&exports, conf);

        conf->cIapi_->moduleEnumerateExports(GetView(), &enumerateData,
                                             [](AbckitCoreExportDescriptor *func, void *data) {
                                                 auto *vec = static_cast<EnumerateData *>(data)->first;
                                                 auto *config = static_cast<EnumerateData *>(data)->second;
                                                 vec->push_back(core::ExportDescriptor(func, config));
                                                 return true;
                                             });
    }

    Module(AbckitCoreModule *module, const ApiConfig *conf) : View(module), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_MODULE_H
