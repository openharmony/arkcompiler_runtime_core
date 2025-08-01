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

#ifndef CPP_ABCKIT_CORE_CLASS_IMPL_H
#define CPP_ABCKIT_CORE_CLASS_IMPL_H

#include "class.h"
#include "annotation.h"
#include "function.h"
#include "module.h"

namespace abckit::core {

inline const File *Class::GetFile() const
{
    return GetResource();
}

inline std::string Class::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->classGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Function> Class::GetAllMethods() const
{
    std::vector<core::Function> methods;
    Payload<std::vector<core::Function> *> payload {&methods, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->classEnumerateMethods(GetView(), &payload, [](AbckitCoreFunction *method, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
        payload.data->push_back(core::Function(method, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return methods;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Annotation> Class::GetAnnotations() const
{
    std::vector<core::Annotation> anns;

    Payload<std::vector<core::Annotation> *> payload {&anns, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->classEnumerateAnnotations(
        GetView(), &payload, [](AbckitCoreAnnotation *method, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Annotation> *> *>(data);
            payload.data->push_back(core::Annotation(method, payload.config, payload.resource));
            return true;
        });

    CheckError(GetApiConfig());

    return anns;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Class::EnumerateMethods(const std::function<bool(core::Function)> &cb) const
{
    Payload<const std::function<bool(core::Function)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->classEnumerateMethods(
        GetView(), &payload, [](AbckitCoreFunction *method, void *data) -> bool {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Function)> &> *>(data);
            return payload.data(core::Function(method, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

inline bool Class::GetAllMethodsInner(std::vector<core::Function> &methods) const
{
    Payload<std::vector<core::Function> *> payload {&methods, GetApiConfig(), GetResource()};

    auto isNormalExit =
        GetApiConfig()->cIapi_->classEnumerateMethods(GetView(), &payload, [](AbckitCoreFunction *method, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
            payload.data->push_back(core::Function(method, payload.config, payload.resource));
            return true;
        });
    return isNormalExit;
}

inline bool Class::GetAllAnnotationsInner(std::vector<core::Annotation> &anns) const
{
    Payload<std::vector<core::Annotation> *> payload {&anns, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cIapi_->classEnumerateAnnotations(
        GetView(), &payload, [](AbckitCoreAnnotation *method, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Annotation> *> *>(data);
            payload.data->push_back(core::Annotation(method, payload.config, payload.resource));
            return true;
        });
    return isNormalExit;
}

inline Class::Class(AbckitCoreClass *klass, const ApiConfig *conf, const File *file)
    : ViewInResource(klass), conf_(conf)
{
    SetResource(file);
};

inline bool Class::EnumerateAnnotations(const std::function<bool(core::Annotation)> &cb) const
{
    Payload<const std::function<bool(core::Annotation)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->classEnumerateAnnotations(
        GetView(), &payload, [](AbckitCoreAnnotation *method, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Annotation)> &> *>(data);
            return payload.data(core::Annotation(method, payload.config, payload.resource));
        });

    CheckError(GetApiConfig());
    return isNormalExit;
}

inline Module Class::GetModule() const
{
    AbckitCoreModule *mod = GetApiConfig()->cIapi_->classGetModule(GetView());
    CheckError(GetApiConfig());
    return Module(mod, GetApiConfig(), GetResource());
}

inline Function Class::GetParentFunction() const
{
    AbckitCoreFunction *func = GetApiConfig()->cIapi_->classGetParentFunction(GetView());
    CheckError(GetApiConfig());
    return Function(func, GetApiConfig(), GetResource());
}

inline Namespace Class::GetParentNamespace() const
{
    AbckitCoreNamespace *ns = GetApiConfig()->cIapi_->classGetParentNamespace(GetView());
    CheckError(GetApiConfig());
    return Namespace(ns, GetApiConfig(), GetResource());
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_CLASS_IMPL_H
