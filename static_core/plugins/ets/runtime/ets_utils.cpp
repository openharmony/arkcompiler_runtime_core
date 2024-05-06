/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets {

bool IsEtsGlobalClassName(const std::string &descriptor)
{
    ASSERT(descriptor.length() >= 2U);  // L...;
    ASSERT('L' == descriptor[0]);
    ASSERT(';' == descriptor[descriptor.size() - 1]);

    constexpr size_t ETSGLOBAL_SEMICOLON_LENGTH = sizeof(ETSGLOBAL_CLASS_NAME);

    const auto etsGlobalSubstringPos = descriptor.rfind(ETSGLOBAL_CLASS_NAME);
    const bool etsGlobalClass = (1 == etsGlobalSubstringPos) && descriptor.length() == 1 + ETSGLOBAL_SEMICOLON_LENGTH;
    const bool endsWithETSGLOBAL = descriptor.length() - ETSGLOBAL_SEMICOLON_LENGTH == etsGlobalSubstringPos;
    const bool etsGlobalClassInPackage = endsWithETSGLOBAL && '/' == descriptor[etsGlobalSubstringPos - 1];

    return etsGlobalClass || etsGlobalClassInPackage;
}

void LambdaUtils::InvokeVoid(EtsCoroutine *coro, EtsObject *lambda)
{
    EtsMethod *invoke = lambda->GetClass()->GetMethod("invoke");
    if (invoke == nullptr) {
        LOG(FATAL, RUNTIME) << "No method 'invoke' found";
        return;
    }
    Value arg(lambda->GetCoreType());
    invoke->GetPandaMethod()->InvokeVoid(coro, &arg);
}
}  // namespace ark::ets
