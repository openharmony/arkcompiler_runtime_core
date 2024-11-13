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

#include "./class.h"

namespace abckit::core {

inline std::string_view Class::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->classGetName(GetView());
    CheckError(conf);
    std::string_view view = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return view;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Function> Class::GetAllMethods() const
{
    const ApiConfig *conf = GetApiConfig();
    std::vector<core::Function> methods;

    using EnumerateData = std::pair<std::vector<core::Function> *, const ApiConfig *>;
    EnumerateData enumerateData(&methods, conf);

    conf->cIapi_->classEnumerateMethods(GetView(), &enumerateData, [](AbckitCoreFunction *method, void *data) {
        auto *vec = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        vec->push_back(core::Function(method, config));
        return true;
    });

    CheckError(conf);

    return methods;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Annotation> Class::GetAnnotations() const
{
    const ApiConfig *conf = GetApiConfig();
    std::vector<core::Annotation> anns;

    using EnumerateData = std::pair<std::vector<core::Annotation> *, const ApiConfig *>;
    EnumerateData enumerateData(&anns, conf);

    conf->cIapi_->classEnumerateAnnotations(GetView(), &enumerateData, [](AbckitCoreAnnotation *method, void *data) {
        auto *vec = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        vec->push_back(core::Annotation(method, config));
        return true;
    });

    CheckError(conf);

    return anns;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline void Class::EnumerateMethods(const std::function<bool(core::Function)> &cb) const
{
    const ApiConfig *conf = GetApiConfig();
    using EnumerateData = std::pair<const std::function<bool(core::Function)> &, const ApiConfig *>;
    EnumerateData enumerateData(cb, conf);

    conf->cIapi_->classEnumerateMethods(GetView(), &enumerateData, [](AbckitCoreFunction *method, void *data) {
        const std::function<bool(core::Function)> &callback = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        return callback(core::Function(method, config));
    });
    CheckError(conf);
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_CLASS_IMPL_H
