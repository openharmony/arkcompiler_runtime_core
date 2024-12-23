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

#ifndef CPP_ABCKIT_ARKTS_FUNCTION_IMPL_H
#define CPP_ABCKIT_ARKTS_FUNCTION_IMPL_H

#include "./function.h"
#include "./annotation.h"
#include "../core/annotation.h"
#include "./annotation_interface.h"

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit::arkts {

inline AbckitArktsFunction *Function::TargetCast() const
{
    auto ret = GetApiConfig()->cArktsIapi_->coreFunctionToArktsFunction(GetView());
    CheckError(GetApiConfig());
    return ret;
}

inline Function::Function(const core::Function &coreOther) : core::Function(coreOther), targetChecker_(this) {};

inline bool Function::IsNative() const
{
    auto arktsFunc = GetApiConfig()->cArktsIapi_->coreFunctionToArktsFunction(GetView());
    CheckError(GetApiConfig());
    auto arktsIsNative = GetApiConfig()->cArktsIapi_->functionIsNative(arktsFunc);
    CheckError(GetApiConfig());
    return arktsIsNative;
}

inline Annotation Function::AddAnnotation(AnnotationInterface ai) const
{
    const struct AbckitArktsAnnotationCreateParams params {
        ai.TargetCast()
    };
    auto arktsAnno = GetApiConfig()->cArktsMapi_->functionAddAnnotation(TargetCast(), &params);
    CheckError(GetApiConfig());
    auto coreAnno = GetApiConfig()->cArktsIapi_->arktsAnnotationToCoreAnnotation(arktsAnno);
    CheckError(GetApiConfig());
    return Annotation(core::Annotation(coreAnno, GetApiConfig(), GetResource()));
}

inline Function Function::RemoveAnnotation(Annotation anno) const
{
    GetApiConfig()->cArktsMapi_->functionRemoveAnnotation(TargetCast(), anno.TargetCast());
    CheckError(GetApiConfig());
    return *this;
}

}  // namespace abckit::arkts
// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_ARKTS_FUNCTION_IMPL_H
