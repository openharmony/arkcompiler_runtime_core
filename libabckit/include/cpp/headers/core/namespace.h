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

#ifndef CPP_ABCKIT_CORE_NAMESPACE_H
#define CPP_ABCKIT_CORE_NAMESPACE_H

#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "cpp/headers/core/annotation_interface.h"
#include "cpp/headers/core/function.h"

#include <vector>
#include <utility>

namespace abckit::core {

class Namespace : public View<AbckitCoreNamespace *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class Module;

public:
    Namespace(const Namespace &other) = default;
    Namespace &operator=(const Namespace &other) = default;
    Namespace(Namespace &&other) = default;
    Namespace &operator=(Namespace &&other) = default;
    ~Namespace() override = default;

    std::string_view GetName() const
    {
        AbckitString *abcName = GetApiConfig()->cIapi_->namespaceGetName(GetView());
        CheckError(GetApiConfig());
        std::string_view name = GetApiConfig()->cIapi_->abckitStringToString(abcName);
        CheckError(GetApiConfig());
        return name;
    }

    // Core API's.
    // ...

private:
    Namespace(AbckitCoreNamespace *ns, const ApiConfig *conf) : View(ns), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_NAMESPACE_H
