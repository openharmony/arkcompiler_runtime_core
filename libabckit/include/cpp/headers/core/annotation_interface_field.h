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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_H
#define CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_H

#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "libabckit/include/c/metadata_core.h"

#include <string>
#include <string_view>

namespace abckit::core {

class AnnotationInterfaceField : public View<AbckitCoreAnnotationInterfaceField *> {
    friend class core::Annotation;
    friend class arkts::Annotation;
    friend class core::AnnotationInterface;
    friend class arkts::AnnotationInterface;

public:
    AnnotationInterfaceField(const AnnotationInterfaceField &other) = default;
    AnnotationInterfaceField &operator=(const AnnotationInterfaceField &other) = default;
    AnnotationInterfaceField(AnnotationInterfaceField &&other) = default;
    AnnotationInterfaceField &operator=(AnnotationInterfaceField &&other) = default;
    ~AnnotationInterfaceField() override = default;

    // Core API's.
    // ...

    std::string_view GetName();

private:
    AnnotationInterfaceField(AbckitCoreAnnotationInterfaceField *anni, const ApiConfig *conf)
        : View(anni), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_H
