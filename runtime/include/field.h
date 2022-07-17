/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_FIELD_H_
#define PANDA_RUNTIME_FIELD_H_

#include <cstdint>
#include <atomic>

#include "intrinsics_enum.h"
#include "libpandafile/file.h"
#include "libpandafile/file_items.h"
#include "libpandafile/modifiers.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/class_helper.h"
#include "runtime/include/value-inl.h"
#include "libpandabase/macros.h"

namespace panda {

class Class;

class ClassLinkerErrorHandler;

class Field {
public:
    using UniqId = uint64_t;

    Field(Class *klass, panda_file::File::EntityId file_id, uint32_t access_flags, panda_file::Type type)
        : class_word_(static_cast<ClassHelper::classWordSize>(ToObjPtrType(klass))), file_id_(file_id)
    {
        access_flags_ = access_flags | (static_cast<uint32_t>(type.GetEncoding()) << ACC_TYPE_SHIFT);
    }

    Class *GetClass() const
    {
        return reinterpret_cast<Class *>(class_word_);
    }

    void SetClass(Class *cls)
    {
        class_word_ = static_cast<ClassHelper::classWordSize>(ToObjPtrType(cls));
    }

    static constexpr uint32_t GetClassOffset()
    {
        return MEMBER_OFFSET(Field, class_word_);
    }

    const panda_file::File *GetPandaFile() const;

    panda_file::File::EntityId GetFileId() const
    {
        return file_id_;
    }

    uint32_t GetAccessFlags() const
    {
        return access_flags_;
    }

    uint32_t GetOffset() const
    {
        return offset_;
    }

    void SetOffset(uint32_t offset)
    {
        offset_ = offset;
    }

    static constexpr uint32_t GetOffsetOffset()
    {
        return MEMBER_OFFSET(Field, offset_);
    }

    Class *ResolveTypeClass(ClassLinkerErrorHandler *error_handler = nullptr) const;

    panda_file::Type GetType() const
    {
        return panda_file::Type(GetTypeId());
    }

    panda_file::Type::TypeId GetTypeId() const
    {
        return static_cast<panda_file::Type::TypeId>((access_flags_ & ACC_TYPE) >> ACC_TYPE_SHIFT);
    }

    panda_file::File::StringData GetName() const;

    bool IsPublic() const
    {
        return (access_flags_ & ACC_PUBLIC) != 0;
    }

    bool IsPrivate() const
    {
        return (access_flags_ & ACC_PRIVATE) != 0;
    }

    bool IsProtected() const
    {
        return (access_flags_ & ACC_PROTECTED) != 0;
    }

    bool IsStatic() const
    {
        return (access_flags_ & ACC_STATIC) != 0;
    }

    bool IsVolatile() const
    {
        return (access_flags_ & ACC_VOLATILE) != 0;
    }

    bool IsFinal() const
    {
        return (access_flags_ & ACC_FINAL) != 0;
    }

    static inline UniqId CalcUniqId(const panda_file::File *file, panda_file::File::EntityId file_id)
    {
        constexpr uint64_t HALF = 32ULL;
        uint64_t uid = file->GetFilenameHash();
        uid <<= HALF;
        uid |= file_id.GetOffset();
        return uid;
    }

    UniqId GetUniqId() const
    {
        return CalcUniqId(GetPandaFile(), file_id_);
    }

    ~Field() = default;

    NO_COPY_SEMANTIC(Field);
    NO_MOVE_SEMANTIC(Field);

private:
    ClassHelper::classWordSize class_word_;
    panda_file::File::EntityId file_id_;
    uint32_t access_flags_;
    uint32_t offset_ {0};
};

}  // namespace panda

#endif  // PANDA_RUNTIME_FIELD_H_
