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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_UNSIGNED_ARRAYS_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_UNSIGNED_ARRAYS_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets {

namespace test {
class EtsStdCoreTypedUArrayBaseTest;
}  // namespace test

class EtsStdCoreTypedUArrayBase : public EtsObject {
public:
    EtsStdCoreTypedUArrayBase() = delete;
    ~EtsStdCoreTypedUArrayBase() = delete;

    NO_COPY_SEMANTIC(EtsStdCoreTypedUArrayBase);
    NO_MOVE_SEMANTIC(EtsStdCoreTypedUArrayBase);

    static constexpr size_t GetClassSize()
    {
        return sizeof(EtsStdCoreTypedUArrayBase);
    }

    static constexpr size_t GetBufferOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedUArrayBase, buffer_);
    }

    static constexpr size_t GetBytesPerElementOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedUArrayBase, bytesPerElement_);
    }

    static constexpr size_t GetByteOffsetOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedUArrayBase, byteOffset_);
    }

    static constexpr size_t GetByteLengthOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedUArrayBase, byteLength_);
    }

    static constexpr size_t GetLengthIntOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedUArrayBase, lengthInt_);
    }

    ObjectPointer<EtsObject> GetBuffer() const
    {
        return EtsObject::FromCoreType(ObjectAccessor::GetObject(this, GetBufferOffset()));
    }

    void SetBuffer(ObjectPointer<EtsObject> buffer)
    {
        buffer_ = buffer;
    }

    EtsDouble GetByteOffset() const
    {
        return byteOffset_;
    }

    void SetByteOffset(EtsDouble offset)
    {
        byteOffset_ = offset;
    }

    EtsDouble GetByteLength() const
    {
        return byteLength_;
    }

    void SetByteLength(EtsDouble byteLength)
    {
        byteLength_ = byteLength;
    }

    EtsInt GetBytesPerElement() const
    {
        return bytesPerElement_;
    }

    EtsInt GetLengthInt() const
    {
        return lengthInt_;
    }

    void SetLengthInt(EtsInt length)
    {
        lengthInt_ = length;
    }

    ObjectPointer<EtsString> GetName() const
    {
        return reinterpret_cast<EtsString *>(
            ObjectAccessor::GetObject(this, MEMBER_OFFSET(EtsStdCoreTypedUArrayBase, name_)));
    }

    void Initialize(EtsExecutionContext *executionCtx, EtsInt lengthInt, EtsInt bytesPerElement, EtsInt byteOffset,
                    EtsObject *buffer, EtsString *name)
    {
        ASSERT(buffer != nullptr);
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, GetBufferOffset(), buffer->GetCoreType());
        ObjectAccessor::SetObject(executionCtx->GetMT(), this, MEMBER_OFFSET(EtsStdCoreTypedUArrayBase, name_),
                                  name != nullptr ? name->GetCoreType() : nullptr);
        bytesPerElement_ = bytesPerElement;
        byteOffset_ = byteOffset;
        byteLength_ = lengthInt * bytesPerElement;
        lengthInt_ = lengthInt;
    }

private:
    ObjectPointer<EtsObject> buffer_;
    ObjectPointer<EtsString> name_;
    EtsInt bytesPerElement_;
    EtsInt byteOffset_;
    EtsInt byteLength_;
    EtsInt lengthInt_;

    friend class test::EtsStdCoreTypedUArrayBaseTest;
};

template <typename T>
class EtsStdCoreTypedUArray : public EtsStdCoreTypedUArrayBase {
public:
    using ElementType = T;
};

class EtsStdCoreUInt8ClampedArray : public EtsStdCoreTypedUArray<uint8_t> {
public:
    static constexpr int MIN = 0;
    static constexpr int MAX = 255;
};
class EtsStdCoreUInt8Array : public EtsStdCoreTypedUArray<uint8_t> {};
class EtsStdCoreUInt16Array : public EtsStdCoreTypedUArray<uint16_t> {};
class EtsStdCoreUInt32Array : public EtsStdCoreTypedUArray<uint32_t> {};
class EtsStdCoreBigUInt64Array : public EtsStdCoreTypedUArray<uint64_t> {};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_UNSIGNED_ARRAYS_H
