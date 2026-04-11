/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_stubs.h"
#include "runtime/include/managed_thread.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "libarkfile/proto_data_accessor-inl.h"

namespace ark::ets {

static constexpr const char *GETTER_PREFIX = "%%get-";
static constexpr const char *SETTER_PREFIX = "%%set-";

ALWAYS_INLINE inline bool EtsReferenceNullish(EtsExecutionContext *executionCtx, EtsObject *ref)
{
    ASSERT(executionCtx != nullptr);
    return ref == nullptr || ref == EtsObject::FromCoreType(executionCtx->GetNullValue());
}

ALWAYS_INLINE inline bool IsValueNaN(EtsObject *ref)
{
    if (UNLIKELY(ref == nullptr || !ref->GetClass()->IsBoxed())) {
        return false;
    }
    if (ref->GetClass()->IsBoxedDouble()) {
        return std::isnan(static_cast<double>(EtsBoxPrimitive<EtsDouble>::Unbox(ref)));
    }
    if (ref->GetClass()->IsBoxedFloat()) {
        return std::isnan(static_cast<float>(EtsBoxPrimitive<EtsFloat>::Unbox(ref)));
    }
    return false;
}

template <bool IS_STRICT>
ALWAYS_INLINE inline bool EtsReferenceEquals(EtsExecutionContext *executionCtx, EtsObject *ref1, EtsObject *ref2)
{
    if (UNLIKELY(ref1 == ref2)) {
        return !IsValueNaN(ref1);
    }

    if (EtsReferenceNullish(executionCtx, ref1) || EtsReferenceNullish(executionCtx, ref2)) {
        if constexpr (IS_STRICT) {
            return false;
        } else {
            return EtsReferenceNullish(executionCtx, ref1) && EtsReferenceNullish(executionCtx, ref2);
        }
    }

    ASSERT(ref1 != nullptr);
    ASSERT(ref2 != nullptr);
    if (LIKELY(!(ref1->GetClass()->IsValueTyped() && ref2->GetClass()->IsValueTyped()))) {
        return false;
    }
    return EtsValueTypedEquals(executionCtx, ref1, ref2);
}

ALWAYS_INLINE inline EtsString *EtsReferenceTypeof(EtsExecutionContext *executionCtx, EtsObject *ref)
{
    return EtsGetTypeof(executionCtx, ref);
}

ALWAYS_INLINE inline bool EtsIstrue(EtsExecutionContext *executionCtx, EtsObject *obj)
{
    return EtsGetIstrue(executionCtx, obj);
}

// CC-OFFNXT(C_RULE_ID_INLINE_FUNCTION_SIZE) Perf critical common runtime code stub
inline EtsClass *GetMethodOwnerClassInFrames(EtsExecutionContext *executionCtx, uint32_t depth)
{
    auto stack = StackWalker::Create(executionCtx->GetMT());
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
