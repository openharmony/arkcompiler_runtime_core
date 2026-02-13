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

#include <charconv>
#include <optional>
#include <type_traits>
#include <cmath>
#include "ets_handle.h"
#include "ets_panda_file_items.h"
#include "libarkbase/utils/utf.h"
#include "libarkbase/utils/utils.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_typed_arrays.h"
#include "plugins/ets/runtime/types/ets_typed_unsigned_arrays.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include "intrinsics.h"
#include "cross_values.h"
#include "types/ets_object.h"
#include "types/ets_primitives.h"

constexpr uint64_t DOUBLE_SIGNIFICAND_SIZE = 52;
constexpr uint64_t DOUBLE_EXPONENT_BIAS = 1023;
constexpr uint64_t DOUBLE_SIGN_MASK = 0x8000000000000000ULL;
constexpr uint64_t DOUBLE_EXPONENT_MASK = (0x7FFULL << DOUBLE_SIGNIFICAND_SIZE);
constexpr uint64_t DOUBLE_SIGNIFICAND_MASK = 0x000FFFFFFFFFFFFFULL;
constexpr uint64_t DOUBLE_HIDDEN_BIT = (1ULL << DOUBLE_SIGNIFICAND_SIZE);
#define INT64_BITS (64)
#define INT8_BITS (8)
#define INT16_BITS (16)
#define INT32_BITS (32)

namespace ark::ets::intrinsics {

template <typename T>
static void *GetNativeData(T *array)
{
    auto *arrayBuffer = static_cast<EtsStdCoreArrayBuffer *>(&*array->GetBuffer());
    if (UNLIKELY(arrayBuffer->WasDetached())) {
        EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->escompatTypeError, "ArrayBuffer was detached");
        return nullptr;
    }
    return arrayBuffer->GetData();
}

template <typename T>
static void EtsCoreTypedArraySet(T *thisArray, EtsInt pos, typename T::ElementType val)
{
    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return;
    }

    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (UNLIKELY(pos < 0 || pos >= thisArray->GetLengthInt())) {
        EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreRangeError, "invalid index");
        return;
    }
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    ObjectAccessor::SetPrimitive(
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        data, pos * sizeof(typename T::ElementType) + static_cast<EtsInt>(thisArray->GetByteOffset()), val);
}

template <typename T>
typename T::ElementType EtsCoreTypedArrayGet(T *thisArray, EtsInt pos)
{
    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return 0;
    }

    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (UNLIKELY(pos < 0 || pos >= thisArray->GetLengthInt())) {
        EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreRangeError, "invalid index");
        return 0;
    }
    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return ObjectAccessor::GetPrimitive<typename T::ElementType>(
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        data, pos * sizeof(typename T::ElementType) + static_cast<EtsInt>(thisArray->GetByteOffset()));
}

template <typename T>
typename T::ElementType EtsCoreTypedArrayGetUnsafe(T *thisArray, EtsInt pos)
{
    ASSERT(pos >= 0 && pos < thisArray->GetLengthInt());

    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return 0;
    }

    return ObjectAccessor::GetPrimitive<typename T::ElementType>(
        data, pos * sizeof(typename T::ElementType) + static_cast<EtsInt>(thisArray->GetByteOffset()));
}

extern "C" void EtsStdCoreInt8ArraySetInt(ark::ets::EtsStdCoreInt8Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" void EtsStdCoreInt8ArraySetByte(ark::ets::EtsStdCoreInt8Array *thisArray, EtsInt pos, EtsByte val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsStdCoreInt8ArrayGet(ark::ets::EtsStdCoreInt8Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsByte EtsStdCoreInt8ArrayGetUnsafe(ark::ets::EtsStdCoreInt8Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

// We can just copy the backing buffer if the types are the same, or if we are converting e.g. Int16 <-> Uint16, as
// the binary representation will be the same. Unfortunately, we cannot use this fact to convert Uint32 -> Int32,
// because in the already implemented method Int32Array.from(ArrayLike<number>), the Uint32 -> Double -> Int32
// conversion takes place actually and Double -> Int32 clips values to MAX_INT.
template <typename T, typename U>
struct CompatibleElements : std::false_type {
};

template <>
struct CompatibleElements<EtsStdCoreInt8Array, EtsStdCoreUInt8Array> : std::true_type {
};

template <>
struct CompatibleElements<EtsStdCoreUInt8Array, EtsStdCoreInt8Array> : std::true_type {
};

template <>
struct CompatibleElements<EtsStdCoreInt16Array, EtsStdCoreUInt16Array> : std::true_type {
};

template <>
struct CompatibleElements<EtsStdCoreUInt16Array, EtsStdCoreInt16Array> : std::true_type {
};

template <>
struct CompatibleElements<EtsStdCoreUInt32Array, EtsStdCoreInt32Array> : std::true_type {
};

template <typename T, typename S = T,
          typename = std::enable_if_t<std::disjunction_v<std::is_same<T, S>, CompatibleElements<T, S>>>>
static void EtsCoreTypedArraySetValuesImpl(T *thisArray, S *srcArray, EtsInt pos)
{
    auto *dstData = GetNativeData(thisArray);
    if (UNLIKELY(dstData == nullptr)) {
        return;
    }
    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *srcData = GetNativeData(srcArray);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (UNLIKELY(srcData == nullptr)) {
        return;
    }

    using ElementType = typename T::ElementType;
    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (UNLIKELY(pos < 0 || pos + srcArray->GetLengthInt() > thisArray->GetLengthInt())) {
        EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreRangeError, "offset is out of bounds");
        return;
    }
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (UNLIKELY(srcArray->GetLengthInt() == 0)) {
        return;
    }
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *dst = ToVoidPtr(ToUintPtr(dstData) + thisArray->GetByteOffset() + pos * sizeof(ElementType));
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *src = ToVoidPtr(ToUintPtr(srcData) + srcArray->GetByteOffset());
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    [[maybe_unused]] auto error = memmove_s(dst, (thisArray->GetLengthInt() - pos) * sizeof(ElementType), src,
                                            // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
                                            srcArray->GetLengthInt() * sizeof(ElementType));
    ASSERT(error == EOK);
}

namespace {

namespace unbox {

constexpr uint64_t LONG_SHIFT = 32U;
constexpr uint64_t LONG_MASK = (uint64_t {1} << LONG_SHIFT) - uint64_t {1U};

[[nodiscard]] EtsLong GetLong(const EtsBigInt *bigint)
{
    const auto *bytes = bigint->GetBytes();
    ASSERT(bytes != nullptr);
    auto buff = EtsLong {0};
    const auto len = std::min(uint32_t {2U}, static_cast<uint32_t>(bytes->GetLength()));
    for (uint32_t i = 0; i < len; ++i) {
        buff += static_cast<EtsLong>((static_cast<uint32_t>(bytes->Get(i)) & LONG_MASK) << i * LONG_SHIFT);
    }
    return bigint->GetSign() * buff;
}

[[nodiscard]] EtsUlong GetULong(const EtsBigInt *bigint)
{
    return static_cast<EtsUlong>(GetLong(bigint));
}

template <typename Cond>
bool CheckCastedClass(Cond cond, const EtsClass *expectedClass, const EtsClass *objectClass)
{
    if (UNLIKELY(!cond(objectClass))) {
        ThrowClassCastException(expectedClass->GetRuntimeClass(), objectClass->GetRuntimeClass());
        return false;
    }
    return true;
}

template <typename T, typename = void>
class ArrayElement;

template <typename T>
class ArrayElement<T, std::enable_if_t<std::is_integral_v<typename T::ElementType>>> {
public:
    using Type = typename T::ElementType;

    ArrayElement() : coreIntClass_(PlatformTypes(EtsExecutionContext::GetCurrent())->coreInt) {}

    std::optional<Type> GetTyped(EtsObject *object) const
    {
        const auto value = Unbox(object);
        return value.has_value() ? std::optional(ToTyped(*value)) : std::nullopt;
    }

private:
    std::optional<EtsInt> Unbox(EtsObject *object) const
    {
        if (CheckCastedClass([](const EtsClass *klass) { return klass->IsBoxedInt(); }, coreIntClass_,
                             object->GetClass())) {
            return EtsBoxPrimitive<EtsInt>::Unbox(object);
        }
        return std::nullopt;
    }

    static Type ToTyped(EtsInt value)
    {
        return static_cast<Type>(value);
    }

    const EtsClass *coreIntClass_;
};

template <typename T>
class ArrayElement<T, std::enable_if_t<std::is_floating_point_v<typename T::ElementType>>> {
public:
    using Type = typename T::ElementType;

    ArrayElement() : coreDoubleClass_(PlatformTypes(EtsExecutionContext::GetCurrent())->coreDouble) {}

    std::optional<Type> GetTyped(EtsObject *object) const
    {
        const auto value = Unbox(object);
        return value.has_value() ? std::optional(ToTyped(*value)) : std::nullopt;
    }

private:
    std::optional<EtsDouble> Unbox(EtsObject *object) const
    {
        if (CheckCastedClass([](const EtsClass *klass) { return klass->IsBoxedDouble(); }, coreDoubleClass_,
                             object->GetClass())) {
            return EtsBoxPrimitive<EtsDouble>::Unbox(object);
        }
        return std::nullopt;
    }

    static Type ToTyped(EtsDouble value);

