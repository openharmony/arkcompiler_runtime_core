/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_escompat_array.h"

#include "runtime/include/managed_thread.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets {

/* static */
EtsEscompatArray *EtsEscompatArray::Create(EtsExecutionContext *executionCtx, size_t length)
{
    ASSERT(!executionCtx->GetMT()->HasPendingException());

    EtsHandleScope scope(executionCtx);

    const EtsPlatformTypes *platformTypes = PlatformTypes(executionCtx);

    // Create std.core array
    EtsClass *klass = platformTypes->escompatArray;
    EtsHandle<EtsEscompatArray> arrayHandle(executionCtx, FromEtsObject(EtsObject::Create(executionCtx, klass)));
    if (UNLIKELY(arrayHandle.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }

    // Create fixed array, field of escompat
    EtsClass *cls = platformTypes->coreObject;
    auto buffer = EtsObjectArray::Create(cls, length);
    if (UNLIKELY(buffer == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }
    ObjectAccessor::SetObject(executionCtx->GetMT(), arrayHandle.GetPtr(), GetBufferOffset(), buffer->GetCoreType());

    // Set length
    arrayHandle->actualLength_ = length;

    return arrayHandle.GetPtr();
}

static bool IsOutOfBounds(EtsInt index, EtsInt actualLength)
{
    if (UNLIKELY(index < 0 || index >= actualLength)) {
        ThrowEtsException(EtsExecutionContext::GetCurrent(), PlatformTypes()->coreRangeError, "Out of bounds");
        return true;
    }
    return false;
}

EtsObject *EtsEscompatArray::EscompatArrayGet(EtsInt index)
{
    // Can use private fields as part of `std.core.Array` method implementation
    if (UNLIKELY(IsOutOfBounds(index, GetActualLengthFromEscompatArrayImpl()))) {
        return nullptr;
    }
    return GetDataFromEscompatArrayImpl()->Get(index);
}

void EtsEscompatArray::EscompatArraySet(EtsInt index, EtsObject *value)
{
    // Can use private fields as part of `std.core.Array` method implementation
    if (UNLIKELY(IsOutOfBounds(index, GetActualLengthFromEscompatArrayImpl()))) {
        return;
    }
    GetDataFromEscompatArrayImpl()->Set(index, value);
}

EtsObject *EtsEscompatArray::EscompatArrayGetUnsafe(EtsInt index)
{
    ASSERT(index >= 0);
    [[maybe_unused]] EtsInt actualLength = GetActualLengthFromEscompatArrayImpl();
    ASSERT(index < actualLength);
    return GetDataFromEscompatArrayImpl()->Get(index);
}

void EtsEscompatArray::EscompatArraySetUnsafe(EtsInt index, EtsObject *value)
{
    ASSERT(index >= 0);
    [[maybe_unused]] EtsInt actualLength = GetActualLengthFromEscompatArrayImpl();
    ASSERT(index < actualLength);
    GetDataFromEscompatArrayImpl()->Set(index, value);
}

bool EtsEscompatArray::GetLength(EtsExecutionContext *executionCtx, EtsInt *result)
{
    ASSERT(executionCtx != nullptr);
    ASSERT(!executionCtx->GetMT()->HasPendingException());
    ASSERT(result != nullptr);
    if (LIKELY(IsExactlyEscompatArray(executionCtx))) {
        // Fast path, object is exactly `std.core.Array`
        *result = GetActualLengthFromEscompatArray();
        return true;
    }
    // Slow path, because `length` getter might be overriden
    EtsMethod *lengthGetter = GetClass()->ResolveVirtualMethod(PlatformTypes(executionCtx)->escompatArrayGetLength);
    ASSERT(lengthGetter != nullptr);
    Value args(GetCoreType());
    Value resultLength = lengthGetter->GetPandaMethod()->Invoke(executionCtx->GetMT(), &args);
    if (executionCtx->GetMT()->HasPendingException()) {
        return false;
    }
    *result = resultLength.GetAs<EtsInt>();
    return true;
}

bool EtsEscompatArray::SetRef(EtsExecutionContext *executionCtx, size_t index, EtsObject *ref)
{
    ASSERT(executionCtx != nullptr);
    ASSERT(!executionCtx->GetMT()->HasPendingException());
    if (LIKELY(IsExactlyEscompatArray(executionCtx))) {
        // Fast path, object is exactly `std.core.Array`
        EscompatArraySet(index, ref);
    } else {
        // Slow path, because `$_set` might be overriden
        EtsMethod *setter = GetClass()->ResolveVirtualMethod(PlatformTypes(executionCtx)->escompatArraySet);
        ASSERT(setter != nullptr);
        std::array args = {Value(GetCoreType()), Value(static_cast<EtsInt>(index)), Value(ref->GetCoreType())};
        setter->GetPandaMethod()->Invoke(executionCtx->GetMT(), args.data());
    }
    return !executionCtx->GetMT()->HasPendingException();
}

std::optional<EtsObject *> EtsEscompatArray::GetRef(EtsExecutionContext *executionCtx, size_t index)
{
    ASSERT(executionCtx != nullptr);
    ASSERT(!executionCtx->GetMT()->HasPendingException());
    if (LIKELY(IsExactlyEscompatArray(executionCtx))) {
        // Fast path, object is exactly `std.core.Array`
        auto *obj = EscompatArrayGet(index);
        return executionCtx->GetMT()->HasPendingException() ? std::nullopt : std::optional<EtsObject *>(obj);
    }
    // Slow path, because `$_get` might be overriden
    EtsMethod *getter = GetClass()->ResolveVirtualMethod(PlatformTypes(executionCtx)->escompatArrayGet);
    ASSERT(getter != nullptr);
    std::array args = {Value(GetCoreType()), Value(static_cast<EtsInt>(index))};
    Value result = getter->GetPandaMethod()->Invoke(executionCtx->GetMT(), args.data());
    if (executionCtx->GetMT()->HasPendingException()) {
        return {};
    }
    return EtsObject::FromCoreType(result.GetAs<ObjectHeader *>());
}

EtsObject *EtsEscompatArray::Pop(EtsExecutionContext *executionCtx)
{
    ASSERT(executionCtx != nullptr);
    ASSERT(!executionCtx->GetMT()->HasPendingException());
    if (LIKELY(IsExactlyEscompatArray(executionCtx))) {
        // Fast path, object is exactly `std.core.Array`
        return EscompatArrayPop();
    }

    // Slow path, because `pop` might be overriden
    auto *popMethod = GetClass()->ResolveVirtualMethod(PlatformTypes(executionCtx)->escompatArrayPop);
    ASSERT(popMethod != nullptr);

    Value arg(GetCoreType());
    Value result = popMethod->GetPandaMethod()->Invoke(executionCtx->GetMT(), &arg);
    if (executionCtx->GetMT()->HasPendingException()) {
        return nullptr;
    }
    return EtsObject::FromCoreType(result.GetAs<ObjectHeader *>());
}

EtsObject *EtsEscompatArray::EscompatArrayPop()
{
    auto length = GetActualLengthFromEscompatArrayImpl();
    if (length == 0) {
        return nullptr;
    }
    --length;

    auto *underlyingBuffer = GetDataFromEscompatArrayImpl();
    auto ref = underlyingBuffer->Get(length);
    underlyingBuffer->Set(length, nullptr);
    SetActualLengthFromEscompatArrayImpl(length);
    return ref;
}

}  // namespace ark::ets
