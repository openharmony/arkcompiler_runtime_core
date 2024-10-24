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

#include "cpp/headers/core/class.h"

namespace abckit::core {

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Function> Class::GetAllMethods() const
{
    const ApiConfig *conf = GetApiConfig();
    std::vector<core::Function> methods;

    using EnumerateData = std::pair<std::vector<core::Function> *, const ApiConfig *>;
    EnumerateData enumerateData(&methods, conf);

    conf->cIapi_->classEnumerateMethods(GetView(), (void *)&enumerateData, [](AbckitCoreFunction *method, void *data) {
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

    conf->cIapi_->classEnumerateAnnotations(GetView(), (void *)&enumerateData,
                                            [](AbckitCoreAnnotation *method, void *data) {
                                                auto *vec = static_cast<EnumerateData *>(data)->first;
                                                auto *config = static_cast<EnumerateData *>(data)->second;
                                                vec->push_back(core::Annotation(method, config));
                                                return true;
                                            });

    CheckError(conf);

    return anns;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_CLASS_IMPL_H
