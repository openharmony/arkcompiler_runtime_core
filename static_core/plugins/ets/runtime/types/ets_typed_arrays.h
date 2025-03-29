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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_ARRAYS_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_ARRAYS_H

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {
class EtsEscompatInt8Array : public EtsObject {
public:
    EtsEscompatInt8Array() = delete;
    ~EtsEscompatInt8Array() = delete;

    NO_COPY_SEMANTIC(EtsEscompatInt8Array);
    NO_MOVE_SEMANTIC(EtsEscompatInt8Array);

    static constexpr size_t GetBufferOffset()
    {
        return MEMBER_OFFSET(EtsEscompatInt8Array, buffer_);
    }

    static constexpr size_t GetByteOffsetOffset()
    {
        return MEMBER_OFFSET(EtsEscompatInt8Array, byteOffset_);
    }

    static constexpr size_t GetByteLengthOffset()
    {
        return MEMBER_OFFSET(EtsEscompatInt8Array, byteLength_);
    }

    static constexpr size_t GetNameOffset()
    {
        return MEMBER_OFFSET(EtsEscompatInt8Array, name_);
    }

    static constexpr size_t GetLengthIntOffset()
    {
        return MEMBER_OFFSET(EtsEscompatInt8Array, lengthInt_);
    }

    static constexpr size_t GetArrayBufferBackedOffset()
    {
        return MEMBER_OFFSET(EtsEscompatInt8Array, arrayBufferBacked_);
    }

    ObjectPointer<EtsObject> GetBuffer()
    {
        return buffer_;
    }

    EtsDouble GetByteOffset()
    {
        return byteOffset_;
    }

    EtsDouble GetByteLength()
    {
        return byteLength_;
    }

    EtsDouble GetBytesPerElement()
    {
        return bytesPerElement_;
    }

    EtsInt GetLengthInt()
    {
        return lengthInt_;
    }

    EtsBoolean GetArrayBufferBacked()
    {
        return arrayBufferBacked_;
    }

    ObjectPointer<EtsString> GetName()
    {
        return name_;
    }

private:
    ObjectPointer<EtsObject> buffer_;
    EtsDouble bytesPerElement_;
    EtsDouble byteOffset_;
    EtsDouble byteLength_;
    EtsInt lengthInt_;
    EtsBoolean arrayBufferBacked_;
    ObjectPointer<EtsString> name_;
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_TYPED_ARRAYS_H
