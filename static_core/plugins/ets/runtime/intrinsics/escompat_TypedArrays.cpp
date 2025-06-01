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
    if (!array->IsArrayBufferBacked()) {
        return static_cast<EtsEscompatSharedArrayBuffer *>(&*array->GetBuffer())
            ->GetSharedMemory()
            ->GetData()
            ->GetData<void>();
    }

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
}  // namespace ark::ets::intrinsics
