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

#include "plugins/ets/runtime/types/ets_typed_arrays.h"
#include "plugins/ets/runtime/types/ets_typed_unsigned_arrays.h"
#include "intrinsics.h"
#include "cross_values.h"

namespace ark::ets::intrinsics {

template <typename T>
static void *GetNativeData(T *array)
{
    auto *arrayBuffer = static_cast<EtsEscompatArrayBuffer *>(&*array->GetBuffer());
    if (UNLIKELY(arrayBuffer->WasDetached())) {
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        ThrowEtsException(coro, panda_file_items::class_descriptors::TYPE_ERROR, "ArrayBuffer was detached");
        return nullptr;
    }

    return arrayBuffer->GetData();
}

template <typename T>
static void EtsEscompatTypedArraySet(T *thisArray, EtsInt pos, typename T::ElementType val)
{
    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return;
    }

    if (UNLIKELY(pos < 0 || pos >= thisArray->GetLengthInt())) {
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        ThrowEtsException(coro, panda_file_items::class_descriptors::RANGE_ERROR, "invalid index");
        return;
    }
    ObjectAccessor::SetPrimitive(
        data, pos * sizeof(typename T::ElementType) + static_cast<EtsInt>(thisArray->GetByteOffset()), val);
}

template <typename T>
typename T::ElementType EtsEscompatTypedArrayGet(T *thisArray, EtsInt pos)
{
    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return 0;
    }

    if (UNLIKELY(pos < 0 || pos >= thisArray->GetLengthInt())) {
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        ThrowEtsException(coro, panda_file_items::class_descriptors::RANGE_ERROR, "invalid index");
        return 0;
    }
    return ObjectAccessor::GetPrimitive<typename T::ElementType>(
        data, pos * sizeof(typename T::ElementType) + static_cast<EtsInt>(thisArray->GetByteOffset()));
}

