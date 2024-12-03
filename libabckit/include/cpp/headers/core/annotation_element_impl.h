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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_IMPL_H
#define CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_IMPL_H

#include "./annotation_element.h"

namespace abckit::core {

inline std::string_view AnnotationElement::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *abcStr = conf->cIapi_->annotationElementGetName(GetView());
    CheckError(conf);
    std::string_view sView = conf->cIapi_->abckitStringToString(abcStr);
    CheckError(conf);
    return sView;
}

inline abckit::Value AnnotationElement::GetValue() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitValue *value = conf->cIapi_->annotationElementGetValue(GetView());
    CheckError(conf);
    return Value(value, conf, GetResource());
}

inline AnnotationElement::AnnotationElement(AbckitCoreAnnotationElement *anne, const ApiConfig *conf, const File *file)
    : ViewInResource(anne), conf_(conf)
{
    SetResource(file);
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_IMPL_H
