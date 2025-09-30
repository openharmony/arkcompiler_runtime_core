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

#ifndef CPP_ABCKIT_ARKTS_ENUM_IMPL_H
#define CPP_ABCKIT_ARKTS_ENUM_IMPL_H

#include "enum.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit::arkts {

inline AbckitArktsEnum *Enum::TargetCast() const
{
    const auto ret = GetApiConfig()->cArktsIapi_->coreEnumToArktsEnum(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Enum::Enum(const core::Enum &coreOther) : core::Enum(coreOther), targetChecker_(this) {}

inline bool Enum::SetName(const std::string &name) const
{
    const auto ret = GetApiConfig()->cArktsMapi_->enumSetName(TargetCast(), name.c_str());
    CheckError(GetApiConfig());
    return ret;
}

}  // namespace abckit::arkts
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_ARKTS_ENUM_IMPL_H
