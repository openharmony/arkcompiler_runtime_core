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

#include <charconv>
#include <type_traits>
#include "ets_handle.h"
#include "libpandabase/utils/utf.h"
#include "libpandabase/utils/utils.h"
#include "plugins/ets/runtime/types/ets_typed_arrays.h"
#include "plugins/ets/runtime/types/ets_typed_unsigned_arrays.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include "intrinsics.h"
#include "cross_values.h"
#include "types/ets_object.h"

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
    auto *arrayBuffer = static_cast<EtsEscompatArrayBuffer *>(&*array->GetBuffer());
    if (UNLIKELY(arrayBuffer->WasDetached())) {
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        ThrowEtsException(coro, panda_file_items::class_descriptors::TYPE_ERROR, "ArrayBuffer was detached");
        return nullptr;
    }

    return arrayBuffer->GetData();
}

static EtsHandle<EtsEscompatArrayBuffer> CreateArrayBuffer(EtsCoroutine *coro, EtsInt byteLength,
                                                           const uint8_t *data = nullptr)
{
    void *buffer = nullptr;
    EtsHandle<EtsEscompatArrayBuffer> newBuffer(coro, EtsEscompatArrayBuffer::Create(coro, byteLength, &buffer));
    if (UNLIKELY(newBuffer.GetPtr() == nullptr)) {
        return newBuffer;
    }
    if (data != nullptr && byteLength > 0) {
        std::copy_n(data, byteLength, reinterpret_cast<uint8_t *>(buffer));
    }
    return newBuffer;
}

template <typename T>
static void EtsEscompatTypedArraySet(T *thisArray, EtsInt pos, typename T::ElementType val)
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
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        ThrowEtsException(coro, panda_file_items::class_descriptors::RANGE_ERROR, "invalid index");
        return;
    }
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    ObjectAccessor::SetPrimitive(
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        data, pos * sizeof(typename T::ElementType) + static_cast<EtsInt>(thisArray->GetByteOffset()), val);
}

