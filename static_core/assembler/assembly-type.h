/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_TYPE_H
#define PANDA_ASSEMBLER_ASSEMBLY_TYPE_H

#include "define.h"
#include "file_items.h"
#include "isa.h"
#include "libpandabase/macros.h"

namespace ark::pandasm {

class Type {
public:
    enum VerificationType {
        TYPE_ID_OBJECT,
        TYPE_ID_ARRAY,
        TYPE_ID_CLASS,
        TYPE_ID_ANY_OBJECT,
    };

    Type() = default;
    DEFAULT_MOVE_SEMANTIC(Type);
    DEFAULT_COPY_SEMANTIC(Type);
    ~Type() = default;

    Type(std::string_view componentName, size_t rank, bool ignorePrimitive = false)
        : componentName_(componentName), rank_(rank)
    {
        name_ = GetName(componentName_, rank_);
        typeId_ = GetId(name_, ignorePrimitive);
    }

    Type(const Type &componentType, size_t rank)
        : Type(componentType.GetComponentName(), componentType.GetRank() + rank)
    {
    }

    PANDA_PUBLIC_API std::string GetDescriptor(bool ignorePrimitive = false) const;
    PANDA_PUBLIC_API std::string GetName() const
    {
        return name_;
    }

    std::string GetPandasmName() const
    {
        std::string namePa {name_};
        std::replace(namePa.begin(), namePa.end(), '/', '.');
        return namePa;
    }

    std::string GetComponentName() const
    {
        return componentName_;
    }

    size_t GetRank() const
    {
        return rank_;
    }

    Type GetComponentType() const
    {
        return Type(componentName_, rank_ > 0 ? rank_ - 1 : 0);
    }

    panda_file::Type::TypeId GetId() const
    {
        return typeId_;
    }

    bool IsArrayContainsPrimTypes() const
    {
        auto elem = GetId(componentName_);
        return elem != panda_file::Type::TypeId::REFERENCE;
    }

    bool IsValid() const
    {
        return !componentName_.empty();
    }

    bool IsArray() const
    {
        return rank_ > 0;
    }

    bool IsObject() const
    {
        return typeId_ == panda_file::Type::TypeId::REFERENCE;
    }

    bool IsTagged() const
    {
        return typeId_ == panda_file::Type::TypeId::TAGGED;
    }

    bool IsIntegral() const
    {
        return typeId_ == panda_file::Type::TypeId::U1 || typeId_ == panda_file::Type::TypeId::U8 ||
               typeId_ == panda_file::Type::TypeId::I8 || typeId_ == panda_file::Type::TypeId::U16 ||
               typeId_ == panda_file::Type::TypeId::I16 || typeId_ == panda_file::Type::TypeId::U32 ||
               typeId_ == panda_file::Type::TypeId::I32 || typeId_ == panda_file::Type::TypeId::U64 ||
               typeId_ == panda_file::Type::TypeId::I64;
    }

    bool FitsInto32() const
    {
        return typeId_ == panda_file::Type::TypeId::U1 || typeId_ == panda_file::Type::TypeId::U8 ||
               typeId_ == panda_file::Type::TypeId::I8 || typeId_ == panda_file::Type::TypeId::U16 ||
               typeId_ == panda_file::Type::TypeId::I16 || typeId_ == panda_file::Type::TypeId::U32 ||
               typeId_ == panda_file::Type::TypeId::I32;
    }

    bool IsFloat32() const
    {
        return typeId_ == panda_file::Type::TypeId::F32;
    }

    bool IsFloat64() const
    {
        return typeId_ == panda_file::Type::TypeId::F64;
    }

    bool IsPrim32() const
    {
        return (IsIntegral() && FitsInto32()) || IsFloat32();
    }

    bool IsPrim64() const
    {
        return (IsIntegral() && !FitsInto32()) || IsFloat64();
    }

    bool IsPrimitive() const
    {
        return IsPrim64() || IsPrim32();
    }

    bool IsVoid() const
    {
        return typeId_ == panda_file::Type::TypeId::VOID;
    }

    PANDA_PUBLIC_API static panda_file::Type::TypeId GetId(std::string_view name, bool ignorePrimitive = false);

    PANDA_PUBLIC_API static pandasm::Type FromPrimitiveId(panda_file::Type::TypeId id);

    bool operator==(const Type &type) const
    {
        return name_ == type.name_;
    }

    bool operator<(const Type &type) const
    {
        return name_ < type.name_;
    }

    static PANDA_PUBLIC_API Type FromDescriptor(std::string_view descriptor);

    static PANDA_PUBLIC_API Type FromName(std::string_view name, bool ignorePrimitive = false);

    static PANDA_PUBLIC_API bool IsPandaPrimitiveType(const std::string &name);
    static bool IsStringType(const std::string &name, ark::panda_file::SourceLang lang);

private:
    static PANDA_PUBLIC_API std::string GetName(std::string_view componentName, size_t rank);

    std::string componentName_;
    size_t rank_ {0};
    std::string name_;
    panda_file::Type::TypeId typeId_ {panda_file::Type::TypeId::VOID};
};

}  // namespace ark::pandasm

namespace std {

template <>
class hash<ark::pandasm::Type> {
public:
    size_t operator()(const ark::pandasm::Type &type) const
    {
        return std::hash<std::string>()(type.GetName());
    }
};

}  // namespace std

#endif  // PANDA_ASSEMBLER_ASSEMBLY_TYPE_H