    const EtsClass *coreDoubleClass_;
};

template <>
auto ArrayElement<EtsStdCoreInt32Array>::ToTyped(EtsInt value) -> Type
{
    return value;
}

template <>
auto ArrayElement<EtsStdCoreUInt8ClampedArray>::ToTyped(EtsInt value) -> Type
{
    if (!(value > ark::ets::EtsStdCoreUInt8ClampedArray::MIN)) {
        return ark::ets::EtsStdCoreUInt8ClampedArray::MIN;
    }
    if (value > ark::ets::EtsStdCoreUInt8ClampedArray::MAX) {
        return ark::ets::EtsStdCoreUInt8ClampedArray::MAX;
    }
    return static_cast<uint8_t>(value);
}

template <>
auto ArrayElement<EtsStdCoreFloat32Array>::ToTyped(EtsDouble value) -> Type
{
    return static_cast<Type>(value);
}

template <>
auto ArrayElement<EtsStdCoreFloat64Array>::ToTyped(EtsDouble value) -> Type
{
    return value;
}

template <typename ArrayType>
class BigInt64ArrayElement {
public:
    using Type = typename ArrayType::ElementType;

    BigInt64ArrayElement() : stdCoreBigIntClass_(PlatformTypes(EtsExecutionContext::GetCurrent())->coreBigInt) {}

    std::optional<Type> GetTyped(EtsObject *object) const
    {
        const auto *bigint = Cast(object);
        return bigint != nullptr ? std::optional(ArrayElement<ArrayType>::ToTyped(bigint)) : std::nullopt;
    }

private:
    const EtsBigInt *Cast(EtsObject *object) const
    {
        if (CheckCastedClass([](const EtsClass *klass) { return klass->IsBigInt(); }, stdCoreBigIntClass_,
                             object->GetClass())) {
            return EtsBigInt::FromEtsObject(object);
        }
        return nullptr;
    }

    static Type ToTyped([[maybe_unused]] const EtsBigInt *bigint);

    friend ArrayElement<ArrayType>;

    const EtsClass *stdCoreBigIntClass_;
};

template <>
class ArrayElement<EtsStdCoreBigInt64Array> final : public BigInt64ArrayElement<EtsStdCoreBigInt64Array> {
private:
    static Type ToTyped(const EtsBigInt *bigint)
    {
        ASSERT(bigint != nullptr);
        return GetLong(bigint);
    }
    friend BigInt64ArrayElement<EtsStdCoreBigInt64Array>;
};

template <>
class ArrayElement<EtsStdCoreBigUInt64Array> final : public BigInt64ArrayElement<EtsStdCoreBigUInt64Array> {
private:
    static Type ToTyped(const EtsBigInt *bigint)
    {
        ASSERT(bigint != nullptr);
        return GetULong(bigint);
    }
    friend BigInt64ArrayElement<EtsStdCoreBigUInt64Array>;
};

}  // namespace unbox
}  // namespace

template <typename T>
static void EtsCoreTypedArraySetValuesFromFixedArray(T *thisArray, void *dstData, EtsObjectArray *srcData,
                                                     uint32_t actualLength)
{
    using ElementType = typename T::ElementType;

    ASSERT(thisArray != nullptr);
    ASSERT(dstData != nullptr);
    ASSERT(srcData != nullptr);

    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (UNLIKELY(actualLength > static_cast<uint32_t>(thisArray->GetLengthInt()))) {
        EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreRangeError, "offset is out of bounds");
        return;
    }

    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto offset = static_cast<EtsInt>(thisArray->GetByteOffset());
    const auto arrayElement = unbox::ArrayElement<T>();
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    for (size_t i = 0; i < actualLength; ++i) {
        const auto val = arrayElement.GetTyped(srcData->Get(i));
        if (!val.has_value()) {
            break;
        }
        ObjectAccessor::SetPrimitive(dstData, offset, *val);
        offset += sizeof(ElementType);
    }
}

/// Slow path, because methods of `srcArray` might be overriden
template <typename T>
static void EtsCoreTypedArraySetValuesFromArraySlowPath(T *thisArray, void *dstData, EtsEscompatArray *srcArray,
                                                        EtsExecutionContext *executionCtx)
{
    using ElementType = typename T::ElementType;

    ASSERT(thisArray != nullptr);
    ASSERT(srcArray != nullptr);
    ASSERT(executionCtx != nullptr);

    EtsInt thisArrayLengthInt = thisArray->GetLengthInt();
    EtsInt offset = thisArray->GetByteOffset();

    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsEscompatArray> srcArrayHandle(executionCtx, srcArray);

    EtsInt actualLength = 0;
    if (UNLIKELY(!srcArrayHandle->GetLength(executionCtx, &actualLength))) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return;
    }

    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (UNLIKELY(actualLength < 0 || actualLength > thisArrayLengthInt)) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreRangeError, "offset is out of bounds");
        return;
    }

    const auto arrayElement = unbox::ArrayElement<T>();
    for (size_t i = 0; i < static_cast<size_t>(actualLength); ++i) {
        auto optElement = srcArrayHandle->GetRef(executionCtx, i);
        if (UNLIKELY(!optElement)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            return;
        }
        if (UNLIKELY(*optElement == nullptr)) {
            PandaStringStream ss;
            ss << "element at index " << i << " is undefined";
            ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreNullPointerError, ss.str());
            return;
        }
        const auto val = arrayElement.GetTyped(*optElement);
        if (!val.has_value()) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            break;
        }
        ObjectAccessor::SetPrimitive(dstData, offset, *val);
        offset += sizeof(ElementType);
    }
}

template <typename T>
static void EtsCoreTypedArraySetValuesFromArrayImpl(T *thisArray, EtsEscompatArray *srcArray)
{
    ASSERT(thisArray != nullptr);
    ASSERT(srcArray != nullptr);

    auto *dstData = GetNativeData(thisArray);
    if (UNLIKELY(dstData == nullptr)) {
        return;
    }

    EtsExecutionContext *executionCtx = EtsExecutionContext::GetCurrent();
    if (LIKELY(srcArray->IsExactlyEscompatArray(executionCtx))) {
        // Fast path in case of `srcArray` being exactly `std.core.Array`
        EtsCoreTypedArraySetValuesFromFixedArray(thisArray, dstData, srcArray->GetDataFromEscompatArray(),
                                                 srcArray->GetActualLengthFromEscompatArray());
    } else {
        EtsCoreTypedArraySetValuesFromArraySlowPath(thisArray, dstData, srcArray, executionCtx);
    }
}

