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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_UNSIGNED_ARRAYS_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_UNSIGNED_ARRAYS_H

#include "plugins/ets/runtime/types/ets_object.h"
<<<<<<< HEAD

namespace ark::ets {
=======
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets {

namespace test {
class EtsEscompatTypedUArrayBaseTest;
}  // namespace test

>>>>>>> OpenHarmony_feature_20250328
class EtsEscompatTypedUArrayBase : public EtsObject {
public:
    EtsEscompatTypedUArrayBase() = delete;
    ~EtsEscompatTypedUArrayBase() = delete;

    NO_COPY_SEMANTIC(EtsEscompatTypedUArrayBase);
    NO_MOVE_SEMANTIC(EtsEscompatTypedUArrayBase);

<<<<<<< HEAD
=======
    static constexpr size_t GetClassSize()
    {
        return sizeof(EtsEscompatTypedUArrayBase);
    }

>>>>>>> OpenHarmony_feature_20250328
    static constexpr size_t GetBufferOffset()
    {
        return MEMBER_OFFSET(EtsEscompatTypedUArrayBase, buffer_);
    }

<<<<<<< HEAD
=======
    static constexpr size_t GetBytesPerElementOffset()
    {
        return MEMBER_OFFSET(EtsEscompatTypedUArrayBase, bytesPerElement_);
    }

>>>>>>> OpenHarmony_feature_20250328
    static constexpr size_t GetByteOffsetOffset()
    {
        return MEMBER_OFFSET(EtsEscompatTypedUArrayBase, byteOffset_);
    }

    static constexpr size_t GetByteLengthOffset()
    {
        return MEMBER_OFFSET(EtsEscompatTypedUArrayBase, byteLength_);
    }

    static constexpr size_t GetLengthIntOffset()
    {
        return MEMBER_OFFSET(EtsEscompatTypedUArrayBase, lengthInt_);
    }

<<<<<<< HEAD
    static constexpr size_t GetArrayBufferBackedOffset()
    {
        return MEMBER_OFFSET(EtsEscompatTypedUArrayBase, arrayBufferBacked_);
    }

    ObjectPointer<EtsObject> GetBuffer()
    {
        return buffer_;
    }

    EtsDouble GetByteOffset()
=======
    ObjectPointer<EtsObject> GetBuffer() const
    {
        return EtsObject::FromCoreType(ObjectAccessor::GetObject(this, GetBufferOffset()));
    }

    void SetBuffer(ObjectPointer<EtsObject> buffer)
    {
        buffer_ = buffer;
    }

    EtsDouble GetByteOffset() const
>>>>>>> OpenHarmony_feature_20250328
    {
        return byteOffset_;
    }

<<<<<<< HEAD
    EtsDouble GetByteLength()
=======
    void SetByteOffset(EtsDouble offset)
    {
        byteOffset_ = offset;
    }

    EtsDouble GetByteLength() const
>>>>>>> OpenHarmony_feature_20250328
    {
        return byteLength_;
    }

<<<<<<< HEAD
    EtsDouble GetBytesPerElement()
=======
    void SetByteLength(EtsDouble byteLength)
    {
        byteLength_ = byteLength;
    }

    EtsInt GetBytesPerElement() const
>>>>>>> OpenHarmony_feature_20250328
    {
        return bytesPerElement_;
    }

<<<<<<< HEAD
    EtsInt GetLengthInt()
=======
    EtsInt GetLengthInt() const
>>>>>>> OpenHarmony_feature_20250328
    {
        return lengthInt_;
    }

<<<<<<< HEAD
    bool IsArrayBufferBacked()
    {
        return arrayBufferBacked_ != 0;
    }

    ObjectPointer<EtsString> GetName()
    {
        return name_;
=======
    void SetLengthInt(EtsInt length)
    {
        lengthInt_ = length;
    }

    ObjectPointer<EtsString> GetName() const
    {
        return reinterpret_cast<EtsString *>(
            ObjectAccessor::GetObject(this, MEMBER_OFFSET(EtsEscompatTypedUArrayBase, name_)));
    }

    void Initialize(EtsCoroutine *coro, EtsInt lengthInt, EtsInt bytesPerElement, EtsInt byteOffset, EtsObject *buffer,
                    EtsString *name)
    {
        ASSERT(buffer != nullptr);
        ObjectAccessor::SetObject(coro, this, GetBufferOffset(), buffer->GetCoreType());
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsEscompatTypedUArrayBase, name_),
                                  name != nullptr ? name->GetCoreType() : nullptr);
        bytesPerElement_ = bytesPerElement;
        byteOffset_ = byteOffset;
        byteLength_ = lengthInt * bytesPerElement;
        lengthInt_ = lengthInt;
>>>>>>> OpenHarmony_feature_20250328
    }

private:
    ObjectPointer<EtsObject> buffer_;
    ObjectPointer<EtsString> name_;
<<<<<<< HEAD
    EtsDouble bytesPerElement_;
    EtsInt byteOffset_;
    EtsInt byteLength_;
    EtsInt lengthInt_;
    EtsBoolean arrayBufferBacked_;
=======
    EtsInt bytesPerElement_;
    EtsInt byteOffset_;
    EtsInt byteLength_;
    EtsInt lengthInt_;

    friend class test::EtsEscompatTypedUArrayBaseTest;
>>>>>>> OpenHarmony_feature_20250328
};

template <typename T>
class EtsEscompatTypedUArray : public EtsEscompatTypedUArrayBase {
public:
    using ElementType = T;
};

class EtsEscompatUInt8ClampedArray : public EtsEscompatTypedUArray<uint8_t> {
public:
    static constexpr int MIN = 0;
    static constexpr int MAX = 255;
};
class EtsEscompatUInt8Array : public EtsEscompatTypedUArray<uint8_t> {};
class EtsEscompatUInt16Array : public EtsEscompatTypedUArray<uint16_t> {};
class EtsEscompatUInt32Array : public EtsEscompatTypedUArray<uint32_t> {};
class EtsEscompatBigUInt64Array : public EtsEscompatTypedUArray<uint64_t> {};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_UNSIGNED_ARRAYS_H
