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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_STUBS_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_STUBS_INL_H

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_stubs.h"

namespace ark::ets {

ALWAYS_INLINE inline bool EtsReferenceNullish(EtsCoroutine *coro, EtsObject *ref)
{
    return ref == nullptr || ref == EtsObject::FromCoreType(coro->GetUndefinedObject());
}

ALWAYS_INLINE inline bool IsRefNullish(EtsCoroutine *coro, EtsObject *ref)
{
    return ref == nullptr || ref == EtsObject::FromCoreType(coro->GetUndefinedObject());
}

ALWAYS_INLINE inline bool EtsReferenceEquals(EtsCoroutine *coro, EtsObject *ref1, EtsObject *ref2)
{
    if (UNLIKELY(ref1 == ref2)) {
        return true;
    }

    if (IsRefNullish(coro, ref1) || IsRefNullish(coro, ref2)) {
        return IsRefNullish(coro, ref1) && IsRefNullish(coro, ref2);
    }

    if (LIKELY(!(ref1->GetClass()->IsValueTyped() && ref2->GetClass()->IsValueTyped()))) {
        return false;
    }
    return EtsValueTypedEquals(coro, ref1, ref2);
}

// CC-OFFNXT(C_RULE_ID_INLINE_FUNCTION_SIZE) Perf critical common runtime code stub
inline EtsClass *GetMethodOwnerClassInFrames(EtsCoroutine *coro, uint32_t depth)
{
    auto stack = StackWalker::Create(coro);
    for (uint32_t i = 0; i < depth && stack.HasFrame(); ++i) {
        stack.NextFrame();
    }
    if (UNLIKELY(!stack.HasFrame())) {
        return nullptr;
    }
    auto method = stack.GetMethod();
    if (UNLIKELY(method == nullptr)) {
        return nullptr;
    }
    return EtsClass::FromRuntimeClass(method->GetClass());
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_STUBS_INL_H