extern "C" void EtsStdCoreInt8ArraySetValues(ark::ets::EtsStdCoreInt8Array *thisArray,
                                             ark::ets::EtsStdCoreInt8Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreInt8ArraySetValuesFromUnsigned(ark::ets::EtsStdCoreInt8Array *thisArray,
                                                         ark::ets::EtsStdCoreUInt8Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreInt8ArraySetValuesWithOffset(ark::ets::EtsStdCoreInt8Array *thisArray,
                                                       ark::ets::EtsStdCoreInt8Array *srcArray, EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreInt8ArraySetValuesFromArray(ark::ets::EtsStdCoreInt8Array *thisArray,
                                                      ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

template <typename T, typename V>
static void EtsCoreTypedArrayFillInternal(T *thisArray, V val, EtsInt begin, EtsInt end)
{
    static_assert(sizeof(V) == sizeof(typename T::ElementType));
    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return;
    }
    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto offset = static_cast<EtsInt>(thisArray->GetByteOffset()) + begin * sizeof(V);
    for (auto i = begin; i < end; ++i) {
        ObjectAccessor::SetPrimitive(data, offset, val);
        offset += sizeof(V);
    }
}

extern "C" void EtsStdCoreInt8ArrayFillInternal(ark::ets::EtsStdCoreInt8Array *thisArray, EtsByte val, EtsInt begin,
                                                EtsInt end)
{
    EtsCoreTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsStdCoreInt16ArraySetInt(ark::ets::EtsStdCoreInt16Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" void EtsStdCoreInt16ArraySetShort(ark::ets::EtsStdCoreInt16Array *thisArray, EtsInt pos, EtsShort val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsStdCoreInt16ArrayGet(ark::ets::EtsStdCoreInt16Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsShort EtsStdCoreInt16ArrayGetUnsafe(ark::ets::EtsStdCoreInt16Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

extern "C" void EtsStdCoreInt16ArraySetValues(ark::ets::EtsStdCoreInt16Array *thisArray,
                                              ark::ets::EtsStdCoreInt16Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreInt16ArraySetValuesFromUnsigned(ark::ets::EtsStdCoreInt16Array *thisArray,
                                                          ark::ets::EtsStdCoreUInt16Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreInt16ArraySetValuesWithOffset(ark::ets::EtsStdCoreInt16Array *thisArray,
                                                        ark::ets::EtsStdCoreInt16Array *srcArray, EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreInt16ArraySetValuesFromArray(ark::ets::EtsStdCoreInt16Array *thisArray,
                                                       ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

extern "C" void EtsStdCoreInt16ArrayFillInternal(ark::ets::EtsStdCoreInt16Array *thisArray, EtsShort val, EtsInt begin,
                                                 EtsInt end)
{
    EtsCoreTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsStdCoreInt32ArraySetInt(ark::ets::EtsStdCoreInt32Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsStdCoreInt32ArrayGet(ark::ets::EtsStdCoreInt32Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsInt EtsStdCoreInt32ArrayGetUnsafe(ark::ets::EtsStdCoreInt32Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

extern "C" void EtsStdCoreInt32ArraySetValues(ark::ets::EtsStdCoreInt32Array *thisArray,
                                              ark::ets::EtsStdCoreInt32Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreInt32ArraySetValuesWithOffset(ark::ets::EtsStdCoreInt32Array *thisArray,
                                                        ark::ets::EtsStdCoreInt32Array *srcArray, EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreInt32ArraySetValuesFromArray(ark::ets::EtsStdCoreInt32Array *thisArray,
                                                       ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

extern "C" void EtsStdCoreInt32ArrayFillInternal(ark::ets::EtsStdCoreInt32Array *thisArray, EtsInt val, EtsInt begin,
                                                 EtsInt end)
{
    EtsCoreTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsStdCoreBigInt64ArraySetLong(ark::ets::EtsStdCoreBigInt64Array *thisArray, EtsInt pos, EtsLong val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsLong EtsStdCoreBigInt64ArrayGet(ark::ets::EtsStdCoreBigInt64Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsLong EtsStdCoreBigInt64ArrayGetUnsafe(ark::ets::EtsStdCoreBigInt64Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

extern "C" void EtsStdCoreBigInt64ArraySetValues(ark::ets::EtsStdCoreBigInt64Array *thisArray,
                                                 ark::ets::EtsStdCoreBigInt64Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreBigInt64ArraySetValuesWithOffset(ark::ets::EtsStdCoreBigInt64Array *thisArray,
                                                           ark::ets::EtsStdCoreBigInt64Array *srcArray, EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreBigInt64ArraySetValuesFromArray(ark::ets::EtsStdCoreBigInt64Array *thisArray,
                                                          ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

extern "C" void EtsStdCoreBigInt64ArrayFillInternal(ark::ets::EtsStdCoreBigInt64Array *thisArray, EtsLong val,
                                                    EtsInt begin, EtsInt end)
{
    EtsCoreTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsStdCoreFloat32ArraySetFloat(ark::ets::EtsStdCoreFloat32Array *thisArray, EtsInt pos, EtsFloat val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsStdCoreFloat32ArrayGet(ark::ets::EtsStdCoreFloat32Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsFloat EtsStdCoreFloat32ArrayGetUnsafe(ark::ets::EtsStdCoreFloat32Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

extern "C" void EtsStdCoreFloat32ArraySetValues(ark::ets::EtsStdCoreFloat32Array *thisArray,
                                                ark::ets::EtsStdCoreFloat32Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreFloat32ArraySetValuesWithOffset(ark::ets::EtsStdCoreFloat32Array *thisArray,
                                                          ark::ets::EtsStdCoreFloat32Array *srcArray, EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreFloat32ArraySetValuesFromArray(ark::ets::EtsStdCoreFloat32Array *thisArray,
                                                         ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

extern "C" void EtsStdCoreFloat32ArrayFillInternal(ark::ets::EtsStdCoreFloat32Array *thisArray, EtsFloat val,
                                                   EtsInt begin, EtsInt end)
{
    EtsCoreTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsStdCoreFloat32ArrayFillInternalInt(ark::ets::EtsStdCoreFloat32Array *thisArray, int32_t val,
                                                      EtsInt begin, EtsInt end)
{
    EtsCoreTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsStdCoreFloat64ArraySetDouble(ark::ets::EtsStdCoreFloat64Array *thisArray, EtsInt pos, EtsDouble val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsStdCoreFloat64ArrayGet(ark::ets::EtsStdCoreFloat64Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsDouble EtsStdCoreFloat64ArrayGetUnsafe(ark::ets::EtsStdCoreFloat64Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

extern "C" void EtsStdCoreFloat64ArraySetValues(ark::ets::EtsStdCoreFloat64Array *thisArray,
                                                ark::ets::EtsStdCoreFloat64Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreFloat64ArraySetValuesWithOffset(ark::ets::EtsStdCoreFloat64Array *thisArray,
                                                          ark::ets::EtsStdCoreFloat64Array *srcArray, EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreFloat64ArraySetValuesFromArray(ark::ets::EtsStdCoreFloat64Array *thisArray,
                                                         ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

extern "C" void EtsStdCoreFloat64ArrayFillInternal(ark::ets::EtsStdCoreFloat64Array *thisArray, EtsDouble val,
                                                   EtsInt begin, EtsInt end)
{
    EtsCoreTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsStdCoreFloat64ArrayFillInternalInt(ark::ets::EtsStdCoreFloat64Array *thisArray, int64_t val,
                                                      EtsInt begin, EtsInt end)
{
    EtsCoreTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsStdCoreUInt8ClampedArraySetInt(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray, EtsInt pos,
                                                  EtsInt val)
{
    if (val > ark::ets::EtsStdCoreUInt8ClampedArray::MAX) {
        val = ark::ets::EtsStdCoreUInt8ClampedArray::MAX;
    } else if (val < ark::ets::EtsStdCoreUInt8ClampedArray::MIN) {
        val = ark::ets::EtsStdCoreUInt8ClampedArray::MIN;
    }
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsStdCoreUInt8ClampedArrayGet(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsInt EtsStdCoreUInt8ClampedArrayGetUnsafe(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

extern "C" void EtsStdCoreUInt8ClampedArraySetValues(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray,
                                                     ark::ets::EtsStdCoreUInt8ClampedArray *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreUInt8ClampedArraySetValuesWithOffset(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray,
                                                               ark::ets::EtsStdCoreUInt8ClampedArray *srcArray,
                                                               EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreUint8ClampedArraySetValuesFromArray(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray,
                                                              ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

extern "C" void EtsStdCoreUInt8ClampedArrayFillInternal(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray, EtsInt val,
                                                        EtsInt begin, EtsInt end)
{
    using ElementType = ark::ets::EtsStdCoreUInt8ClampedArray::ElementType;
    EtsCoreTypedArrayFillInternal(thisArray, static_cast<ElementType>(val), begin, end);
}

extern "C" void EtsStdCoreUInt8ArraySetInt(ark::ets::EtsStdCoreUInt8Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsStdCoreUInt8ArrayGet(ark::ets::EtsStdCoreUInt8Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsInt EtsStdCoreUInt8ArrayGetUnsafe(ark::ets::EtsStdCoreUInt8Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

extern "C" void EtsStdCoreUInt8ArraySetValues(ark::ets::EtsStdCoreUInt8Array *thisArray,
                                              ark::ets::EtsStdCoreUInt8Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreUint8ArraySetValuesFromSigned(ark::ets::EtsStdCoreUInt8Array *thisArray,
                                                        ark::ets::EtsStdCoreInt8Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreUInt8ArraySetValuesWithOffset(ark::ets::EtsStdCoreUInt8Array *thisArray,
                                                        ark::ets::EtsStdCoreUInt8Array *srcArray, EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreUint8ArraySetValuesFromArray(ark::ets::EtsStdCoreUInt8Array *thisArray,
                                                       ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

extern "C" void EtsStdCoreUInt8ArrayFillInternal(ark::ets::EtsStdCoreUInt8Array *thisArray, EtsInt val, EtsInt begin,
                                                 EtsInt end)
{
    using ElementType = ark::ets::EtsStdCoreUInt8Array::ElementType;
    EtsCoreTypedArrayFillInternal(thisArray, static_cast<ElementType>(val), begin, end);
}

extern "C" void EtsStdCoreUInt16ArraySetInt(ark::ets::EtsStdCoreUInt16Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsStdCoreUInt16ArrayGet(ark::ets::EtsStdCoreUInt16Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsInt EtsStdCoreUInt16ArrayGetUnsafe(ark::ets::EtsStdCoreUInt16Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

extern "C" void EtsStdCoreUInt16ArraySetValues(ark::ets::EtsStdCoreUInt16Array *thisArray,
                                               ark::ets::EtsStdCoreUInt16Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreUint16ArraySetValuesFromSigned(ark::ets::EtsStdCoreUInt16Array *thisArray,
                                                         ark::ets::EtsStdCoreInt16Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreUInt16ArraySetValuesWithOffset(ark::ets::EtsStdCoreUInt16Array *thisArray,
                                                         ark::ets::EtsStdCoreUInt16Array *srcArray, EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreUint16ArraySetValuesFromArray(ark::ets::EtsStdCoreUInt16Array *thisArray,
                                                        ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

extern "C" void EtsStdCoreUInt16ArrayFillInternal(ark::ets::EtsStdCoreUInt16Array *thisArray, EtsInt val, EtsInt begin,
                                                  EtsInt end)
{
    using ElementType = ark::ets::EtsStdCoreUInt16Array::ElementType;
    EtsCoreTypedArrayFillInternal(thisArray, static_cast<ElementType>(val), begin, end);
}

extern "C" void EtsStdCoreUInt32ArraySetInt(ark::ets::EtsStdCoreUInt32Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" void EtsStdCoreUInt32ArraySetLong(ark::ets::EtsStdCoreUInt32Array *thisArray, EtsInt pos, EtsLong val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsStdCoreUInt32ArrayGet(ark::ets::EtsStdCoreUInt32Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsLong EtsStdCoreUInt32ArrayGetUnsafe(ark::ets::EtsStdCoreUInt32Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

extern "C" void EtsStdCoreUInt32ArraySetValues(ark::ets::EtsStdCoreUInt32Array *thisArray,
                                               ark::ets::EtsStdCoreUInt32Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreUint32ArraySetValuesFromSigned(ark::ets::EtsStdCoreUInt32Array *thisArray,
                                                         ark::ets::EtsStdCoreInt32Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreUInt32ArraySetValuesWithOffset(ark::ets::EtsStdCoreUInt32Array *thisArray,
                                                         ark::ets::EtsStdCoreUInt32Array *srcArray, EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreUint32ArraySetValuesFromArray(ark::ets::EtsStdCoreUInt32Array *thisArray,
                                                        ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

extern "C" void EtsStdCoreUInt32ArrayFillInternal(ark::ets::EtsStdCoreUInt32Array *thisArray, EtsLong val, EtsInt begin,
                                                  EtsInt end)
{
    using ElementType = ark::ets::EtsStdCoreUInt32Array::ElementType;
    EtsCoreTypedArrayFillInternal(thisArray, static_cast<ElementType>(val), begin, end);
}

extern "C" void EtsStdCoreBigUInt64ArraySetInt(ark::ets::EtsStdCoreBigUInt64Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" void EtsStdCoreBigUInt64ArraySetLong(ark::ets::EtsStdCoreBigUInt64Array *thisArray, EtsInt pos, EtsLong val)
{
    EtsCoreTypedArraySet(thisArray, pos, val);
}

extern "C" EtsLong EtsStdCoreBigUInt64ArrayGet(ark::ets::EtsStdCoreBigUInt64Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGet(thisArray, pos);
}

extern "C" EtsLong EtsStdCoreBigUInt64ArrayGetUnsafe(ark::ets::EtsStdCoreBigUInt64Array *thisArray, EtsInt pos)
{
    return EtsCoreTypedArrayGetUnsafe(thisArray, pos);
}

extern "C" void EtsStdCoreBigUInt64ArraySetValues(ark::ets::EtsStdCoreBigUInt64Array *thisArray,
                                                  ark::ets::EtsStdCoreBigUInt64Array *srcArray)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsStdCoreBigUInt64ArraySetValuesWithOffset(ark::ets::EtsStdCoreBigUInt64Array *thisArray,
                                                            ark::ets::EtsStdCoreBigUInt64Array *srcArray, EtsInt pos)
{
    EtsCoreTypedArraySetValuesImpl(thisArray, srcArray, pos);
}

extern "C" void EtsStdCoreBigUint64ArraySetValuesFromArray(ark::ets::EtsStdCoreBigUInt64Array *thisArray,
                                                           ark::ets::EtsEscompatArray *srcArray)
{
    EtsCoreTypedArraySetValuesFromArrayImpl(thisArray, srcArray);
}

extern "C" void EtsStdCoreBigUInt64ArrayFillInternal(ark::ets::EtsStdCoreBigUInt64Array *thisArray, EtsLong val,
                                                     EtsInt begin, EtsInt end)
{
    EtsCoreTypedArrayFillInternal(thisArray, val, begin, end);
}

/*
 * Typed Arrays: Reverse Implementation
 */
template <typename T>
static inline T *EtsCoreTypedArrayReverseImpl(T *thisArray)
{
    auto *arrData = GetNativeData(thisArray);
    if (UNLIKELY(arrData == nullptr)) {
        return nullptr;
    }
    using ElementType = typename T::ElementType;
    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *ptrFirst = reinterpret_cast<ElementType *>(ToVoidPtr(ToUintPtr(arrData) + thisArray->GetByteOffset()));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *ptrLast = ptrFirst +
                    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
                    thisArray->GetLengthInt();
    std::reverse(ptrFirst, ptrLast);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return thisArray;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REVERSE_CALL_DECL(Type)                                                   \
    /* CC-OFFNXT(G.PRE.02) name part */                                           \
    extern "C" ark::ets::EtsStdCore##Type##Array *EtsStdCore##Type##ArrayReverse( \
        ark::ets::EtsStdCore##Type##Array *thisArray)                             \
    {                                                                             \
        /* CC-OFFNXT(G.PRE.05) function gen */                                    \
        return EtsCoreTypedArrayReverseImpl(thisArray);                           \
    }  // namespace ark::ets::intrinsics

REVERSE_CALL_DECL(Int8)
REVERSE_CALL_DECL(Int16)
REVERSE_CALL_DECL(Int32)
REVERSE_CALL_DECL(BigInt64)
REVERSE_CALL_DECL(Float32)
REVERSE_CALL_DECL(Float64)

REVERSE_CALL_DECL(UInt8)
REVERSE_CALL_DECL(UInt8Clamped)
REVERSE_CALL_DECL(UInt16)
REVERSE_CALL_DECL(UInt32)
REVERSE_CALL_DECL(BigUInt64)

#undef REVERSE_CALL_DECL

/*
 * Typed Arrays: reversed alloc data implementation: needs for toReversed().
 */
template <typename ElementType>
static ALWAYS_INLINE inline void TypedArrayReverseCopyBuffer(EtsStdCoreArrayBuffer *dstBuf,
                                                             EtsStdCoreArrayBuffer *srcBuf, EtsInt byteOffset,
                                                             EtsInt srcLength)
{
    ASSERT(srcBuf != nullptr);
    ASSERT(dstBuf != nullptr);
    ASSERT(!srcBuf->WasDetached());
    ASSERT(!dstBuf->WasDetached());
    ASSERT(byteOffset >= 0);
    ASSERT(static_cast<unsigned long>(byteOffset) <= (srcLength * sizeof(ElementType)));
    const void *src = srcBuf->GetData();
    auto *dest = dstBuf->GetData<ElementType *>();
    ASSERT(src != nullptr);
    ASSERT(dest != nullptr);
    const ElementType *start = reinterpret_cast<ElementType *>(ToUintPtr(src) + byteOffset);
    std::reverse_copy(start, start + srcLength, dest);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT(*dest == *(start + srcLength - 1));          // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define REVERSE_COPY_BUFFER_CALL_DECL(Type)                                                                \
    /* CC-OFFNXT(G.PRE.02) name part */                                                                    \
    extern "C" void EtsStdCore##Type##ArrayReverseCopyBuffer(                                              \
        EtsStdCoreArrayBuffer *dstBuf, EtsStdCoreArrayBuffer *srcBuf, EtsInt byteOffset, EtsInt srcLength) \
    {                                                                                                      \
        using ElementType = EtsStdCore##Type##Array::ElementType;                                          \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                             \
        TypedArrayReverseCopyBuffer<ElementType>(dstBuf, srcBuf, byteOffset, srcLength);                   \
    }  // namespace ark::ets::intrinsics

REVERSE_COPY_BUFFER_CALL_DECL(Int8)
REVERSE_COPY_BUFFER_CALL_DECL(Int16)
REVERSE_COPY_BUFFER_CALL_DECL(Int32)
REVERSE_COPY_BUFFER_CALL_DECL(BigInt64)
REVERSE_COPY_BUFFER_CALL_DECL(Float32)
REVERSE_COPY_BUFFER_CALL_DECL(Float64)

REVERSE_COPY_BUFFER_CALL_DECL(UInt8)
REVERSE_COPY_BUFFER_CALL_DECL(UInt16)
REVERSE_COPY_BUFFER_CALL_DECL(UInt32)
REVERSE_COPY_BUFFER_CALL_DECL(BigUInt64)

#undef REVERSE_COPY_BUFFER_CALL_DECL

static inline int32_t NormalizeIndex(int32_t idx, int32_t arrayLength)
{
    if (idx < -arrayLength) {
        return 0;
    }
    if (idx < 0) {
        return arrayLength + idx;
    }
    if (idx > arrayLength) {
        return arrayLength;
    }
    return idx;
}

template <typename T>
void EtsCoreTypedArrayCopyWithinImpl(T *thisArray, EtsInt target, EtsInt start, EtsInt count)
{
    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return;
    }

    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto arrayLength = thisArray->GetLengthInt();
    using ElementType = typename T::ElementType;
    // See (GetNativeData) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *targetAddress = ToVoidPtr(ToUintPtr(data) + thisArray->GetByteOffset() + target * sizeof(ElementType));
    // See (GetNativeData) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *startAddress = ToVoidPtr(ToUintPtr(data) + thisArray->GetByteOffset() + start * sizeof(ElementType));
    [[maybe_unused]] auto error = memmove_s(targetAddress, (arrayLength - start) * sizeof(ElementType), startAddress,
                                            count * sizeof(ElementType));
    ASSERT(error == EOK);
    // See (GetNativeData) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_STD_CORE_COPY_WITHIN(Type)                                                                                 \
    /* CC-OFFNXT(G.PRE.02) name part */                                                                                \
    extern "C" void EtsStdCore##Type##ArrayCopyWithinImpl(ark::ets::EtsStdCore##Type##Array *thisArray, EtsInt target, \
                                                          EtsInt start, EtsInt end)                                    \
    {                                                                                                                  \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                         \
        EtsCoreTypedArrayCopyWithinImpl(thisArray, target, start, end);                                                \
    }  // namespace ark::ets::intrinsics

ETS_STD_CORE_COPY_WITHIN(Int8)
ETS_STD_CORE_COPY_WITHIN(Int16)
ETS_STD_CORE_COPY_WITHIN(Int32)
ETS_STD_CORE_COPY_WITHIN(BigInt64)
ETS_STD_CORE_COPY_WITHIN(Float32)
ETS_STD_CORE_COPY_WITHIN(Float64)
ETS_STD_CORE_COPY_WITHIN(UInt8)
ETS_STD_CORE_COPY_WITHIN(UInt16)
ETS_STD_CORE_COPY_WITHIN(UInt32)
ETS_STD_CORE_COPY_WITHIN(BigUInt64)
ETS_STD_CORE_COPY_WITHIN(UInt8Clamped)

#undef ETS_STD_CORE_COPY_WITHIN

template <typename T>
bool Cmp(const typename T::ElementType &left, const typename T::ElementType &right)
{
    const auto lNumber = static_cast<double>(left);
    const auto rNumber = static_cast<double>(right);
    if (std::isnan(lNumber)) {
        return false;
    }
    if (std::isnan(rNumber)) {
        return true;
    }
    return left < right;
}

template <typename T>
T *EtsCoreTypedArraySortWrapper(T *thisArray, bool withNaN = false)
{
    using ElementType = typename T::ElementType;

    auto nBytes = static_cast<size_t>(thisArray->GetByteLength());
    ASSERT(nBytes == (static_cast<size_t>(thisArray->GetLengthInt()) * sizeof(ElementType)));
    // If it's an empty or one-element array then nothing to sort.
    if (UNLIKELY(nBytes <= sizeof(ElementType))) {
        return thisArray;
    }

    auto *arrayBuffer = static_cast<EtsStdCoreArrayBuffer *>(&*thisArray->GetBuffer());
    if (UNLIKELY(arrayBuffer->WasDetached())) {
        auto executionCtx = EtsExecutionContext::GetCurrent();
        [[maybe_unused]] EtsHandleScope scope(executionCtx);
        EtsHandle<EtsObject> handle(executionCtx, thisArray);
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->escompatTypeError, "ArrayBuffer was detached");
        return static_cast<T *>(handle.GetPtr());
    }
    void *dataPtr = arrayBuffer->GetData();
    ASSERT(dataPtr != nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    Span<EtsByte> data(static_cast<EtsByte *>(dataPtr) + static_cast<size_t>(thisArray->GetByteOffset()), nBytes);
    if (withNaN) {
        std::sort(reinterpret_cast<ElementType *>(data.begin()), reinterpret_cast<ElementType *>(data.end()), Cmp<T>);
    } else {
        std::sort(reinterpret_cast<ElementType *>(data.begin()), reinterpret_cast<ElementType *>(data.end()));
    }
    return thisArray;
}

template <typename T>
T *EtsCoreTypedArraySortWithNaN(T *thisArray)
{
    return EtsCoreTypedArraySortWrapper(thisArray, true);
}

template <typename T>
T *EtsCoreTypedArraySort(T *thisArray)
{
    return EtsCoreTypedArraySortWrapper(thisArray);
}

extern "C" ark::ets::EtsStdCoreInt8Array *EtsStdCoreInt8ArraySort(ark::ets::EtsStdCoreInt8Array *thisArray)
{
    return EtsCoreTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsStdCoreInt16Array *EtsStdCoreInt16ArraySort(ark::ets::EtsStdCoreInt16Array *thisArray)
{
    return EtsCoreTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsStdCoreInt32Array *EtsStdCoreInt32ArraySort(ark::ets::EtsStdCoreInt32Array *thisArray)
{
    return EtsCoreTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsStdCoreBigInt64Array *EtsStdCoreBigInt64ArraySort(ark::ets::EtsStdCoreBigInt64Array *thisArray)
{
    return EtsCoreTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsStdCoreFloat32Array *EtsStdCoreFloat32ArraySort(ark::ets::EtsStdCoreFloat32Array *thisArray)
{
    return EtsCoreTypedArraySortWithNaN(thisArray);
}

extern "C" ark::ets::EtsStdCoreFloat64Array *EtsStdCoreFloat64ArraySort(ark::ets::EtsStdCoreFloat64Array *thisArray)
{
    return EtsCoreTypedArraySortWithNaN(thisArray);
}

template <typename Array, typename = std::enable_if_t<std::is_floating_point_v<typename Array::ElementType>>>
static bool EtsCoreTypedArrayContainsNaN(Array *array, EtsInt pos)
{
    using ElementType = typename Array::ElementType;
    auto length = array->GetLengthInt();
    auto byteOffset = static_cast<size_t>(array->GetByteOffset());
    uint32_t normalIndex = NormalizeIndex(pos, length);
    auto *data = GetNativeData(array);
    auto *begin = ToVoidPtr(ToUintPtr(data) + byteOffset);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    Span<ElementType> span(static_cast<ElementType *>(begin) + normalIndex, length - normalIndex);
    return std::any_of(span.begin(), span.end(), [](ElementType elem) { return elem != elem; });
}

extern "C" EtsBoolean EtsStdCoreFloat32ArrayContainsNaN(ark::ets::EtsStdCoreFloat32Array *thisArray, EtsInt pos)
{
    return ToEtsBoolean(EtsCoreTypedArrayContainsNaN(thisArray, pos));
}

extern "C" EtsBoolean EtsStdCoreFloat64ArrayContainsNaN(ark::ets::EtsStdCoreFloat64Array *thisArray, EtsInt pos)
{
    return ToEtsBoolean(EtsCoreTypedArrayContainsNaN(thisArray, pos));
}

extern "C" ark::ets::EtsStdCoreUInt8ClampedArray *EtsStdCoreUInt8ClampedArraySort(
    ark::ets::EtsStdCoreUInt8ClampedArray *thisArray)
{
    return EtsCoreTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsStdCoreUInt8Array *EtsStdCoreUInt8ArraySort(ark::ets::EtsStdCoreUInt8Array *thisArray)
{
    return EtsCoreTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsStdCoreUInt16Array *EtsStdCoreUInt16ArraySort(ark::ets::EtsStdCoreUInt16Array *thisArray)
{
    return EtsCoreTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsStdCoreUInt32Array *EtsStdCoreUInt32ArraySort(ark::ets::EtsStdCoreUInt32Array *thisArray)
{
    return EtsCoreTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsStdCoreBigUInt64Array *EtsStdCoreBigUInt64ArraySort(
    ark::ets::EtsStdCoreBigUInt64Array *thisArray)
{
    return EtsCoreTypedArraySort(thisArray);
}

#define INVALID_INDEX (-1)

template <typename T, typename Pred>
static EtsInt EtsCoreTypedArrayIndexOf(T *thisArray, EtsInt fromIndex, Pred pred)
{
    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return INVALID_INDEX;
    }

    using ElementType = typename T::ElementType;
    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *array = reinterpret_cast<ElementType *>(ToUintPtr(data) + static_cast<int>(thisArray->GetByteOffset()));
    // See (GetNativeData) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto arrayLength = thisArray->GetLengthInt();
    fromIndex = NormalizeIndex(fromIndex, arrayLength);
    Span<ElementType> span(static_cast<ElementType *>(array), arrayLength);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const auto idxIt = std::find_if(span.begin() + fromIndex, span.end(), pred);
    return idxIt == span.end() ? INVALID_INDEX : std::distance(span.begin(), idxIt);
}

template <typename T, typename SE>
static EtsInt EtsCoreTypedArrayIndexOfLong(T *thisArray, SE searchElement, EtsInt fromIndex)
{
    using ElementType = typename T::ElementType;
    return EtsCoreTypedArrayIndexOf(
        thisArray, fromIndex,
        [element = static_cast<ElementType>(searchElement)](ElementType e) { return e == element; });
}

template <typename T>
static EtsInt EtsCoreTypedArrayIndexOfNumber(T *thisArray, double searchElement, EtsInt fromIndex)
{
    if (std::isnan(searchElement)) {
        return INVALID_INDEX;
    }

    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return INVALID_INDEX;
    }

    using ElementType = typename T::ElementType;
    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *array = reinterpret_cast<ElementType *>(ToUintPtr(data) + static_cast<int>(thisArray->GetByteOffset()));
    // See (GetNativeData) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto arrayLength = thisArray->GetLengthInt();
    fromIndex = NormalizeIndex(fromIndex, arrayLength);
    for (int i = fromIndex; i < arrayLength; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (static_cast<double>(array[i]) == searchElement) {
            return i;
        }
    }
    return INVALID_INDEX;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_STD_CORE_INDEX_OF_NUMBER(Type)                                                               \
    /* CC-OFFNXT(G.PRE.02) name part */                                                                  \
    extern "C" EtsInt EtsStdCore##Type##ArrayIndexOfNumber(ark::ets::EtsStdCore##Type##Array *thisArray, \
                                                           EtsDouble searchElement, EtsInt fromIndex)    \
    {                                                                                                    \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                           \
        return EtsCoreTypedArrayIndexOfNumber(thisArray, searchElement, fromIndex);                      \
    }  // namespace ark::ets::intrinsics

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_STD_CORE_INDEX_OF_LONG(Type)                                                               \
    /* CC-OFFNXT(G.PRE.02) name part */                                                                \
    extern "C" EtsInt EtsStdCore##Type##ArrayIndexOfLong(ark::ets::EtsStdCore##Type##Array *thisArray, \
                                                         EtsLong searchElement, EtsInt fromIndex)      \
    {                                                                                                  \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                         \
        return EtsCoreTypedArrayIndexOfLong(thisArray, searchElement, fromIndex);                      \
    }  // namespace ark::ets::intrinsics

ETS_STD_CORE_INDEX_OF_NUMBER(Int8)
ETS_STD_CORE_INDEX_OF_NUMBER(Int16)
ETS_STD_CORE_INDEX_OF_NUMBER(Int32)
ETS_STD_CORE_INDEX_OF_NUMBER(BigInt64)
ETS_STD_CORE_INDEX_OF_NUMBER(Float32)
ETS_STD_CORE_INDEX_OF_NUMBER(Float64)
ETS_STD_CORE_INDEX_OF_NUMBER(UInt8)
ETS_STD_CORE_INDEX_OF_NUMBER(UInt16)
ETS_STD_CORE_INDEX_OF_NUMBER(UInt32)
ETS_STD_CORE_INDEX_OF_NUMBER(BigUInt64)
ETS_STD_CORE_INDEX_OF_NUMBER(UInt8Clamped)
ETS_STD_CORE_INDEX_OF_LONG(Int8)
ETS_STD_CORE_INDEX_OF_LONG(Int16)
ETS_STD_CORE_INDEX_OF_LONG(Int32)
ETS_STD_CORE_INDEX_OF_LONG(BigInt64)
ETS_STD_CORE_INDEX_OF_LONG(Float32)
ETS_STD_CORE_INDEX_OF_LONG(Float64)
ETS_STD_CORE_INDEX_OF_LONG(UInt8)
ETS_STD_CORE_INDEX_OF_LONG(UInt16)
ETS_STD_CORE_INDEX_OF_LONG(UInt32)
ETS_STD_CORE_INDEX_OF_LONG(BigUInt64)
ETS_STD_CORE_INDEX_OF_LONG(UInt8Clamped)

template <typename T1, typename T2>
static EtsInt EtsCoreTypedArrayLastIndexOfLong(T1 *thisArray, T2 searchElement, EtsInt fromIndex)
{
    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return INVALID_INDEX;
    }

    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    int arrayLength = thisArray->GetLengthInt();
    if (arrayLength == 0) {
        return INVALID_INDEX;
    }

    int startIndex = arrayLength + fromIndex;
    if (fromIndex >= 0) {
        startIndex = (arrayLength - 1 < fromIndex) ? arrayLength - 1 : fromIndex;
    }

    using ElementType = typename T1::ElementType;
    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *array = reinterpret_cast<ElementType *>(ToUintPtr(data) + static_cast<int>(thisArray->GetByteOffset()));
    auto element = static_cast<ElementType>(searchElement);
    for (int i = startIndex; i >= 0; i--) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (array[i] == element) {
            return i;
        }
    }

    return INVALID_INDEX;
}

template <typename T>
static EtsInt EtsCoreTypedArrayLastIndexOfNumber(T *thisArray, double searchElement, EtsInt fromIndex)
{
    if (std::isnan(searchElement)) {
        return INVALID_INDEX;
    }

    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return INVALID_INDEX;
    }

    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    int arrayLength = thisArray->GetLengthInt();
    if (arrayLength == 0) {
        return INVALID_INDEX;
    }

    int startIndex = arrayLength + fromIndex;
    if (fromIndex >= 0) {
        startIndex = (arrayLength - 1 < fromIndex) ? arrayLength - 1 : fromIndex;
    }

    using ElementType = typename T::ElementType;
    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *array = reinterpret_cast<ElementType *>(ToUintPtr(data) + static_cast<int>(thisArray->GetByteOffset()));
    for (int i = startIndex; i >= 0; i--) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (static_cast<double>(array[i]) == searchElement) {
            return i;
        }
    }

    return INVALID_INDEX;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_STD_CORE_LAST_INDEX_OF_NUMBER(Type)                                                              \
    /* CC-OFFNXT(G.PRE.02) name part */                                                                      \
    extern "C" EtsInt EtsStdCore##Type##ArrayLastIndexOfNumber(ark::ets::EtsStdCore##Type##Array *thisArray, \
                                                               EtsDouble searchElement, EtsInt fromIndex)    \
    {                                                                                                        \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                               \
        return EtsCoreTypedArrayLastIndexOfNumber(thisArray, searchElement, fromIndex);                      \
    }  // namespace ark::ets::intrinsics

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_STD_CORE_LAST_INDEX_OF_LONG(Type)                                                              \
    /* CC-OFFNXT(G.PRE.02) name part */                                                                    \
    extern "C" EtsInt EtsStdCore##Type##ArrayLastIndexOfLong(ark::ets::EtsStdCore##Type##Array *thisArray, \
                                                             EtsLong searchElement, EtsInt fromIndex)      \
    {                                                                                                      \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                             \
        return EtsCoreTypedArrayLastIndexOfLong(thisArray, searchElement, fromIndex);                      \
    }  // namespace ark::ets::intrinsics

ETS_STD_CORE_LAST_INDEX_OF_NUMBER(Int8)
ETS_STD_CORE_LAST_INDEX_OF_NUMBER(Int16)
ETS_STD_CORE_LAST_INDEX_OF_NUMBER(Int32)
ETS_STD_CORE_LAST_INDEX_OF_NUMBER(BigInt64)
ETS_STD_CORE_LAST_INDEX_OF_NUMBER(Float32)
ETS_STD_CORE_LAST_INDEX_OF_NUMBER(Float64)
ETS_STD_CORE_LAST_INDEX_OF_NUMBER(UInt8)
ETS_STD_CORE_LAST_INDEX_OF_NUMBER(UInt16)
ETS_STD_CORE_LAST_INDEX_OF_NUMBER(UInt32)
ETS_STD_CORE_LAST_INDEX_OF_NUMBER(BigUInt64)
ETS_STD_CORE_LAST_INDEX_OF_NUMBER(UInt8Clamped)
ETS_STD_CORE_LAST_INDEX_OF_LONG(Int8)
ETS_STD_CORE_LAST_INDEX_OF_LONG(Int16)
ETS_STD_CORE_LAST_INDEX_OF_LONG(Int32)
ETS_STD_CORE_LAST_INDEX_OF_LONG(BigInt64)
ETS_STD_CORE_LAST_INDEX_OF_LONG(Float32)
ETS_STD_CORE_LAST_INDEX_OF_LONG(Float64)
ETS_STD_CORE_LAST_INDEX_OF_LONG(UInt8)
ETS_STD_CORE_LAST_INDEX_OF_LONG(UInt16)
ETS_STD_CORE_LAST_INDEX_OF_LONG(UInt32)
ETS_STD_CORE_LAST_INDEX_OF_LONG(BigUInt64)
ETS_STD_CORE_LAST_INDEX_OF_LONG(UInt8Clamped)

#undef INVALID_INDEX

/* Compute a max size in chars required for not null-terminated string
 * representation of the specified integral or floating point type.
 */
template <typename T>
constexpr size_t MaxChars()
{
    static_assert(std::numeric_limits<T>::radix == 2U);
    if constexpr (std::is_integral_v<T>) {
        auto bits = std::numeric_limits<T>::digits;
        auto digitPerBit = std::log10(std::numeric_limits<T>::radix);
        auto digits = std::ceil(bits * digitPerBit) + static_cast<int>(std::is_signed_v<T>);
        return digits;
    } else {
        static_assert(std::is_floating_point_v<T>);
        // Forced to treat T as double
        auto bits = std::numeric_limits<double>::digits;
        auto digitPerBit = std::log10(std::numeric_limits<double>::radix);
        auto digits = std::ceil(bits * digitPerBit) + static_cast<int>(std::is_signed_v<double>);
        // digits + point + "+e308"
        return digits + 1U + std::char_traits<char>::length("+e308");
    }
}

template <typename T>
size_t NumberToU8Chars(PandaVector<char> &buf, size_t pos, T number)
{
    if constexpr (std::is_integral_v<T>) {
        auto [strEnd, result] = std::to_chars(&buf[pos], &buf[pos + MaxChars<T>()], number);
        ASSERT(result == std::errc());
        return strEnd - &buf[pos];
    } else {
        static_assert(std::is_floating_point_v<T>);
        // Forced to treat T as double
        auto asDouble = static_cast<double>(number);
        return intrinsics::helpers::FpToStringDecimalRadix(asDouble, [&buf, &pos](std::string_view str) {
            ASSERT(str.length() <= MaxChars<T>());
            auto err = memcpy_s(&buf[pos], MaxChars<T>(), str.data(), str.length());
            if (err != EOK) {
                UNREACHABLE();
            }
            return str.length();
        });
    }
}

template <typename T>
size_t NumberToU16Chars(PandaVector<EtsChar> &buf, size_t pos, T number)
{
    if constexpr (std::is_integral_v<T>) {
        auto num = number < 0 ? -static_cast<uint64_t>(number) : static_cast<uint64_t>(number);
        auto nDigits = (CountDigits(num) + static_cast<uint32_t>(number < 0));
        ASSERT(pos + nDigits <= buf.size());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf::UInt64ToUtf16Array(num, buf.data() + pos, nDigits, number < 0);
        return nDigits;
    } else {
        static_assert(std::is_floating_point_v<T>);
        static_assert(std::is_unsigned_v<EtsChar>);
        // Forced to treat T as double
        auto asDouble = static_cast<double>(number);
        return intrinsics::helpers::FpToStringDecimalRadix(asDouble, [&buf, &pos](std::string_view str) {
            ASSERT(str.length() <= MaxChars<T>());
            for (size_t i = 0; i < str.length(); i++) {
                buf[pos + i] = static_cast<EtsChar>(str[i]);
            }
            return str.length();
        });
    }
}

template <typename T>
static ark::ets::EtsString *TypedArrayJoinUtf16(Span<T> &data, ark::ets::EtsString *separator)
{
    ASSERT(!data.empty());
    ASSERT(separator->IsUtf16());

    const size_t sepSize = separator->GetUtf16Length();
    PandaVector<EtsChar> buf(data.Size() * (MaxChars<T>() + sepSize));
    size_t strSize = 0;
    auto n = data.Size() - 1;
    if (sepSize == 1) {
        PandaVector<uint16_t> tree16Buf;
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto sep =
            separator->IsTreeString() ? separator->GetTreeStringDataUtf16(tree16Buf)[0] : separator->GetDataUtf16()[0];
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (auto i = 0U; i < n; i++) {
            strSize += NumberToU16Chars(buf, strSize, static_cast<T>(data[i]));
            buf[strSize] = sep;
            strSize += 1;
        }
    } else {
        for (auto i = 0U; i < n; i++) {
            strSize += NumberToU16Chars(buf, strSize, static_cast<T>(data[i]));
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            separator->CopyDataUtf16(buf.data() + strSize, sepSize);
            strSize += sepSize;
        }
    }
    strSize += NumberToU16Chars(buf, strSize, static_cast<T>(data[n]));
    return EtsString::CreateFromUtf16(reinterpret_cast<EtsChar *>(buf.data()), strSize);
}

template <typename T>
static ark::ets::EtsString *TypedArrayJoinUtf8(Span<T> &data)
{
    ASSERT(!data.empty());
    PandaVector<char> buf(data.Size() * MaxChars<T>());
    size_t strSize = 0;
    auto n = data.Size();
    for (auto i = 0U; i < n; i++) {
        strSize += NumberToU8Chars(buf, strSize, data[i]);
    }
    return EtsString::CreateFromUtf8(buf.data(), strSize);
}

template <typename T>
static ark::ets::EtsString *TypedArrayJoinUtf8(Span<T> &data, ark::ets::EtsString *separator)
{
    ASSERT(!data.empty());
    ASSERT(!separator->IsUtf16() && !separator->IsEmpty());
    const size_t sepSize = separator->GetUtf8Length();
    PandaVector<char> buf(data.Size() * (MaxChars<T>() + sepSize));
    size_t strSize = 0;
    auto n = data.Size() - 1;
    if (sepSize == 1) {
        PandaVector<uint8_t> tree8Buf;
        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto sep =
            separator->IsTreeString() ? separator->GetTreeStringDataUtf8(tree8Buf)[0] : separator->GetDataUtf8()[0];
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (auto i = 0U; i < n; i++) {
            strSize += NumberToU8Chars(buf, strSize, data[i]);
            buf[strSize] = sep;
            strSize += 1;
        }
    } else {
        for (auto i = 0U; i < n; i++) {
            strSize += NumberToU8Chars(buf, strSize, data[i]);
            separator->CopyDataMUtf8(&buf[strSize], sepSize, false);
            strSize += sepSize;
        }
    }
    strSize += NumberToU8Chars(buf, strSize, data[n]);
    return EtsString::CreateFromUtf8(buf.data(), strSize);
}

template <typename T>
static ark::ets::EtsString *TypedArrayJoin(T *thisArray, ark::ets::EtsString *separator)
{
    using ElementType = typename T::ElementType;

    auto length = thisArray->GetLengthInt();
    if (UNLIKELY(length <= 0)) {
        return EtsString::CreateNewEmptyString();
    }

    void *dataPtr = GetNativeData(thisArray);
    if (UNLIKELY(dataPtr == nullptr)) {
        return nullptr;
    }

    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *tyDataPtr = reinterpret_cast<ElementType *>(static_cast<EtsByte *>(dataPtr) +
                                                      // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
                                                      static_cast<size_t>(thisArray->GetByteOffset()));
    Span<ElementType> data(tyDataPtr, length);
    // See (GetNativeData) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (separator->IsEmpty()) {
        return TypedArrayJoinUtf8(data);
    }
    // See (GetNativeData) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (!separator->IsUtf16()) {
        // See (GetNativeData) reason
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        return TypedArrayJoinUtf8(data, separator);
    }
    // See (GetNativeData) reason
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    return TypedArrayJoinUtf16(data, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreInt8ArrayJoin(ark::ets::EtsStdCoreInt8Array *thisArray,
                                                        ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreUint8ArrayJoin(ark::ets::EtsStdCoreUInt8Array *thisArray,
                                                         ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreUint8ClampedArrayJoin(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray,
                                                                ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreInt16ArrayJoin(ark::ets::EtsStdCoreInt16Array *thisArray,
                                                         ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreUint16ArrayJoin(ark::ets::EtsStdCoreUInt16Array *thisArray,
                                                          ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreInt32ArrayJoin(ark::ets::EtsStdCoreInt32Array *thisArray,
                                                         ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreUint32ArrayJoin(ark::ets::EtsStdCoreUInt32Array *thisArray,
                                                          ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreBigInt64ArrayJoin(ark::ets::EtsStdCoreBigInt64Array *thisArray,
                                                            ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreBigUint64ArrayJoin(ark::ets::EtsStdCoreBigUInt64Array *thisArray,
                                                             ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreFloat32ArrayJoin(ark::ets::EtsStdCoreFloat32Array *thisArray,
                                                           ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsStdCoreFloat64ArrayJoin(ark::ets::EtsStdCoreFloat64Array *thisArray,
                                                           ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" EtsInt TypedArrayDoubleToInt(EtsDouble val, uint32_t bits = INT32_BITS)
{
    int32_t ret = 0;
    auto u64 = bit_cast<uint64_t>(val);
    int exp = static_cast<int>((u64 & DOUBLE_EXPONENT_MASK) >> DOUBLE_SIGNIFICAND_SIZE) - DOUBLE_EXPONENT_BIAS;
    if (exp < static_cast<int>(bits - 1)) {
        // smaller than INT<bits>_MAX, fast conversion
        ret = static_cast<int32_t>(val);
    } else if (exp < static_cast<int>(bits + DOUBLE_SIGNIFICAND_SIZE)) {
        // Still has significand bits after mod 2^<bits>
        // Get low <bits> bits by shift left <64 - bits> and shift right <64 - bits>
        uint64_t value = (((u64 & DOUBLE_SIGNIFICAND_MASK) | DOUBLE_HIDDEN_BIT)
                          << (static_cast<uint32_t>(exp) - DOUBLE_SIGNIFICAND_SIZE + INT64_BITS - bits)) >>
                         (INT64_BITS - bits);
        ret = static_cast<int32_t>(value);
        if ((u64 & DOUBLE_SIGN_MASK) == DOUBLE_SIGN_MASK && ret != INT32_MIN) {
            ret = -ret;
        }
    } else {
        // No significand bits after mod 2^<bits>, contains NaN and INF
        ret = 0;
    }
    return ret;
}

extern "C" EtsInt TypedArrayDoubleToInt32(EtsDouble val)
{
    return TypedArrayDoubleToInt(val, INT32_BITS);
}

extern "C" EtsShort TypedArrayDoubleToInt16(EtsDouble val)
{
    return TypedArrayDoubleToInt(val, INT16_BITS);
}

extern "C" EtsByte TypedArrayDoubleToInt8(EtsDouble val)
{
    return TypedArrayDoubleToInt(val, INT8_BITS);
}

extern "C" EtsLong TypedArrayDoubleToUint32(EtsDouble val)
{
    // Convert to int32_t first, then cast to uint32_t
    auto ret = TypedArrayDoubleToInt(val, INT32_BITS);
    auto pTmp = reinterpret_cast<uint32_t *>(&ret);
    return static_cast<EtsLong>(static_cast<uint32_t>(*pTmp));
}

extern "C" EtsInt TypedArrayDoubleToUint16(EtsDouble val)
{
    // Convert to int16_t first, then cast to uint16_t
    auto ret = TypedArrayDoubleToInt(val, INT16_BITS);
    auto pTmp = reinterpret_cast<int16_t *>(&ret);
    return static_cast<EtsInt>(static_cast<uint16_t>(*pTmp));
}

extern "C" EtsInt TypedArrayDoubleToUint8(EtsDouble val)
{
    // Convert to int8_t first, then cast to uint8_t
    auto ret = TypedArrayDoubleToInt(val, 8);
    auto pTmp = reinterpret_cast<uint8_t *>(&ret);
    return static_cast<EtsInt>(reinterpret_cast<uint8_t>(*pTmp));
}

template <typename T>
static T CastNumberHelper(double val) = delete;

template <typename T>
static T CastNumber(double val)
{
    if (std::isinf(val)) {
        return 0;
    }
    if (std::isnan(val)) {
        return 0;
    }
    return CastNumberHelper<T>(val);
}

template <>
EtsByte CastNumberHelper(double val)
{
    return TypedArrayDoubleToInt8(val);
}

template <>
EtsShort CastNumberHelper(double val)
{
    return TypedArrayDoubleToInt16(val);
}

template <>
EtsInt CastNumberHelper(double val)
{
    return TypedArrayDoubleToInt32(val);
}

template <>
EtsLong CastNumberHelper(double val)
{
    return StdCoreDoubleToLong(val);
}

template <typename T>
static T CastNumberUnsignedHelper(double val) = delete;

template <typename T>
static T CastNumberUnsigned(double val)
{
    if (std::isinf(val)) {
        return 0;
    }
    return CastNumberUnsignedHelper<T>(val);
}

template <>
uint8_t CastNumberUnsignedHelper(double val)
{
    auto ret = TypedArrayDoubleToInt(val, INT8_BITS);
    auto pTmp = reinterpret_cast<uint8_t *>(&ret);
    return reinterpret_cast<uint8_t>(*pTmp);
}

template <>
uint16_t CastNumberUnsignedHelper(double val)
{
    auto ret = TypedArrayDoubleToInt(val, INT16_BITS);
    auto pTmp = reinterpret_cast<uint16_t *>(&ret);
    return static_cast<uint16_t>(*pTmp);
}

template <>
uint32_t CastNumberUnsignedHelper(double val)
{
    auto ret = TypedArrayDoubleToInt(val, INT32_BITS);
    auto pTmp = reinterpret_cast<uint32_t *>(&ret);
    return static_cast<uint32_t>(*pTmp);
}

template <typename T1, typename T2>
static void EtsCoreTypedArrayOfImpl(T1 *thisArray, T2 *src)
{
    auto *arrayPtr = GetNativeData(thisArray);
    if (UNLIKELY(arrayPtr == nullptr)) {
        return;
    }

    using ElementType = typename T1::ElementType;
    auto *dst = reinterpret_cast<ElementType *>(ToUintPtr(arrayPtr) + static_cast<int>(thisArray->GetByteOffset()));
    std::copy_n(src, thisArray->GetLengthInt(), dst);
}

template <typename T1, typename T2>
static void EtsCoreTypedArrayOfImplNumber(T1 *thisArray, T2 *src)
{
    auto *arrayPtr = GetNativeData(thisArray);
    if (UNLIKELY(arrayPtr == nullptr)) {
        return;
    }

    using ElementType = typename T1::ElementType;
    auto *dst = reinterpret_cast<ElementType *>(ToUintPtr(arrayPtr) + static_cast<int>(thisArray->GetByteOffset()));
    for (int i = 0; i < thisArray->GetLengthInt(); i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        dst[i] = CastNumber<ElementType>(src[i]);
    }
}

template <typename T1, typename T2>
static void EtsCoreTypedArrayOfImplNumberUnsigned(T1 *thisArray, T2 *src)
{
    auto *arrayPtr = GetNativeData(thisArray);
    if (UNLIKELY(arrayPtr == nullptr)) {
        return;
    }

    using ElementType = typename T1::ElementType;
    auto *dst = reinterpret_cast<ElementType *>(ToUintPtr(arrayPtr) + static_cast<int>(thisArray->GetByteOffset()));
    for (int i = 0; i < thisArray->GetLengthInt(); i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        dst[i] = CastNumberUnsigned<ElementType>(src[i]);
    }
}

static int Clamp(double val)
{
    const int minUint8 = 0;
    const int maxUint8 = 255;
    if (std::isnan(val)) {
        return minUint8;
    }
    if (val > maxUint8) {
        val = maxUint8;
    } else if (val < minUint8) {
        val = minUint8;
    }
    return val;
}

template <typename T1, typename T2>
static void EtsCoreTypedArrayOfImplClamped(T1 *thisArray, T2 *src)
{
    auto *arrayPtr = GetNativeData(thisArray);
    if (UNLIKELY(arrayPtr == nullptr)) {
        return;
    }

    using ElementType = typename T1::ElementType;
    auto *dst = reinterpret_cast<ElementType *>(ToUintPtr(arrayPtr) + static_cast<int>(thisArray->GetByteOffset()));
    for (int i = 0; i < thisArray->GetLengthInt(); i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        dst[i] = Clamp(src[i]);
    }
}

extern "C" void EtsStdCoreInt8ArrayOfInt(ark::ets::EtsStdCoreInt8Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsInt *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreInt8ArrayOfNumber(ark::ets::EtsStdCoreInt8Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsDouble *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImplNumber(thisArray, srcAddress);
}

extern "C" void EtsStdCoreInt8ArrayOfByte(ark::ets::EtsStdCoreInt8Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsByte *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreInt16ArrayOfInt(ark::ets::EtsStdCoreInt16Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsInt *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreInt16ArrayOfNumber(ark::ets::EtsStdCoreInt16Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsDouble *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImplNumber(thisArray, srcAddress);
}

extern "C" void EtsStdCoreInt16ArrayOfShort(ark::ets::EtsStdCoreInt16Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsShort *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreInt32ArrayOfInt(ark::ets::EtsStdCoreInt32Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsInt *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreInt32ArrayOfNumber(ark::ets::EtsStdCoreInt32Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsDouble *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImplNumber(thisArray, srcAddress);
}

extern "C" void EtsStdCoreBigInt64ArrayOfInt(ark::ets::EtsStdCoreBigInt64Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsInt *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

template <typename T, typename ToLong>
void EtsCoreTypedArrayOfBigIntImpl(T *thisArray, EtsTypedObjectArray<EtsBigInt> *src, ToLong toLong)
{
    auto *arrayPtr = GetNativeData(thisArray);
    if (UNLIKELY(arrayPtr == nullptr)) {
        return;
    }

    using ElementType = typename T::ElementType;
    auto *dst = reinterpret_cast<ElementType *>(ToUintPtr(arrayPtr) + static_cast<int>(thisArray->GetByteOffset()));
    ASSERT(thisArray->GetLengthInt() >= 0 && static_cast<size_t>(thisArray->GetLengthInt()) >= src->GetLength());
    std::generate_n(dst, src->GetLength(), [idx = 0, src, toLong]() mutable { return toLong(src->Get(idx++)); });
}

extern "C" void EtsStdCoreBigInt64ArrayOfBigInt(ark::ets::EtsStdCoreBigInt64Array *thisArray, ark::ObjectHeader *src)
{
    // The method fills the typed array from a FixedArray<bigint> and in contrast to
    // EtsStdCoreBigInt64ArraySetValuesFromArray, which fills a typed array from std.core.Array<bigint>, we can be
    // sure that `src` is an instance of the EtsTypedObjectArray<EtsBigInt> class.
    EtsCoreTypedArrayOfBigIntImpl(thisArray, EtsTypedObjectArray<EtsBigInt>::FromCoreType(src),
                                  [](EtsBigInt *bigint) { return unbox::GetLong(bigint); });
}

extern "C" void EtsStdCoreBigInt64ArrayOfLong(ark::ets::EtsStdCoreBigInt64Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsLong *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreBigInt64ArrayOfNumber(ark::ets::EtsStdCoreBigInt64Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsDouble *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImplNumber(thisArray, srcAddress);
}

extern "C" void EtsStdCoreFloat32ArrayOfInt(ark::ets::EtsStdCoreFloat32Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsInt *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreFloat32ArrayOfFloat(ark::ets::EtsStdCoreFloat32Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsFloat *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreFloat32ArrayOfNumber(ark::ets::EtsStdCoreFloat32Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsDouble *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreFloat64ArrayOfInt(ark::ets::EtsStdCoreFloat64Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsInt *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreFloat64ArrayOfNumber(ark::ets::EtsStdCoreFloat64Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsDouble *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint8ClampedArrayOfNumber(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsDouble *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImplClamped(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint8ClampedArrayOfInt(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsInt *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImplClamped(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint8ClampedArrayOfShort(ark::ets::EtsStdCoreUInt8ClampedArray *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsShort *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImplClamped(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint8ArrayOfNumber(ark::ets::EtsStdCoreUInt8Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsDouble *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImplNumberUnsigned(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint8ArrayOfInt(ark::ets::EtsStdCoreUInt8Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsInt *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint8ArrayOfShort(ark::ets::EtsStdCoreUInt8Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsShort *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint16ArrayOfNumber(ark::ets::EtsStdCoreUInt16Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsDouble *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImplNumberUnsigned(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint16ArrayOfInt(ark::ets::EtsStdCoreUInt16Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsInt *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint32ArrayOfNumber(ark::ets::EtsStdCoreUInt32Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsDouble *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImplNumberUnsigned(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint32ArrayOfInt(ark::ets::EtsStdCoreUInt32Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsUint *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreUint32ArrayOfLong(ark::ets::EtsStdCoreUInt32Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsLong *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreBigUint64ArrayOfInt(ark::ets::EtsStdCoreBigUInt64Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsInt *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreBigUint64ArrayOfLong(ark::ets::EtsStdCoreBigUInt64Array *thisArray, EtsCharArray *src)
{
    auto *srcAddress = reinterpret_cast<EtsLong *>(ToUintPtr(src->GetCoreType()->GetData()));
    EtsCoreTypedArrayOfImpl(thisArray, srcAddress);
}

extern "C" void EtsStdCoreBigUint64ArrayOfBigInt(ark::ets::EtsStdCoreBigUInt64Array *thisArray, ark::ObjectHeader *src)
{
    // The method fills the typed array from a FixedArray<bigint> and in contrast to
    // EtsStdCoreBigUint64ArraySetValuesFromArray, which fills a typed array from std.core.Array<bigint>, we can be
    // sure that `src` is an instance of the EtsTypedObjectArray<EtsBigInt> class.
    EtsCoreTypedArrayOfBigIntImpl(thisArray, EtsTypedObjectArray<EtsBigInt>::FromCoreType(src),
                                  [](EtsBigInt *bigint) { return unbox::GetULong(bigint); });
}
}  // namespace ark::ets::intrinsics