template <typename T>
typename T::ElementType EtsEscompatTypedArrayGet(T *thisArray, EtsInt pos)
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
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        ThrowEtsException(coro, panda_file_items::class_descriptors::RANGE_ERROR, "invalid index");
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
typename T::ElementType EtsEscompatTypedArrayGetUnsafe(T *thisArray, EtsInt pos)
{
    ASSERT(pos >= 0 && pos < thisArray->GetLengthInt());

    auto *data = GetNativeData(thisArray);
    if (UNLIKELY(data == nullptr)) {
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

extern "C" EtsByte EtsEscompatInt8ArrayGetUnsafe(ark::ets::EtsEscompatInt8Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
}

template <typename T>
static void EtsEscompatTypedArraySetValuesImpl(T *thisArray, T *srcArray, EtsInt pos)
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
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        ThrowEtsException(coro, panda_file_items::class_descriptors::RANGE_ERROR, "offset is out of bounds");
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

template <typename T, typename V>
static void EtsEscompatTypedArrayFillInternal(T *thisArray, V val, EtsInt begin, EtsInt end)
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

extern "C" void EtsEscompatInt8ArrayFillInternal(ark::ets::EtsEscompatInt8Array *thisArray, EtsByte val, EtsInt begin,
                                                 EtsInt end)
{
    EtsEscompatTypedArrayFillInternal(thisArray, val, begin, end);
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

extern "C" EtsShort EtsEscompatInt16ArrayGetUnsafe(ark::ets::EtsEscompatInt16Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
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

extern "C" void EtsEscompatInt16ArrayFillInternal(ark::ets::EtsEscompatInt16Array *thisArray, EtsShort val,
                                                  EtsInt begin, EtsInt end)
{
    EtsEscompatTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsEscompatInt32ArraySetInt(ark::ets::EtsEscompatInt32Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatInt32ArrayGet(ark::ets::EtsEscompatInt32Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" EtsInt EtsEscompatInt32ArrayGetUnsafe(ark::ets::EtsEscompatInt32Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
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

extern "C" void EtsEscompatInt32ArrayFillInternal(ark::ets::EtsEscompatInt32Array *thisArray, EtsInt val, EtsInt begin,
                                                  EtsInt end)
{
    EtsEscompatTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsEscompatBigInt64ArraySetLong(ark::ets::EtsEscompatBigInt64Array *thisArray, EtsInt pos, EtsLong val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsLong EtsEscompatBigInt64ArrayGet(ark::ets::EtsEscompatBigInt64Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" EtsLong EtsEscompatBigInt64ArrayGetUnsafe(ark::ets::EtsEscompatBigInt64Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
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

extern "C" void EtsEscompatBigInt64ArrayFillInternal(ark::ets::EtsEscompatBigInt64Array *thisArray, EtsLong val,
                                                     EtsInt begin, EtsInt end)
{
    EtsEscompatTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsEscompatFloat32ArraySetFloat(ark::ets::EtsEscompatFloat32Array *thisArray, EtsInt pos, EtsFloat val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatFloat32ArrayGet(ark::ets::EtsEscompatFloat32Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" EtsFloat EtsEscompatFloat32ArrayGetUnsafe(ark::ets::EtsEscompatFloat32Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
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

extern "C" void EtsEscompatFloat32ArrayFillInternal(ark::ets::EtsEscompatFloat32Array *thisArray, EtsFloat val,
                                                    EtsInt begin, EtsInt end)
{
    EtsEscompatTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsEscompatFloat32ArrayFillInternalInt(ark::ets::EtsEscompatFloat32Array *thisArray, int32_t val,
                                                       EtsInt begin, EtsInt end)
{
    EtsEscompatTypedArrayFillInternal(thisArray, val, begin, end);
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

extern "C" EtsDouble EtsEscompatFloat64ArrayGetUnsafe(ark::ets::EtsEscompatFloat64Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
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

extern "C" void EtsEscompatFloat64ArrayFillInternal(ark::ets::EtsEscompatFloat64Array *thisArray, EtsDouble val,
                                                    EtsInt begin, EtsInt end)
{
    EtsEscompatTypedArrayFillInternal(thisArray, val, begin, end);
}

extern "C" void EtsEscompatFloat64ArrayFillInternalInt(ark::ets::EtsEscompatFloat64Array *thisArray, int64_t val,
                                                       EtsInt begin, EtsInt end)
{
    EtsEscompatTypedArrayFillInternal(thisArray, val, begin, end);
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

extern "C" EtsInt EtsEscompatUInt8ClampedArrayGetUnsafe(ark::ets::EtsEscompatUInt8ClampedArray *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
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

extern "C" void EtsEscompatUInt8ClampedArrayFillInternal(ark::ets::EtsEscompatUInt8ClampedArray *thisArray, EtsInt val,
                                                         EtsInt begin, EtsInt end)
{
    using ElementType = ark::ets::EtsEscompatUInt8ClampedArray::ElementType;
    EtsEscompatTypedArrayFillInternal(thisArray, static_cast<ElementType>(val), begin, end);
}

extern "C" void EtsEscompatUInt8ArraySetInt(ark::ets::EtsEscompatUInt8Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatUInt8ArrayGet(ark::ets::EtsEscompatUInt8Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" EtsInt EtsEscompatUInt8ArrayGetUnsafe(ark::ets::EtsEscompatUInt8Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
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

extern "C" void EtsEscompatUInt8ArrayFillInternal(ark::ets::EtsEscompatUInt8Array *thisArray, EtsInt val, EtsInt begin,
                                                  EtsInt end)
{
    using ElementType = ark::ets::EtsEscompatUInt8Array::ElementType;
    EtsEscompatTypedArrayFillInternal(thisArray, static_cast<ElementType>(val), begin, end);
}

extern "C" void EtsEscompatUInt16ArraySetInt(ark::ets::EtsEscompatUInt16Array *thisArray, EtsInt pos, EtsInt val)
{
    EtsEscompatTypedArraySet(thisArray, pos, val);
}

extern "C" EtsDouble EtsEscompatUInt16ArrayGet(ark::ets::EtsEscompatUInt16Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGet(thisArray, pos);
}

extern "C" EtsInt EtsEscompatUInt16ArrayGetUnsafe(ark::ets::EtsEscompatUInt16Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
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

extern "C" void EtsEscompatUInt16ArrayFillInternal(ark::ets::EtsEscompatUInt16Array *thisArray, EtsInt val,
                                                   EtsInt begin, EtsInt end)
{
    using ElementType = ark::ets::EtsEscompatUInt16Array::ElementType;
    EtsEscompatTypedArrayFillInternal(thisArray, static_cast<ElementType>(val), begin, end);
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

extern "C" EtsLong EtsEscompatUInt32ArrayGetUnsafe(ark::ets::EtsEscompatUInt32Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
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

extern "C" void EtsEscompatUInt32ArrayFillInternal(ark::ets::EtsEscompatUInt32Array *thisArray, EtsLong val,
                                                   EtsInt begin, EtsInt end)
{
    using ElementType = ark::ets::EtsEscompatUInt32Array::ElementType;
    EtsEscompatTypedArrayFillInternal(thisArray, static_cast<ElementType>(val), begin, end);
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

extern "C" EtsLong EtsEscompatBigUInt64ArrayGetUnsafe(ark::ets::EtsEscompatBigUInt64Array *thisArray, EtsInt pos)
{
    return EtsEscompatTypedArrayGetUnsafe(thisArray, pos);
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

extern "C" void EtsEscompatBigUInt64ArrayFillInternal(ark::ets::EtsEscompatBigUInt64Array *thisArray, EtsLong val,
                                                      EtsInt begin, EtsInt end)
{
    EtsEscompatTypedArrayFillInternal(thisArray, val, begin, end);
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
#define REVERSE_CALL_DECL(Type)                                                     \
    /* CC-OFFNXT(G.PRE.02) name part */                                             \
    extern "C" ark::ets::EtsEscompat##Type##Array *EtsEscompat##Type##ArrayReverse( \
        ark::ets::EtsEscompat##Type##Array *thisArray)                              \
    {                                                                               \
        /* CC-OFFNXT(G.PRE.05) function gen */                                      \
        return EtsEscompatTypedArrayReverseImpl(thisArray);                         \
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
 * Typed Arrays: ToReversed Implementation
 */
template <typename T>
static T *EtsEscompatTypedArrayToReversedImpl(T *thisArray)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coro);

    auto arrayLength = static_cast<EtsInt>(thisArray->GetLengthInt());
    auto byteOffset = static_cast<size_t>(thisArray->GetByteOffset());
    auto byteLength = static_cast<size_t>(thisArray->GetByteLength());

    // Clone src array to dest array. Alloc new buffer. Assign new buffer to dest array.
    EtsHandle<EtsObject> srcArray(coro, thisArray);
    ASSERT(srcArray.GetPtr() != nullptr);

    auto *srcData = GetNativeData(static_cast<T *>(srcArray.GetPtr()));
    EtsHandle<EtsEscompatArrayBuffer> newBuffer = CreateArrayBuffer(coro, byteLength);
    if (UNLIKELY(newBuffer.GetPtr() == nullptr)) {
        return nullptr;
    }
    auto dstData = newBuffer->GetData();
    auto dstArray = static_cast<T *>(srcArray->Clone());

    ASSERT(dstArray != nullptr);
    dstArray->SetBuffer(newBuffer.GetPtr());
    dstArray->SetByteOffset(0);

    // Copy data from src array to dst array in reversed order.
    ASSERT(srcData != nullptr);
    ASSERT(dstData != nullptr);

    using ElementType = typename T::ElementType;
    auto *src = ToNativePtr<ElementType>(ToUintPtr(srcData) + byteOffset);
    auto *dst = static_cast<ElementType *>(dstData);
    ASSERT(src != nullptr);
    ASSERT(dst != nullptr);

    for (int i = 0; i < arrayLength; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, clang-analyzer-core.NullDereference)
        dst[i] = src[(arrayLength - 1) - i];
    }
    return dstArray;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TO_REVERSED_CALL_DECL(Type)                                                    \
    /* CC-OFFNXT(G.PRE.02) name part */                                                \
    extern "C" ark::ets::EtsEscompat##Type##Array *EtsEscompat##Type##ArrayToReversed( \
        ark::ets::EtsEscompat##Type##Array *thisArray)                                 \
    {                                                                                  \
        /* CC-OFFNXT(G.PRE.05) function gen */                                         \
        return EtsEscompatTypedArrayToReversedImpl(thisArray);                         \
    }

TO_REVERSED_CALL_DECL(Int8)
TO_REVERSED_CALL_DECL(Int16)
TO_REVERSED_CALL_DECL(Int32)
TO_REVERSED_CALL_DECL(BigInt64)
TO_REVERSED_CALL_DECL(Float32)
TO_REVERSED_CALL_DECL(Float64)

TO_REVERSED_CALL_DECL(UInt8)
TO_REVERSED_CALL_DECL(UInt8Clamped)
TO_REVERSED_CALL_DECL(UInt16)
TO_REVERSED_CALL_DECL(UInt32)
TO_REVERSED_CALL_DECL(BigUInt64)

#undef TO_REVERSED_CALL_DECL

static int32_t NormalizeIndex(int32_t idx, int32_t arrayLength)
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

    /**
     * False-positive static-analyzer report:
     * GC can happen only on ThrowException in GetNativeData.
     * But such case meaning data to be nullptr and retun prevents
     * us from proceeding
     */
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto arrayLength = thisArray->GetLengthInt();
    target = NormalizeIndex(target, arrayLength);
    start = NormalizeIndex(start, arrayLength);
    end = NormalizeIndex(end, arrayLength);

    int32_t count = end - start;
    if (count > (arrayLength - target)) {
        count = arrayLength - target;
    }
    if (count <= 0) {
        // See (GetNativeData) reason
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        return thisArray;
    }

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
    if constexpr (std::is_same_v<ElementType, EtsFloat> || std::is_same_v<ElementType, EtsDouble>) {
        std::sort(reinterpret_cast<typename T::ElementType *>(data.begin()),
                  reinterpret_cast<typename T::ElementType *>(data.end()),
                  [](const typename T::ElementType &a, const typename T::ElementType &b) {
                      if (std::isnan(a) && std::isnan(b)) {
                          return false;  // Keep order of NaN elements
                      }
                      if (std::isnan(a)) {
                          return false;  // NaN is greater than any number
                      }
                      if (std::isnan(b)) {
                          return true;  // Any number is less than NaN
                      }
                      return a < b;
                  });
        return thisArray;
    } else {
        std::sort(reinterpret_cast<typename T::ElementType *>(data.begin()),
                  reinterpret_cast<typename T::ElementType *>(data.end()));
        return thisArray;
    }
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

template <typename Array, typename = std::enable_if_t<std::is_floating_point_v<typename Array::ElementType>>>
static bool EtsEscompatTypedArrayContainsNaN(Array *array, EtsInt pos)
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

extern "C" EtsBoolean EtsEscompatFloat32ArrayContainsNaN(ark::ets::EtsEscompatFloat32Array *thisArray, EtsInt pos)
{
    return ToEtsBoolean(EtsEscompatTypedArrayContainsNaN(thisArray, pos));
}

extern "C" EtsBoolean EtsEscompatFloat64ArrayContainsNaN(ark::ets::EtsEscompatFloat64Array *thisArray, EtsInt pos)
{
    return ToEtsBoolean(EtsEscompatTypedArrayContainsNaN(thisArray, pos));
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

#define INVALID_INDEX (-1.0)

template <typename T, typename Pred>
static EtsDouble EtsEscompatArrayIndexOf(T *thisArray, EtsInt fromIndex, Pred pred)
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
static EtsDouble EtsEscompatArrayIndexOfLong(T *thisArray, SE searchElement, EtsInt fromIndex)
{
    using ElementType = typename T::ElementType;
    return EtsEscompatArrayIndexOf(
        thisArray, fromIndex,
        [element = static_cast<ElementType>(searchElement)](ElementType e) { return e == element; });
}

template <typename T>
static EtsDouble EtsEscompatArrayIndexOfNumber(T *thisArray, double searchElement, EtsInt fromIndex)
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
            return static_cast<double>(i);
        }
    }
    return INVALID_INDEX;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_ESCOMPAT_INDEX_OF_NUMBER(Type)                                                                    \
    /* CC-OFFNXT(G.PRE.02) name part */                                                                       \
    extern "C" EtsDouble EtsEscompat##Type##ArrayIndexOfNumber(ark::ets::EtsEscompat##Type##Array *thisArray, \
                                                               EtsDouble searchElement, EtsInt fromIndex)     \
    {                                                                                                         \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                \
        return EtsEscompatArrayIndexOfNumber(thisArray, searchElement, fromIndex);                            \
    }  // namespace ark::ets::intrinsics

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_ESCOMPAT_INDEX_OF_LONG(Type)                                                                    \
    /* CC-OFFNXT(G.PRE.02) name part */                                                                     \
    extern "C" EtsDouble EtsEscompat##Type##ArrayIndexOfLong(ark::ets::EtsEscompat##Type##Array *thisArray, \
                                                             EtsLong searchElement, EtsInt fromIndex)       \
    {                                                                                                       \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                              \
        return EtsEscompatArrayIndexOfLong(thisArray, searchElement, fromIndex);                            \
    }  // namespace ark::ets::intrinsics

ETS_ESCOMPAT_INDEX_OF_NUMBER(Int8)
ETS_ESCOMPAT_INDEX_OF_NUMBER(Int16)
ETS_ESCOMPAT_INDEX_OF_NUMBER(Int32)
ETS_ESCOMPAT_INDEX_OF_NUMBER(BigInt64)
ETS_ESCOMPAT_INDEX_OF_NUMBER(Float32)
ETS_ESCOMPAT_INDEX_OF_NUMBER(Float64)
ETS_ESCOMPAT_INDEX_OF_NUMBER(UInt8)
ETS_ESCOMPAT_INDEX_OF_NUMBER(UInt16)
ETS_ESCOMPAT_INDEX_OF_NUMBER(UInt32)
ETS_ESCOMPAT_INDEX_OF_NUMBER(BigUInt64)
ETS_ESCOMPAT_INDEX_OF_NUMBER(UInt8Clamped)
ETS_ESCOMPAT_INDEX_OF_LONG(Int8)
ETS_ESCOMPAT_INDEX_OF_LONG(Int16)
ETS_ESCOMPAT_INDEX_OF_LONG(Int32)
ETS_ESCOMPAT_INDEX_OF_LONG(BigInt64)
ETS_ESCOMPAT_INDEX_OF_LONG(Float32)
ETS_ESCOMPAT_INDEX_OF_LONG(Float64)
ETS_ESCOMPAT_INDEX_OF_LONG(UInt8)
ETS_ESCOMPAT_INDEX_OF_LONG(UInt16)
ETS_ESCOMPAT_INDEX_OF_LONG(UInt32)
ETS_ESCOMPAT_INDEX_OF_LONG(BigUInt64)
ETS_ESCOMPAT_INDEX_OF_LONG(UInt8Clamped)

template <typename T1, typename T2>
static EtsDouble EtsEscompatArrayLastIndexOfLong(T1 *thisArray, T2 searchElement, EtsInt fromIndex)
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
            return static_cast<double>(i);
        }
    }

    return INVALID_INDEX;
}

template <typename T>
static EtsDouble EtsEscompatArrayLastIndexOfNumber(T *thisArray, double searchElement, EtsInt fromIndex)
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
            return static_cast<double>(i);
        }
    }

    return INVALID_INDEX;
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(Type)                                                                   \
    /* CC-OFFNXT(G.PRE.02) name part */                                                                           \
    extern "C" EtsDouble EtsEscompat##Type##ArrayLastIndexOfNumber(ark::ets::EtsEscompat##Type##Array *thisArray, \
                                                                   EtsDouble searchElement, EtsInt fromIndex)     \
    {                                                                                                             \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                    \
        return EtsEscompatArrayLastIndexOfNumber(thisArray, searchElement, fromIndex);                            \
    }  // namespace ark::ets::intrinsics

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_ESCOMPAT_LAST_INDEX_OF_LONG(Type)                                                                   \
    /* CC-OFFNXT(G.PRE.02) name part */                                                                         \
    extern "C" EtsDouble EtsEscompat##Type##ArrayLastIndexOfLong(ark::ets::EtsEscompat##Type##Array *thisArray, \
                                                                 EtsLong searchElement, EtsInt fromIndex)       \
    {                                                                                                           \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                                  \
        return EtsEscompatArrayLastIndexOfLong(thisArray, searchElement, fromIndex);                            \
    }  // namespace ark::ets::intrinsics

ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(Int8)
ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(Int16)
ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(Int32)
ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(BigInt64)
ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(Float32)
ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(Float64)
ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(UInt8)
ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(UInt16)
ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(UInt32)
ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(BigUInt64)
ETS_ESCOMPAT_LAST_INDEX_OF_NUMBER(UInt8Clamped)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(Int8)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(Int16)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(Int32)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(BigInt64)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(Float32)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(Float64)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(UInt8)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(UInt16)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(UInt32)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(BigUInt64)
ETS_ESCOMPAT_LAST_INDEX_OF_LONG(UInt8Clamped)

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
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto sep = separator->GetDataUtf16()[0];
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
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto sep = separator->GetDataUtf8()[0];
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

extern "C" ark::ets::EtsString *EtsEscompatInt8ArrayJoin(ark::ets::EtsEscompatInt8Array *thisArray,
                                                         ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsEscompatUint8ArrayJoin(ark::ets::EtsEscompatUInt8Array *thisArray,
                                                          ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsEscompatUint8ClampedArrayJoin(ark::ets::EtsEscompatUInt8ClampedArray *thisArray,
                                                                 ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsEscompatInt16ArrayJoin(ark::ets::EtsEscompatInt16Array *thisArray,
                                                          ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsEscompatUint16ArrayJoin(ark::ets::EtsEscompatUInt16Array *thisArray,
                                                           ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsEscompatInt32ArrayJoin(ark::ets::EtsEscompatInt32Array *thisArray,
                                                          ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsEscompatUint32ArrayJoin(ark::ets::EtsEscompatUInt32Array *thisArray,
                                                           ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsEscompatBigInt64ArrayJoin(ark::ets::EtsEscompatBigInt64Array *thisArray,
                                                             ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsEscompatBigUint64ArrayJoin(ark::ets::EtsEscompatBigUInt64Array *thisArray,
                                                              ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsEscompatFloat32ArrayJoin(ark::ets::EtsEscompatFloat32Array *thisArray,
                                                            ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

extern "C" ark::ets::EtsString *EtsEscompatFloat64ArrayJoin(ark::ets::EtsEscompatFloat64Array *thisArray,
                                                            ark::ets::EtsString *separator)
{
    return TypedArrayJoin(thisArray, separator);
}

/*
 * With Implementation
 */

template <typename T>
static T *EtsEscompatTypedArrayWithImpl(T *thisArray, EtsInt pos, typename T::ElementType val)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(coro);

    auto srcLength = static_cast<EtsInt>(thisArray->GetLengthInt());
    auto elementSize = static_cast<size_t>(thisArray->GetBytesPerElement());
    auto byteOffset = static_cast<size_t>(thisArray->GetByteOffset());
    auto byteLength = static_cast<size_t>(thisArray->GetByteLength());

    auto *srcData = GetNativeData(thisArray);
    if (UNLIKELY(srcData == nullptr)) {
        return nullptr;
    }

    if (UNLIKELY(pos < 0 || pos >= srcLength)) {
        ThrowEtsException(coro, panda_file_items::class_descriptors::RANGE_ERROR, "Invalid typed array index");
        return nullptr;
    }

    EtsHandle<EtsObject> srcArray(coro, thisArray);
    if (UNLIKELY(srcArray.GetPtr() == nullptr)) {
        return nullptr;
    }

    auto *src = ToVoidPtr(ToUintPtr(srcData) + byteOffset);
    EtsHandle<EtsEscompatArrayBuffer> newBuffer = CreateArrayBuffer(coro, byteLength, reinterpret_cast<uint8_t *>(src));
    if (UNLIKELY(newBuffer.GetPtr() == nullptr) || UNLIKELY(newBuffer->GetData() == nullptr)) {
        return nullptr;
    }

    auto dstArray = static_cast<T *>(srcArray->Clone());
    if (UNLIKELY(dstArray == nullptr)) {
        return nullptr;
    }

    dstArray->SetBuffer(newBuffer.GetPtr());
    dstArray->SetByteOffset(0);

    ObjectAccessor::SetPrimitive(newBuffer->GetData(), pos * elementSize, val);
    return dstArray;
}

extern "C" ark::ets::EtsEscompatUInt8ClampedArray *EtsEscompatUInt8ClampedArrayWith(
    ark::ets::EtsEscompatUInt8ClampedArray *thisArray, EtsInt pos, EtsInt val)
{
    if (val > ark::ets::EtsEscompatUInt8ClampedArray::MAX) {
        val = ark::ets::EtsEscompatUInt8ClampedArray::MAX;
    } else if (val < ark::ets::EtsEscompatUInt8ClampedArray::MIN) {
        val = ark::ets::EtsEscompatUInt8ClampedArray::MIN;
    }
    return EtsEscompatTypedArrayWithImpl(thisArray, pos, val);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPED_ARRAY_WITH_CALL_DECL(AType, EType)                                   \
    /* CC-OFFNXT(G.PRE.02) name part */                                            \
    extern "C" ark::ets::EtsEscompat##AType##Array *EtsEscompat##AType##ArrayWith( \
        ark::ets::EtsEscompat##AType##Array *thisArray, EtsInt pos, EType val)     \
    {                                                                              \
        /* CC-OFFNXT(G.PRE.05) function gen */                                     \
        return EtsEscompatTypedArrayWithImpl(thisArray, pos, val);                 \
    }

TYPED_ARRAY_WITH_CALL_DECL(Int8, EtsByte)
TYPED_ARRAY_WITH_CALL_DECL(Int16, EtsShort)
TYPED_ARRAY_WITH_CALL_DECL(Int32, EtsInt)
TYPED_ARRAY_WITH_CALL_DECL(BigInt64, EtsLong)
TYPED_ARRAY_WITH_CALL_DECL(Float32, EtsFloat)
TYPED_ARRAY_WITH_CALL_DECL(Float64, EtsDouble)
TYPED_ARRAY_WITH_CALL_DECL(UInt8, EtsInt)
TYPED_ARRAY_WITH_CALL_DECL(UInt16, EtsInt)
TYPED_ARRAY_WITH_CALL_DECL(UInt32, EtsLong)
TYPED_ARRAY_WITH_CALL_DECL(BigUInt64, EtsLong)

static EtsEscompatArrayBuffer *CreateEtsEscompatArrayBuffer(EtsCoroutine *coro, PandaEtsVM *vm, size_t length,
                                                            void **resultData)
{
    ASSERT((coro != nullptr) && (vm != nullptr));
    auto *bufferClass = PlatformTypes(coro)->escompatArrayBuffer->GetRuntimeClass();
    EtsHandle<EtsEscompatArrayBuffer> bufferHandle(
        coro, reinterpret_cast<EtsEscompatArrayBuffer *>(vm->GetHeapManager()->AllocateObject(
                  // CC-OFFNXT(G.FMT.06-CPP) project code style
                  bufferClass, sizeof(EtsEscompatArrayBuffer), DEFAULT_ALIGNMENT, nullptr,
                  mem::ObjectAllocatorBase::ObjMemInitPolicy::NO_INIT, false)));
    if (UNLIKELY(bufferHandle.GetPtr() == nullptr)) {
        return nullptr;
    }

    auto *packetClass = vm->GetClassLinker()->GetClassRoot(EtsClassRoot::BYTE_ARRAY)->GetRuntimeClass();
    EtsHandle<EtsByteArray> packetHandle(coro, reinterpret_cast<EtsByteArray *>(ark::coretypes::Array::Create(
                                                   // CC-OFFNXT(G.FMT.06-CPP) project code style
                                                   packetClass, static_cast<ArraySizeT>(std::max(length, size_t(1))),
                                                   ark::SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT, false)));
    if (UNLIKELY(packetHandle.GetPtr() == nullptr)) {
        return nullptr;
    }

    bufferHandle->Initialize(coro, length, packetHandle.GetPtr());

    if (resultData != nullptr) {
        *resultData = bufferHandle->GetData();
    }

    return bufferHandle.GetPtr();
}

template <typename T>
static T *EtsEscompatTypedArraySliceImpl(T *thisArray, EtsInt begin, EtsInt end)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope s(coro);
    auto *vm = PandaEtsVM::GetCurrent();
    EtsHandle<EtsObject> srcArrayHandle(coro, thisArray);
    if (UNLIKELY((vm == nullptr) || (srcArrayHandle.GetPtr() == nullptr))) {
        return nullptr;
    }

    auto srcLength = static_cast<EtsInt>(reinterpret_cast<T *>(srcArrayHandle.GetPtr())->GetLengthInt());
    begin = NormalizeIndex(begin, srcLength);
    end = NormalizeIndex(end, srcLength);
    EtsInt lengthInt = begin <= end ? end - begin : EtsInt(0);
    auto elementSize = static_cast<size_t>(reinterpret_cast<T *>(srcArrayHandle.GetPtr())->GetBytesPerElement());
    auto byteLength = static_cast<size_t>(lengthInt) * elementSize;
    auto byteOffset = static_cast<size_t>(reinterpret_cast<T *>(srcArrayHandle.GetPtr())->GetByteOffset());
    auto *srcData = GetNativeData(reinterpret_cast<T *>(srcArrayHandle.GetPtr()));
    if (UNLIKELY(srcData == nullptr)) {
        return nullptr;
    }

    void *dst = nullptr;
    EtsHandle<EtsEscompatArrayBuffer> bufferHandle(coro, CreateEtsEscompatArrayBuffer(coro, vm, byteLength, &dst));
    if (UNLIKELY((bufferHandle.GetPtr() == nullptr) || (dst == nullptr))) {
        return nullptr;
    }

    auto *arrayClass = reinterpret_cast<T *>(srcArrayHandle.GetPtr())->GetClass()->GetRuntimeClass();
    EtsHandle<EtsObject> dstArrayHandle(coro, reinterpret_cast<T *>(vm->GetHeapManager()->AllocateObject(
                                                  // CC-OFFNXT(G.FMT.06-CPP) project code style
                                                  arrayClass, sizeof(T), DEFAULT_ALIGNMENT, nullptr,
                                                  mem::ObjectAllocatorBase::ObjMemInitPolicy::NO_INIT, false)));
    if (UNLIKELY(dstArrayHandle.GetPtr() == nullptr)) {
        return nullptr;
    }

    reinterpret_cast<T *>(dstArrayHandle.GetPtr())
        ->Initialize(coro, lengthInt, elementSize, 0, bufferHandle.GetPtr(),
                     reinterpret_cast<T *>(srcArrayHandle.GetPtr())->GetName());
    if (byteLength != size_t(0)) {
        auto *src = ToVoidPtr(ToUintPtr(srcData) + byteOffset + begin * elementSize);
        [[maybe_unused]] auto error = memmove_s(dst, byteLength, src, byteLength);
        ASSERT(error == EOK);
    }

    return reinterpret_cast<T *>(dstArrayHandle.GetPtr());
}

extern "C" ark::ets::EtsEscompatInt8Array *EtsEscompatInt8ArraySlice(ark::ets::EtsEscompatInt8Array *thisArray,
                                                                     EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
}

extern "C" ark::ets::EtsEscompatInt16Array *EtsEscompatInt16ArraySlice(ark::ets::EtsEscompatInt16Array *thisArray,
                                                                       EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
}

extern "C" ark::ets::EtsEscompatInt32Array *EtsEscompatInt32ArraySlice(ark::ets::EtsEscompatInt32Array *thisArray,
                                                                       EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
}

extern "C" ark::ets::EtsEscompatBigInt64Array *EtsEscompatBigInt64ArraySlice(
    ark::ets::EtsEscompatBigInt64Array *thisArray, EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
}

extern "C" ark::ets::EtsEscompatFloat32Array *EtsEscompatFloat32ArraySlice(ark::ets::EtsEscompatFloat32Array *thisArray,
                                                                           EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
}

extern "C" ark::ets::EtsEscompatFloat64Array *EtsEscompatFloat64ArraySlice(ark::ets::EtsEscompatFloat64Array *thisArray,
                                                                           EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
}

extern "C" ark::ets::EtsEscompatUInt8Array *EtsEscompatUInt8ArraySlice(ark::ets::EtsEscompatUInt8Array *thisArray,
                                                                       EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
}

extern "C" ark::ets::EtsEscompatUInt16Array *EtsEscompatUInt16ArraySlice(ark::ets::EtsEscompatUInt16Array *thisArray,
                                                                         EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
}

extern "C" ark::ets::EtsEscompatUInt32Array *EtsEscompatUInt32ArraySlice(ark::ets::EtsEscompatUInt32Array *thisArray,
                                                                         EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
}

extern "C" ark::ets::EtsEscompatBigUInt64Array *EtsEscompatBigUInt64ArraySlice(
    ark::ets::EtsEscompatBigUInt64Array *thisArray, EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
}

extern "C" ark::ets::EtsEscompatUInt8ClampedArray *EtsEscompatUInt8ClampedArraySlice(
    ark::ets::EtsEscompatUInt8ClampedArray *thisArray, EtsInt begin, EtsInt end)
{
    return EtsEscompatTypedArraySliceImpl(thisArray, begin, end);
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

}  // namespace ark::ets::intrinsics
