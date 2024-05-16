/**
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

#ifndef PANDA_PUBLIC_INTRINSICS_H
#define PANDA_PUBLIC_INTRINSICS_H

#include "types/ets_string.h"
#include "interop_js/intrinsics_declaration.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "intrinsics.h"

namespace ark::ets::interop::js {

[[maybe_unused]] static JSValue *JSValueFromEtsObject(EtsObject *etsObject)
{
    ASSERT(EtsCoroutine::GetCurrent()->GetPandaVM()->GetClassLinker()->GetEtsClassLinkerExtension()->IsJSValueClass(
        etsObject->GetClass()->GetRuntimeClass()));
    return reinterpret_cast<JSValue *>(etsObject);
}

static ark::ets::EtsString *JSONStringify([[maybe_unused]] ark::ets::EtsObject *obj)
{
#ifdef PANDA_ETS_INTEROP_JS
    return intrinsics::JSONStringifyIntrinsic(JSValueFromEtsObject(obj));
#else
    UNREACHABLE();
#endif
}

}  // namespace ark::ets::interop::js

#endif  // PANDA_PUBLIC_INTRINSICS_H
