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

#ifndef CPP_ABCKIT_CORE_MODULE_IMPL_H
#define CPP_ABCKIT_CORE_MODULE_IMPL_H

#include "./module.h"
#include "./class.h"
#include "./function.h"
#include "./annotation_interface.h"
#include "./namespace.h"
#include "./import_descriptor.h"
#include "./export_descriptor.h"

namespace abckit::core {

inline std::string_view Module::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->moduleGetName(GetView());
    CheckError(conf);
    std::string_view view = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return view;
}

inline std::vector<core::Class> Module::GetClasses() const
{
    std::vector<core::Class> classes;

    GetClassesInner(classes);

    CheckError(GetApiConfig());

    return classes;
}

inline std::vector<core::Function> Module::GetTopLevelFunctions() const
{
    std::vector<core::Function> functions;

    GetTopLevelFunctionsInner(functions);

    CheckError(GetApiConfig());

    return functions;
}

inline std::vector<core::AnnotationInterface> Module::GetAnnotationInterfaces() const
{
    std::vector<core::AnnotationInterface> ifaces;

    GetAnnotationInterfacesInner(ifaces);

    CheckError(GetApiConfig());

    return ifaces;
}

inline std::vector<core::Namespace> Module::GetNamespaces() const
{
    std::vector<core::Namespace> namespaces;

    GetNamespacesInner(namespaces);

    CheckError(GetApiConfig());

    return namespaces;
}

inline std::vector<core::ImportDescriptor> Module::GetImports() const
{
    std::vector<core::ImportDescriptor> imports;

    GetImportsInner(imports);

    CheckError(GetApiConfig());

    return imports;
}

inline std::vector<core::ExportDescriptor> Module::GetExports() const
{
    std::vector<core::ExportDescriptor> exports;

    GetExportsInner(exports);

    CheckError(GetApiConfig());

    return exports;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline void Module::EnumerateNamespaces(const std::function<bool(core::Namespace)> &cb) const
{
    struct Payload {
        const std::function<bool(core::Namespace)> &callback;
        const ApiConfig *config;
    };
    Payload payload {cb, GetApiConfig()};

    GetApiConfig()->cIapi_->moduleEnumerateNamespaces(GetView(), &payload, [](AbckitCoreNamespace *ns, void *data) {
        const auto &payload = *static_cast<Payload *>(data);
        return payload.callback(core::Namespace(ns, payload.config));
    });
    CheckError(GetApiConfig());
}

inline void Module::EnumerateTopLevelFunctions(const std::function<bool(core::Function)> &cb) const
{
    const ApiConfig *conf = GetApiConfig();
    using EnumerateData = std::pair<const std::function<bool(core::Function)> &, const ApiConfig *>;
    EnumerateData enumerateData(cb, GetApiConfig());

    conf->cIapi_->moduleEnumerateTopLevelFunctions(GetView(), &enumerateData, [](AbckitCoreFunction *func, void *data) {
        const std::function<bool(core::Function)> &callback = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        return callback(core::Function(func, config));
    });
    CheckError(conf);
}

inline void Module::EnumerateClasses(const std::function<bool(core::Class)> &cb) const
{
    const ApiConfig *conf = GetApiConfig();
    using EnumerateData = std::pair<const std::function<bool(core::Class)> &, const ApiConfig *>;
    EnumerateData enumerateData(cb, GetApiConfig());

    conf->cIapi_->moduleEnumerateClasses(GetView(), &enumerateData, [](AbckitCoreClass *klass, void *data) {
        const std::function<bool(core::Class)> &callback = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        return callback(core::Class(klass, config));
    });
    CheckError(conf);
}

inline void Module::EnumerateImports(const std::function<bool(core::ImportDescriptor)> &cb) const
{
    const ApiConfig *conf = GetApiConfig();
    using EnumerateData = std::pair<const std::function<bool(core::ImportDescriptor)> &, const ApiConfig *>;
    EnumerateData enumerateData(cb, GetApiConfig());

    conf->cIapi_->moduleEnumerateImports(GetView(), &enumerateData, [](AbckitCoreImportDescriptor *func, void *data) {
        const std::function<bool(core::ImportDescriptor)> &callback = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        return callback(core::ImportDescriptor(func, config));
    });
    CheckError(conf);
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_MODULE_IMPL_H
