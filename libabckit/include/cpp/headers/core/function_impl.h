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

#ifndef CPP_ABCKIT_CORE_FUNCTION_IMPL_H
#define CPP_ABCKIT_CORE_FUNCTION_IMPL_H

#include "./function.h"
#include "./class.h"
#include "./module.h"

namespace abckit::core {

inline Graph Function::GetGraph() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitGraph *graph = conf->cIapi_->createGraphFromFunction(GetView());
    CheckError(conf);
    return Graph(graph, conf_);
}

inline void Function::SetGraph(const Graph &graph)
{
    const ApiConfig *conf = GetApiConfig();
    conf->cMapi_->functionSetGraph(GetView(), graph.GetResource());
    CheckError(conf);
}

inline std::string_view Function::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->functionGetName(GetView());
    CheckError(conf);
    std::string_view view = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return view;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Annotation> Function::GetAnnotations() const
{
    const ApiConfig *conf = GetApiConfig();
    std::vector<core::Annotation> anns;

    using EnumerateData = std::pair<std::vector<core::Annotation> *, const ApiConfig *>;
    EnumerateData enumerateData(&anns, conf);

    conf->cIapi_->functionEnumerateAnnotations(GetView(), &enumerateData, [](AbckitCoreAnnotation *ann, void *data) {
        auto *vec = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        vec->push_back(core::Annotation(ann, config));
        return true;
    });

    CheckError(conf);

    return anns;
}

inline bool Function::IsStatic() const
{
    const ApiConfig *conf = GetApiConfig();
    bool result = conf->cIapi_->functionIsStatic(GetView());
    CheckError(conf);
    return result;
}

inline core::Module Function::GetModule() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreModule *module = conf->cIapi_->functionGetModule(GetView());
    CheckError(conf);
    return core::Module(module, conf_);
}

inline core::Class Function::GetParentClass() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitCoreClass *klass = conf->cIapi_->functionGetParentClass(GetView());
    CheckError(conf);
    return core::Class(klass, conf_);
}

// CC-OFFNXT(G.FUD.06) perf critical
inline void Function::EnumerateNestedFunctions(const std::function<bool(core::Function)> &cb) const
{
    struct Payload {
        const std::function<bool(core::Function)> &callback;
        const ApiConfig *config;
    } payload {cb, GetApiConfig()};

    GetApiConfig()->cIapi_->functionEnumerateNestedFunctions(
        GetView(), &payload, [](AbckitCoreFunction *nestedFunc, void *data) {
            const auto &payload = *static_cast<Payload *>(data);
            return payload.callback(core::Function(nestedFunc, payload.config));
        });
    CheckError(GetApiConfig());
}

// CC-OFFNXT(G.FUD.06) perf critical
inline void Function::EnumerateNestedClasses(const std::function<bool(core::Class)> &cb) const
{
    struct Payload {
        const std::function<bool(core::Class)> &callback;
        const ApiConfig *config;
    } payload {cb, GetApiConfig()};

    GetApiConfig()->cIapi_->functionEnumerateNestedClasses(
        GetView(), &payload, [](AbckitCoreClass *nestedClass, void *data) {
            const auto &payload = *static_cast<Payload *>(data);
            return payload.callback(core::Class(nestedClass, payload.config));
        });
    CheckError(GetApiConfig());
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_FUNCTION_IMPL_H
