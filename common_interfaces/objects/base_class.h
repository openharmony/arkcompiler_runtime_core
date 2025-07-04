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
<<<<<<< HEAD

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
=======
#include <cstdint>
#include "objects/utils/bit_field.h"

namespace panda {
class BaseObject;

enum class ObjectType : uint8_t {
    INVALID = 0,
    FIRST_OBJECT_TYPE,

    LINE_STRING = FIRST_OBJECT_TYPE,
    SLICED_STRING,
    TREE_STRING,

>>>>>>> OpenHarmony_feature_20250328
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

<<<<<<< HEAD
    static constexpr size_t TYPE_BITFIELD_NUM = BITS_PER_BYTE * sizeof(CommonType);

    using ObjectTypeBits = BitField<CommonType, 0, TYPE_BITFIELD_NUM>; // 8

    CommonType GetObjectType() const
=======
    using HeaderType = uint64_t;

    static constexpr size_t TYPE_BITFIELD_NUM = common::BITS_PER_BYTE * sizeof(ObjectType);
    using ObjectTypeBits = common::BitField<ObjectType, 0, TYPE_BITFIELD_NUM>; // 8

    ObjectType GetObjectType() const
>>>>>>> OpenHarmony_feature_20250328
    {
        return ObjectTypeBits::Decode(bitfield_);
    }

<<<<<<< HEAD
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
=======
    bool IsString() const
    {
        return GetObjectType() >= ObjectType::LINE_STRING && GetObjectType() <= ObjectType::TREE_STRING;
>>>>>>> OpenHarmony_feature_20250328
    }

    bool IsLineString() const
    {
<<<<<<< HEAD
        return GetObjectType() == CommonType::LINE_STRING;
=======
        return GetObjectType() == ObjectType::LINE_STRING;
>>>>>>> OpenHarmony_feature_20250328
    }

    bool IsSlicedString() const
    {
<<<<<<< HEAD
        return GetObjectType() == CommonType::SLICED_STRING;
=======
        return GetObjectType() == ObjectType::SLICED_STRING;
>>>>>>> OpenHarmony_feature_20250328
    }

    bool IsTreeString() const
    {
<<<<<<< HEAD
        return GetObjectType() == CommonType::TREE_STRING;
=======
        return GetObjectType() == ObjectType::TREE_STRING;
>>>>>>> OpenHarmony_feature_20250328
    }

private:
    // header_ is a padding field in base class, it will be used to store the root class in ets_runtime.
<<<<<<< HEAD
    [[maybe_unused]] uint64_t header_;
=======
    FIELD_UNUSED_COMMON HeaderType header_;
>>>>>>> OpenHarmony_feature_20250328
    // bitfield will be initialized as the bitfield_ and bitfield1_ of js_hclass.
    // Now only the low 8bits in bitfield are used as the common type encode. Other field has no specific means here
    // but should follow the bitfield and bitfield1_ defines in js_hclass.
    uint64_t bitfield_;
<<<<<<< HEAD

    uint64_t GetBitField() const
    {
        return bitfield_;
    }
};
};
#endif //COMMON_INTERFACE_OBJECTS_BASE_CLASS_H
=======
};
}  // namespace panda
#endif //COMMON_INTERFACE_OBJECTS_BASE_CLASS_H
>>>>>>> OpenHarmony_feature_20250328
