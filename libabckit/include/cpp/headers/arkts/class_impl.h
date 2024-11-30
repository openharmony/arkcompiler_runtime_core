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

#ifndef CPP_ABCKIT_ARKTS_CLASS_IMPL_H
#define CPP_ABCKIT_ARKTS_CLASS_IMPL_H

#include "./class.h"
#include "./annotation.h"

namespace abckit::arkts {

inline AbckitArktsClass *Class::TargetCast() const
{
    auto ret = GetApiConfig()->cArktsIapi_->coreClassToArktsClass(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Class::Class(const core::Class &coreOther) : core::Class(coreOther), targetChecker_(this) {}

inline void Class::RemoveAnnotation(const arkts::Annotation &anno) const
{
    GetApiConfig()->cArktsMapi_->classRemoveAnnotation(TargetCast(), anno.TargetCast());
    CheckError(GetApiConfig());
}

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_CLASS_IMPL_H
