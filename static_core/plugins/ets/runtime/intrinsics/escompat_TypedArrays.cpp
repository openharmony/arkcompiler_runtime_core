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
#include "intrinsics.h"
#include "cross_values.h"

namespace ark::ets::intrinsics {
extern "C" void EtsEscompatInt8ArraySetInt(ObjectHeader *obj, EtsInt pos, EtsInt val)
{
    auto typedArray = ToUintPtr(obj);
    auto length = *reinterpret_cast<EtsInt *>(typedArray + ark::cross_values::GetInt8ArrayLengthOffset(RUNTIME_ARCH));
    if (pos < 0 || pos >= length) {
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        PandaEtsVM::GetCurrent()->GetLanguageContext();
        ThrowException(ctx, coro, utf::CStringAsMutf8("Lstd/core/RangeError;"), utf::CStringAsMutf8("invalid index"));
        return;
    }
    auto arrayBufferBacked = *reinterpret_cast<EtsBoolean *>(
        typedArray + ark::cross_values::GetInt8ArrayArrayBufferBackedOffset(RUNTIME_ARCH));
    auto buffer = ToUintPtr(ToVoidPtr(*reinterpret_cast<ObjectPointerType *>(
        typedArray + ark::cross_values::GetInt8ArrayBufferOffset(RUNTIME_ARCH))));
    auto byteOffset =
        *reinterpret_cast<EtsDouble *>(typedArray + ark::cross_values::GetInt8ArrayByteOffsetOffset(RUNTIME_ARCH));

    if (arrayBufferBacked != 0) {
        auto *arrayBuffer = reinterpret_cast<EtsEscompatArrayBuffer *>(buffer);
        ObjectAccessor::SetPrimitive(arrayBuffer->GetData(), pos * sizeof(EtsByte) + static_cast<EtsInt>(byteOffset),
                                     static_cast<EtsByte>(val));
    } else {
        auto sharedMemory = ToUintPtr(ToVoidPtr(*reinterpret_cast<ObjectPointerType *>(
            buffer + ark::cross_values::GetSharedArrayBufferSharedMemoryOffset(RUNTIME_ARCH))));
        auto *data = ToVoidPtr(*reinterpret_cast<ObjectPointerType *>(
            sharedMemory + ark::cross_values::GetSharedMemoryDataOffset(RUNTIME_ARCH)));
        ObjectAccessor::SetPrimitive(data, coretypes::Array::GetDataOffset() + pos * sizeof(EtsByte) + byteOffset,
                                     static_cast<EtsByte>(val));
    }
}

extern "C" void EtsEscompatInt8ArraySetByte(ObjectHeader *obj, EtsInt pos, EtsByte val)
{
    EtsEscompatInt8ArraySetInt(obj, pos, val);
}

extern "C" EtsDouble EtsEscompatInt8ArrayGet(ObjectHeader *obj, EtsInt pos)
{
    auto typedArray = ToUintPtr(obj);
    auto length = *reinterpret_cast<EtsInt *>(typedArray + ark::cross_values::GetInt8ArrayLengthOffset(RUNTIME_ARCH));
    if (pos < 0 || pos >= length) {
        EtsCoroutine *coro = EtsCoroutine::GetCurrent();
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        PandaEtsVM::GetCurrent()->GetLanguageContext();
        ThrowException(ctx, coro, utf::CStringAsMutf8("Lstd/core/RangeError;"), utf::CStringAsMutf8("invalid index"));
        return 0;
    }
    auto arrayBufferBacked = *reinterpret_cast<EtsBoolean *>(
        typedArray + ark::cross_values::GetInt8ArrayArrayBufferBackedOffset(RUNTIME_ARCH));
    auto buffer = ToUintPtr(ToVoidPtr(*reinterpret_cast<ObjectPointerType *>(
        typedArray + ark::cross_values::GetInt8ArrayBufferOffset(RUNTIME_ARCH))));
    auto byteOffset =
        *reinterpret_cast<EtsDouble *>(typedArray + ark::cross_values::GetInt8ArrayByteOffsetOffset(RUNTIME_ARCH));

    if (arrayBufferBacked != 0) {
        auto *arrayBuffer = reinterpret_cast<EtsEscompatArrayBuffer *>(buffer);
        return ObjectAccessor::GetPrimitive<EtsByte>(arrayBuffer->GetData(),
                                                     pos * sizeof(EtsByte) + static_cast<EtsInt>(byteOffset));
    }

    auto sharedMemory = ToUintPtr(ToVoidPtr(*reinterpret_cast<ObjectPointerType *>(
        buffer + ark::cross_values::GetSharedArrayBufferSharedMemoryOffset(RUNTIME_ARCH))));
    auto *data = ToVoidPtr(*reinterpret_cast<ObjectPointerType *>(
        sharedMemory + ark::cross_values::GetSharedMemoryDataOffset(RUNTIME_ARCH)));
    return ObjectAccessor::GetPrimitive<EtsByte>(data, coretypes::Array::GetDataOffset() + pos * sizeof(EtsByte) +
                                                           byteOffset);
}
}  // namespace ark::ets::intrinsics
