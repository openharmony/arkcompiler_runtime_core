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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_IMPL_H
#define CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_IMPL_H

#include "./annotation_interface_field.h"

namespace abckit::core {

inline std::string_view AnnotationInterfaceField::GetName()
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *abcStr = conf->cIapi_->annotationInterfaceFieldGetName(GetView());
    CheckError(conf);
    std::string_view sView = conf->cIapi_->abckitStringToString(abcStr);
    CheckError(conf);
    return sView;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_IMPL_H
