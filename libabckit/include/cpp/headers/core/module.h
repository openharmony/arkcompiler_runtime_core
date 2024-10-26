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

#include "cpp/headers/declarations.h"
#include "libabckit/include/c/abckit.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "cpp/headers/core/class.h"
#include "cpp/headers/core/export_descriptor.h"
#include "cpp/headers/core/import_descriptor.h"
#include "cpp/headers/core/namespace.h"
#include "libabckit/include/c/metadata_core.h"

#include <string_view>
#include <vector>

namespace abckit::core {

class Module : public View<AbckitCoreModule *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class abckit::File;

public:
    Module(const Module &other) = default;
    Module &operator=(const Module &other) = default;
    Module(Module &&other) = default;
    Module &operator=(Module &&other) = default;
    ~Module() override = default;

    std::vector<core::Class> GetClasses() const;
    std::vector<core::Function> GetTopLevelFunctions() const;
    std::vector<core::AnnotationInterface> GetAnnotationInterfaces() const;
    std::vector<core::Namespace> GetNamespaces() const;
    std::vector<core::ImportDescriptor> GetImports() const;
    std::vector<core::ExportDescriptor> GetExports() const;

    // Core API's.
    // ...

private:
    inline void GetClassesInner(std::vector<core::Class> &classes) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::Class> *, const ApiConfig *>;
        EnumerateData enumerateData(&classes, GetApiConfig());

        conf->cIapi_->moduleEnumerateClasses(GetView(), (void *)&enumerateData, [](AbckitCoreClass *klass, void *data) {
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

        conf->cIapi_->moduleEnumerateTopLevelFunctions(GetView(), (void *)&enumerateData,
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

        conf->cIapi_->moduleEnumerateAnnotationInterfaces(GetView(), (void *)&enumerateData,
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

        conf->cIapi_->moduleEnumerateNamespaces(GetView(), (void *)&enumerateData,
                                                [](AbckitCoreNamespace *func, void *data) {
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

        conf->cIapi_->moduleEnumerateImports(GetView(), (void *)&enumerateData,
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

        conf->cIapi_->moduleEnumerateExports(GetView(), (void *)&enumerateData,
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
