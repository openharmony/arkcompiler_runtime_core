/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_reflect_method.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"

namespace ark::ets {

EtsReflectMethod *EtsReflectMethod::Create(EtsCoroutine *etsCoroutine, bool isStatic, bool isConstructor)
{
    EtsClass *klass = isConstructor ? PlatformTypes(etsCoroutine)->reflectConstructor
                                    : (isStatic ? PlatformTypes(etsCoroutine)->reflectStaticMethod
                                                : PlatformTypes(etsCoroutine)->reflectInstanceMethod);
    EtsObject *etsObject = EtsObject::Create(etsCoroutine, klass);
    return EtsReflectMethod::FromEtsObject(etsObject);
}

}  // namespace ark::ets
