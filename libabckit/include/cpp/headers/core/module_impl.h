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
    Payload<const std::function<bool(core::Namespace)> &> payload {cb, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateNamespaces(GetView(), &payload, [](AbckitCoreNamespace *ns, void *data) {
        const auto &payload = *static_cast<Payload<const std::function<bool(core::Namespace)> &> *>(data);
        return payload.data(core::Namespace(ns, payload.config, payload.resource));
    });
    CheckError(GetApiConfig());
}

inline void Module::EnumerateTopLevelFunctions(const std::function<bool(core::Function)> &cb) const
{
    Payload<const std::function<bool(core::Function)> &> payload {cb, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateTopLevelFunctions(
        GetView(), &payload, [](AbckitCoreFunction *func, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Function)> &> *>(data);
            return payload.data(core::Function(func, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
}

inline void Module::EnumerateClasses(const std::function<bool(core::Class)> &cb) const
{
    Payload<const std::function<bool(core::Class)> &> payload {cb, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateClasses(GetView(), &payload, [](AbckitCoreClass *klass, void *data) {
        const auto &payload = *static_cast<Payload<const std::function<bool(core::Class)> &> *>(data);
        return payload.data(core::Class(klass, payload.config, payload.resource));
    });
    CheckError(GetApiConfig());
}

inline void Module::EnumerateImports(const std::function<bool(core::ImportDescriptor)> &cb) const
{
    Payload<const std::function<bool(core::ImportDescriptor)> &> payload {cb, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateImports(
        GetView(), &payload, [](AbckitCoreImportDescriptor *func, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::ImportDescriptor)> &> *>(data);
            return payload.data(core::ImportDescriptor(func, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
}

inline void Module::GetClassesInner(std::vector<core::Class> &classes) const
{
    Payload<std::vector<core::Class> *> payload {&classes, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateClasses(GetView(), &payload, [](AbckitCoreClass *klass, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Class> *> *>(data);
        payload.data->push_back(core::Class(klass, payload.config, payload.resource));
        return true;
    });
}

inline void Module::GetTopLevelFunctionsInner(std::vector<core::Function> &functions) const
{
    Payload<std::vector<core::Function> *> payload {&functions, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateTopLevelFunctions(
        GetView(), &payload, [](AbckitCoreFunction *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
            payload.data->push_back(core::Function(func, payload.config, payload.resource));
            return true;
        });
}

inline void Module::GetAnnotationInterfacesInner(std::vector<core::AnnotationInterface> &ifaces) const
{
    Payload<std::vector<core::AnnotationInterface> *> payload {&ifaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateAnnotationInterfaces(
        GetView(), &payload, [](AbckitCoreAnnotationInterface *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::AnnotationInterface> *> *>(data);
            payload.data->push_back(core::AnnotationInterface(func, payload.config, payload.resource));
            return true;
        });
}

inline void Module::GetNamespacesInner(std::vector<core::Namespace> &namespaces) const
{
    Payload<std::vector<core::Namespace> *> payload {&namespaces, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateNamespaces(GetView(), &payload, [](AbckitCoreNamespace *func, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Namespace> *> *>(data);
        payload.data->push_back(core::Namespace(func, payload.config, payload.resource));
        return true;
    });
}

inline void Module::GetImportsInner(std::vector<core::ImportDescriptor> &imports) const
{
    Payload<std::vector<core::ImportDescriptor> *> payload {&imports, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateImports(
        GetView(), &payload, [](AbckitCoreImportDescriptor *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::ImportDescriptor> *> *>(data);
            payload.data->push_back(core::ImportDescriptor(func, payload.config, payload.resource));
            return true;
        });
}

inline void Module::GetExportsInner(std::vector<core::ExportDescriptor> &exports) const
{
    Payload<std::vector<core::ExportDescriptor> *> payload {&exports, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->moduleEnumerateExports(
        GetView(), &payload, [](AbckitCoreExportDescriptor *func, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::ExportDescriptor> *> *>(data);
            payload.data->push_back(core::ExportDescriptor(func, payload.config, payload.resource));
            return true;
        });
}

inline Module::Module(AbckitCoreModule *module, const ApiConfig *conf, const File *file)
    : ViewInResource(module), conf_(conf)
{
    SetResource(file);
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_MODULE_IMPL_H
