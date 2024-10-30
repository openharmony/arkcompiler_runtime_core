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

#ifndef CPP_ABCKIT_CORE_EXPORT_DESCRIPTOR_IMPL_H
#define CPP_ABCKIT_CORE_EXPORT_DESCRIPTOR_IMPL_H

#include "./export_descriptor.h"

namespace abckit::core {

inline std::string_view ExportDescriptor::GetName() const
{
    AbckitString *abcName = GetApiConfig()->cIapi_->exportDescriptorGetName(GetView());
    CheckError(GetApiConfig());
    std::string_view name = GetApiConfig()->cIapi_->abckitStringToString(abcName);
    CheckError(GetApiConfig());
    return name;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_EXPORT_DESCRIPTOR_IMPL_H
