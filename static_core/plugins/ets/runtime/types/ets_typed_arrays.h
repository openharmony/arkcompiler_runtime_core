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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_ARRAYS_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_ARRAYS_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets {

namespace test {
class EtsStdCoreTypedArrayBaseTest;
}  // namespace test

class EtsStdCoreTypedArrayBase : public EtsObject {
public:
    EtsStdCoreTypedArrayBase() = delete;
    ~EtsStdCoreTypedArrayBase() = delete;

    NO_COPY_SEMANTIC(EtsStdCoreTypedArrayBase);
    NO_MOVE_SEMANTIC(EtsStdCoreTypedArrayBase);

    static constexpr size_t GetClassSize()
    {
        return sizeof(EtsStdCoreTypedArrayBase);
    }

    static constexpr size_t GetBufferOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedArrayBase, buffer_);
    }

    static constexpr size_t GetBytesPerElementOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedArrayBase, bytesPerElement_);
    }

    static constexpr size_t GetByteOffsetOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedArrayBase, byteOffset_);
    }

    static constexpr size_t GetByteLengthOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedArrayBase, byteLength_);
    }

    static constexpr size_t GetNameOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedArrayBase, name_);
    }

    static constexpr size_t GetLengthIntOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreTypedArrayBase, lengthInt_);
    }

    ObjectPointer<EtsObject> GetBuffer() const
    {
        return EtsObject::FromCoreType(ObjectAccessor::GetObject(this, GetBufferOffset()));
    }

    void SetBuffer(ObjectPointer<EtsObject> buffer)
    {
        buffer_ = buffer;
    }

    EtsInt GetByteOffset() const
    {
        return byteOffset_;
    }

    void SetByteOffset(EtsInt offset)
    {
        byteOffset_ = offset;
    }

    EtsInt GetByteLength() const
    {
        return byteLength_;
    }

    void SetByteLength(EtsInt byteLength)
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
        return EtsString::FromEtsObject(EtsObject::FromCoreType(ObjectAccessor::GetObject(this, GetNameOffset())));
    }

private:
    ObjectPointer<EtsObject> buffer_;
    ObjectPointer<EtsString> name_;
    EtsInt bytesPerElement_;
    EtsInt lengthInt_;
    EtsInt byteOffset_;
    EtsInt byteLength_;

    friend class test::EtsStdCoreTypedArrayBaseTest;
};

template <typename T>
class EtsStdCoreTypedArray : public EtsStdCoreTypedArrayBase {
public:
    using ElementType = T;
};

class EtsStdCoreInt8Array : public EtsStdCoreTypedArray<EtsByte> {};
class EtsStdCoreInt16Array : public EtsStdCoreTypedArray<EtsShort> {};
class EtsStdCoreInt32Array : public EtsStdCoreTypedArray<EtsInt> {};
class EtsStdCoreBigInt64Array : public EtsStdCoreTypedArray<EtsLong> {};
class EtsStdCoreFloat32Array : public EtsStdCoreTypedArray<EtsFloat> {};
class EtsStdCoreFloat64Array : public EtsStdCoreTypedArray<EtsDouble> {};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_ARRAYS_H
