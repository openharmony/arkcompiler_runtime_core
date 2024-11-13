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

#ifndef CPP_ABCKIT_CORE_CLASS_H
#define CPP_ABCKIT_CORE_CLASS_H

#include "../base_classes.h"
#include "./function.h"

#include <functional>
#include <vector>

namespace abckit::core {

class Class : public View<AbckitCoreClass *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class Module;
    friend class Namespace;
    friend class Function;

public:
    Class(const Class &other) = default;
    Class &operator=(const Class &other) = default;
    Class(Class &&other) = default;
    Class &operator=(Class &&other) = default;
    ~Class() override = default;

    std::string_view GetName() const;
    std::vector<core::Function> GetAllMethods() const;
    std::vector<core::Annotation> GetAnnotations() const;
    void EnumerateMethods(const std::function<bool(core::Function)> &cb) const;

    // Core API's.
    // ...

private:
    inline void GetAllMethodsInner(std::vector<core::Function> &methods) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::Function> *, const ApiConfig *>;
        EnumerateData enumerateData(&methods, conf);

        conf->cIapi_->classEnumerateMethods(GetView(), &enumerateData, [](AbckitCoreFunction *method, void *data) {
            auto *vec = static_cast<EnumerateData *>(data)->first;
            auto *config = static_cast<EnumerateData *>(data)->second;
            vec->push_back(core::Function(method, config));
            return true;
        });
    }

    inline void GetAllAnnotationsInner(std::vector<core::Annotation> &anns) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::Annotation> *, const ApiConfig *>;
        EnumerateData enumerateData(&anns, conf);

        conf->cIapi_->classEnumerateAnnotations(GetView(), &enumerateData,
                                                [](AbckitCoreAnnotation *method, void *data) {
                                                    auto *vec = static_cast<EnumerateData *>(data)->first;
                                                    auto *config = static_cast<EnumerateData *>(data)->second;
                                                    vec->push_back(core::Annotation(method, config));
                                                    return true;
                                                });
    }

    Class(AbckitCoreClass *klass, const ApiConfig *conf) : View(klass), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_CLASS_H