extern "C" void EtsEscompatInt8ArraySetInt(ark::ets::EtsEscompatInt8Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" void EtsEscompatInt8ArraySetByte(ark::ets::EtsEscompatInt8Array *thisArray, EtsInt pos, EtsByte val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatInt8ArrayGet(ark::ets::EtsEscompatInt8Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

template <typename T>
static void EtsEscompatTypedArraySetValuesImpl(T *thisArray, T *srcArray, EtsInt pos)
{
    auto *dstData = GetNativeData(thisArray);
    if (UNLIKELY(dstData == nullptr)) {
        return;
    }
    auto *srcData = GetNativeData(srcArray);
    if (UNLIKELY(srcData == nullptr)) {
        return;
    }

    using ElementType = typename T::ElementType;
    if (UNLIKELY(pos < 0 || pos + srcArray->GetLengthInt() > thisArray->GetLengthInt())) {
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        ThrowEtsException(coro, panda_file_items::class_descriptors::RANGE_ERROR, "offset is out of bounds");
        return;
    }
    auto *dst = ToVoidPtr(ToUintPtr(dstData) + thisArray->GetByteOffset() + pos * sizeof(ElementType));
    auto *src = ToVoidPtr(ToUintPtr(srcData) + srcArray->GetByteOffset());
    [[maybe_unused]] auto error = memmove_s(dst, (thisArray->GetLengthInt() - pos) * sizeof(ElementType), src,
                                            srcArray->GetLengthInt() * sizeof(ElementType));
    ASSERT(error == EOK);
}

extern "C" void EtsEscompatInt8ArraySetValues(ark::ets::EtsEscompatInt8Array *thisArray,
                                              ark::ets::EtsEscompatInt8Array *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatInt8ArraySetValuesWithOffset(ark::ets::EtsEscompatInt8Array *thisArray,
                                                        ark::ets::EtsEscompatInt8Array *srcArray, EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

extern "C" void EtsEscompatInt16ArraySetInt(ark::ets::EtsEscompatInt16Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" void EtsEscompatInt16ArraySetShort(ark::ets::EtsEscompatInt16Array *thisArray, EtsInt pos, EtsShort val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatInt16ArrayGet(ark::ets::EtsEscompatInt16Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" void EtsEscompatInt16ArraySetValues(ark::ets::EtsEscompatInt16Array *thisArray,
                                               ark::ets::EtsEscompatInt16Array *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatInt16ArraySetValuesWithOffset(ark::ets::EtsEscompatInt16Array *thisArray,
                                                         ark::ets::EtsEscompatInt16Array *srcArray, EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

extern "C" void EtsEscompatInt32ArraySetInt(ark::ets::EtsEscompatInt32Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatInt32ArrayGet(ark::ets::EtsEscompatInt32Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" void EtsEscompatInt32ArraySetValues(ark::ets::EtsEscompatInt32Array *thisArray,
                                               ark::ets::EtsEscompatInt32Array *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatInt32ArraySetValuesWithOffset(ark::ets::EtsEscompatInt32Array *thisArray,
                                                         ark::ets::EtsEscompatInt32Array *srcArray, EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

extern "C" void EtsEscompatBigInt64ArraySetLong(ark::ets::EtsEscompatBigInt64Array *thisArray, EtsInt pos, EtsLong val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsLong EtsEscompatBigInt64ArrayGet(ark::ets::EtsEscompatBigInt64Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" void EtsEscompatBigInt64ArraySetValues(ark::ets::EtsEscompatBigInt64Array *thisArray,
                                                  ark::ets::EtsEscompatBigInt64Array *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatBigInt64ArraySetValuesWithOffset(ark::ets::EtsEscompatBigInt64Array *thisArray,
                                                            ark::ets::EtsEscompatBigInt64Array *srcArray, EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

extern "C" void EtsEscompatFloat32ArraySetFloat(ark::ets::EtsEscompatFloat32Array *thisArray, EtsInt pos, EtsFloat val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatFloat32ArrayGet(ark::ets::EtsEscompatFloat32Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" void EtsEscompatFloat32ArraySetValues(ark::ets::EtsEscompatFloat32Array *thisArray,
                                                 ark::ets::EtsEscompatFloat32Array *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatFloat32ArraySetValuesWithOffset(ark::ets::EtsEscompatFloat32Array *thisArray,
                                                           ark::ets::EtsEscompatFloat32Array *srcArray, EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

extern "C" void EtsEscompatFloat64ArraySetDouble(ark::ets::EtsEscompatFloat64Array *thisArray, EtsInt pos,
                                                 EtsDouble val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatFloat64ArrayGet(ark::ets::EtsEscompatFloat64Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" void EtsEscompatFloat64ArraySetValues(ark::ets::EtsEscompatFloat64Array *thisArray,
                                                 ark::ets::EtsEscompatFloat64Array *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatFloat64ArraySetValuesWithOffset(ark::ets::EtsEscompatFloat64Array *thisArray,
                                                           ark::ets::EtsEscompatFloat64Array *srcArray, EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

extern "C" void EtsEscompatUInt8ClampedArraySetInt(ark::ets::EtsEscompatUInt8ClampedArray *thisArray, EtsInt pos,
                                                   EtsInt val)
{
    if (val > ark::ets::EtsEscompatUInt8ClampedArray::MAX) {
        val = ark::ets::EtsEscompatUInt8ClampedArray::MAX;
    } else if (val < ark::ets::EtsEscompatUInt8ClampedArray::MIN) {
        val = ark::ets::EtsEscompatUInt8ClampedArray::MIN;
    }
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatUInt8ClampedArrayGet(ark::ets::EtsEscompatUInt8ClampedArray *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" void EtsEscompatUInt8ClampedArraySetValues(ark::ets::EtsEscompatUInt8ClampedArray *thisArray,
                                                      ark::ets::EtsEscompatUInt8ClampedArray *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatUInt8ClampedArraySetValuesWithOffset(ark::ets::EtsEscompatUInt8ClampedArray *thisArray,
                                                                ark::ets::EtsEscompatUInt8ClampedArray *srcArray,
                                                                EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

extern "C" void EtsEscompatUInt8ArraySetInt(ark::ets::EtsEscompatUInt8Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatUInt8ArrayGet(ark::ets::EtsEscompatUInt8Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" void EtsEscompatUInt8ArraySetValues(ark::ets::EtsEscompatUInt8Array *thisArray,
                                               ark::ets::EtsEscompatUInt8Array *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatUInt8ArraySetValuesWithOffset(ark::ets::EtsEscompatUInt8Array *thisArray,
                                                         ark::ets::EtsEscompatUInt8Array *srcArray, EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

extern "C" void EtsEscompatUInt16ArraySetInt(ark::ets::EtsEscompatUInt16Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatUInt16ArrayGet(ark::ets::EtsEscompatUInt16Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" void EtsEscompatUInt16ArraySetValues(ark::ets::EtsEscompatUInt16Array *thisArray,
                                                ark::ets::EtsEscompatUInt16Array *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatUInt16ArraySetValuesWithOffset(ark::ets::EtsEscompatUInt16Array *thisArray,
                                                          ark::ets::EtsEscompatUInt16Array *srcArray, EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

extern "C" void EtsEscompatUInt32ArraySetInt(ark::ets::EtsEscompatUInt32Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" void EtsEscompatUInt32ArraySetLong(ark::ets::EtsEscompatUInt32Array *thisArray, EtsInt pos, EtsLong val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatUInt32ArrayGet(ark::ets::EtsEscompatUInt32Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" void EtsEscompatUInt32ArraySetValues(ark::ets::EtsEscompatUInt32Array *thisArray,
                                                ark::ets::EtsEscompatUInt32Array *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatUInt32ArraySetValuesWithOffset(ark::ets::EtsEscompatUInt32Array *thisArray,
                                                          ark::ets::EtsEscompatUInt32Array *srcArray, EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

extern "C" void EtsEscompatBigUInt64ArraySetInt(ark::ets::EtsEscompatBigUInt64Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" void EtsEscompatBigUInt64ArraySetLong(ark::ets::EtsEscompatBigUInt64Array *thisArray, EtsInt pos,
                                                 EtsLong val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsLong EtsEscompatBigUInt64ArrayGet(ark::ets::EtsEscompatBigUInt64Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" void EtsEscompatBigUInt64ArraySetValues(ark::ets::EtsEscompatBigUInt64Array *thisArray,
                                                   ark::ets::EtsEscompatBigUInt64Array *srcArray)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, 0);
}

extern "C" void EtsEscompatBigUInt64ArraySetValuesWithOffset(ark::ets::EtsEscompatBigUInt64Array *thisArray,
                                                             ark::ets::EtsEscompatBigUInt64Array *srcArray,
                                                             EtsDouble pos)
{
    EtsEscompatTypedArraySetValuesImpl(thisArray, srcArray, static_cast<EtsInt>(pos));
}

/*
 * Typed Arrays: Reverse Implementation
 */
template <typename T>
static T *EtsEscompatTypedArrayReverseImpl(T *thisArray)
{
    auto *arrData = GetNativeData(thisArray);
    if (UNLIKELY(arrData == nullptr)) {
        return nullptr;
    }
    using ElementType = typename T::ElementType;
    auto *ptrFirst = reinterpret_cast<ElementType *>(ToVoidPtr(ToUintPtr(arrData) + thisArray->GetByteOffset()));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto *ptrLast = ptrFirst + thisArray->GetLengthInt();
    std::reverse(ptrFirst, ptrLast);
    return thisArray;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPED_ARRAY_REVERSE_CALL_DECL(Type)                                         \
    /* CC-OFFNXT(G.PRE.02) name part */                                             \
    extern "C" ark::ets::EtsEscompat##Type##Array *EtsEscompat##Type##ArrayReverse( \
        ark::ets::EtsEscompat##Type##Array *thisArray)                              \
    {                                                                               \
        /* CC-OFFNXT(G.PRE.05) function gen */                                      \
        return EtsEscompatTypedArrayReverseImpl(thisArray);                         \
    }  // namespace ark::ets::intrinsics

TYPED_ARRAY_REVERSE_CALL_DECL(Int8)
TYPED_ARRAY_REVERSE_CALL_DECL(Int16)
TYPED_ARRAY_REVERSE_CALL_DECL(Int32)
TYPED_ARRAY_REVERSE_CALL_DECL(BigInt64)
TYPED_ARRAY_REVERSE_CALL_DECL(Float32)
TYPED_ARRAY_REVERSE_CALL_DECL(Float64)

TYPED_ARRAY_REVERSE_CALL_DECL(UInt8)
TYPED_ARRAY_REVERSE_CALL_DECL(UInt8Clamped)
TYPED_ARRAY_REVERSE_CALL_DECL(UInt16)
TYPED_ARRAY_REVERSE_CALL_DECL(UInt32)
TYPED_ARRAY_REVERSE_CALL_DECL(BigUInt64)

int32_t NormalizeIndex(int32_t idx, int32_t arrayLength)
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
T *EtsEscompatTypedArrayCopyWithinImpl(T *thisArray, EtsInt target, EtsInt start, EtsInt end)
{
    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
        return nullptr;
    }

    auto arrayLength = thisArray->GetLengthInt();
    target = NormalizeIndex(target, arrayLength);
    start = NormalizeIndex(start, arrayLength);
    end = NormalizeIndex(end, arrayLength);

    int32_t count = end - start;
    if (count > (arrayLength - target)) {
        count = arrayLength - target;
    }
    if (count <= 0) {
        return thisArray;
    }

    using ElementType = typename T::ElementType;
    auto *targetAddress = ToVoidPtr(ToUintPtr(data) + thisArray->GetByteOffset() + target * sizeof(ElementType));
    auto *startAddress = ToVoidPtr(ToUintPtr(data) + thisArray->GetByteOffset() + start * sizeof(ElementType));
    [[maybe_unused]] auto error = memmove_s(targetAddress, (arrayLength - start) * sizeof(ElementType), startAddress,
                                            count * sizeof(ElementType));
    ASSERT(error == EOK);
    return thisArray;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_ESCOMPAT_COPY_WITHIN(Type)                                                          \
    /* CC-OFFNXT(G.PRE.02) name part */                                                         \
    extern "C" ark::ets::EtsEscompat##Type##Array *EtsEscompat##Type##ArrayCopyWithin(          \
        ark::ets::EtsEscompat##Type##Array *thisArray, EtsInt target, EtsInt start, EtsInt end) \
    {                                                                                           \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                  \
        return EtsEscompatTypedArrayCopyWithinImpl(thisArray, target, start, end);              \
    }  // namespace ark::ets::intrinsics

ETS_ESCOMPAT_COPY_WITHIN(Int8)
ETS_ESCOMPAT_COPY_WITHIN(Int16)
ETS_ESCOMPAT_COPY_WITHIN(Int32)
ETS_ESCOMPAT_COPY_WITHIN(BigInt64)
ETS_ESCOMPAT_COPY_WITHIN(Float32)
ETS_ESCOMPAT_COPY_WITHIN(Float64)
ETS_ESCOMPAT_COPY_WITHIN(UInt8)
ETS_ESCOMPAT_COPY_WITHIN(UInt16)
ETS_ESCOMPAT_COPY_WITHIN(UInt32)
ETS_ESCOMPAT_COPY_WITHIN(BigUInt64)
ETS_ESCOMPAT_COPY_WITHIN(UInt8Clamped)

#undef ETS_ESCOMPAT_COPY_WITHIN

template <typename T>
T *EtsEscompatTypedArraySort(T *thisArray)
{
    using ElementType = typename T::ElementType;

    auto nBytes = static_cast<size_t>(thisArray->GetByteLength());
    ASSERT(nBytes == (static_cast<size_t>(thisArray->GetLengthInt()) * sizeof(ElementType)));
    // If it's an empty or one-element array then nothing to sort.
    if (UNLIKELY(nBytes <= sizeof(ElementType))) {
        return thisArray;
    }

    auto *arrayBuffer = static_cast<EtsEscompatArrayBuffer *>(&*thisArray->GetBuffer());
    if (UNLIKELY(arrayBuffer->WasDetached())) {
        auto coro = EtsCoroutine::GetCurrent();
        [[maybe_unused]] EtsHandleScope scope(coro);
        EtsHandle<EtsObject> handle(coro, thisArray);
        ThrowEtsException(coro, panda_file_items::class_descriptors::TYPE_ERROR, "ArrayBuffer was detached");
        return static_cast<T *>(handle.GetPtr());
    }
    void *dataPtr = arrayBuffer->GetData();
    ASSERT(dataPtr != nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    Span<EtsByte> data(static_cast<EtsByte *>(dataPtr) + static_cast<size_t>(thisArray->GetByteOffset()), nBytes);
    std::sort(reinterpret_cast<typename T::ElementType *>(data.begin()),
              reinterpret_cast<typename T::ElementType *>(data.end()));
    return thisArray;
}

extern "C" ark::ets::EtsEscompatInt8Array *EtsEscompatInt8ArraySort(ark::ets::EtsEscompatInt8Array *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsEscompatInt16Array *EtsEscompatInt16ArraySort(ark::ets::EtsEscompatInt16Array *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsEscompatInt32Array *EtsEscompatInt32ArraySort(ark::ets::EtsEscompatInt32Array *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsEscompatBigInt64Array *EtsEscompatBigInt64ArraySort(
    ark::ets::EtsEscompatBigInt64Array *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsEscompatFloat32Array *EtsEscompatFloat32ArraySort(ark::ets::EtsEscompatFloat32Array *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsEscompatFloat64Array *EtsEscompatFloat64ArraySort(ark::ets::EtsEscompatFloat64Array *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsEscompatUInt8ClampedArray *EtsEscompatUInt8ClampedArraySort(
    ark::ets::EtsEscompatUInt8ClampedArray *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsEscompatUInt8Array *EtsEscompatUInt8ArraySort(ark::ets::EtsEscompatUInt8Array *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsEscompatUInt16Array *EtsEscompatUInt16ArraySort(ark::ets::EtsEscompatUInt16Array *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsEscompatUInt32Array *EtsEscompatUInt32ArraySort(ark::ets::EtsEscompatUInt32Array *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

extern "C" ark::ets::EtsEscompatBigUInt64Array *EtsEscompatBigUInt64ArraySort(
    ark::ets::EtsEscompatBigUInt64Array *thisArray)
{
    return EtsEscompatTypedArraySort(thisArray);
}

}  // namespace ark::ets::intrinsics
