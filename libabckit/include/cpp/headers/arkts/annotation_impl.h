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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_IMPL_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_IMPL_H

#include "./annotation.h"
#include "./annotation_element.h"
#include "../value.h"

namespace abckit::arkts {

inline AbckitArktsAnnotation *Annotation::TargetCast() const
{
    auto ret = GetApiConfig()->cArktsIapi_->coreAnnotationToArktsAnnotation(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Annotation::Annotation(const core::Annotation &coreOther) : core::Annotation(coreOther), targetChecker_(this) {}

inline arkts::Annotation &Annotation::AddElement(const abckit::Value &val, const std::string &name)
{
    struct AbckitArktsAnnotationElementCreateParams params {
        name.c_str(), val.GetView()
    };
    GetApiConfig()->cArktsMapi_->annotationAddAnnotationElement(TargetCast(), &params);
    CheckError(GetApiConfig());
    return *this;
}

inline AbckitCoreAnnotationElement *Annotation::AddAndGetElementImpl(AbckitArktsAnnotationElementCreateParams *params)
{
    AbckitArktsAnnotationElement *arktsAnni =
        GetApiConfig()->cArktsMapi_->annotationAddAnnotationElement(TargetCast(), params);
    CheckError(GetApiConfig());
    AbckitCoreAnnotationElement *coreAnni =
        GetApiConfig()->cArktsIapi_->arktsAnnotationElementToCoreAnnotationElement(arktsAnni);
    CheckError(GetApiConfig());
    return coreAnni;
}

inline arkts::AnnotationElement Annotation::AddAndGetElement(const abckit::Value &val, const std::string_view name)
{
    struct AbckitArktsAnnotationElementCreateParams params {
        name.data(), val.GetView()
    };
    auto *coreAnni = AddAndGetElementImpl(&params);
    core::AnnotationElement element(coreAnni, GetApiConfig(), GetResource());
    return arkts::AnnotationElement(element);
}

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_IMPL_H
