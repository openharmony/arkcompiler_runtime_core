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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_H
#define CPP_ABCKIT_CORE_ANNOTATION_H

#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/core/annotation_interface.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"

namespace abckit::core {

class Annotation : public View<AbckitCoreAnnotation *> {
    friend class core::Function;
    friend class arkts::Function;
    friend class core::Class;
    friend class arkts::Class;

public:
    Annotation(const Annotation &other) = default;
    Annotation &operator=(const Annotation &other) = default;
    Annotation(Annotation &&other) = default;
    Annotation &operator=(Annotation &&other) = default;
    ~Annotation() override = default;

    // Core API's.
    // ...

    core::AnnotationInterface GetInterface()
    {
        AnnotationInterface iface(GetApiConfig()->cIapi_->annotationGetInterface(GetView()), GetApiConfig());
        CheckError(GetApiConfig());
        return iface;
    }

private:
    Annotation(AbckitCoreAnnotation *ann, const ApiConfig *conf) : View(ann), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_H
