/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACE_OBJECTS_BASE_CLASS_H
#define COMMON_INTERFACE_OBJECTS_BASE_CLASS_H

#include "base/bit_field.h"

namespace common {
class BaseObject;

static constexpr uint32_t BITS_PER_BYTE = 8;

enum class CommonType : uint8_t {
    INVALID = 0,
    FIRST_OBJECT_TYPE,
    LINE_STRING = FIRST_OBJECT_TYPE,

    SLICED_STRING,
    TREE_STRING,
    LAST_OBJECT_TYPE = TREE_STRING,

    STRING_FIRST = LINE_STRING,
    STRING_LAST = TREE_STRING,
};

class BaseClass {
public:
    BaseClass() = delete;
    ~BaseClass() = delete;
    NO_MOVE_SEMANTIC_CC(BaseClass);
    NO_COPY_SEMANTIC_CC(BaseClass);

    static constexpr size_t TYPE_BITFIELD_NUM = BITS_PER_BYTE * sizeof(CommonType);

    using ObjectTypeBits = BitField<CommonType, 0, TYPE_BITFIELD_NUM>; // 8

    CommonType GetObjectType() const
    {
        return ObjectTypeBits::Decode(bitfield_);
    }

    void SetObjectType(CommonType type)
    {
        bitfield_ = ObjectTypeBits::Update(bitfield_, type);
    }

    void ClearBitField()
    {
        bitfield_ = 0;
    }

    bool IsString() const
    {
        return GetObjectType() >= CommonType::LINE_STRING && GetObjectType() <= CommonType::TREE_STRING;
    }

    bool IsLineString() const
    {
        return GetObjectType() == CommonType::LINE_STRING;
    }

    bool IsSlicedString() const
    {
        return GetObjectType() == CommonType::SLICED_STRING;
    }

    bool IsTreeString() const
    {
        return GetObjectType() == CommonType::TREE_STRING;
    }

private:
    // header_ is a padding field in base class, it will be used to store the root class in ets_runtime.
    [[maybe_unused]] uint64_t header_;
    // bitfield will be initialized as the bitfield_ and bitfield1_ of js_hclass.
    // Now only the low 8bits in bitfield are used as the common type encode. Other field has no specific means here
    // but should follow the bitfield and bitfield1_ defines in js_hclass.
    uint64_t bitfield_;

    uint64_t GetBitField() const
    {
        return bitfield_;
    }
};
};
#endif //COMMON_INTERFACE_OBJECTS_BASE_CLASS_H