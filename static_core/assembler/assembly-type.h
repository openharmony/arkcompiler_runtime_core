/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "libarkfile/file_items.h"
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

    Type(std::string_view componentName, size_t rank, bool ignorePrimitive = false) : rank_(rank)
    {
        name_ = GetName(componentName, rank_);
        typeId_ = GetId(name_, ignorePrimitive);
        FillComponentNames(componentName);
        Canonicalize();
    }

    Type(const Type &componentType, size_t rank)
        : Type(componentType.GetComponentName(), componentType.GetRank() + rank)
    {
    }

    static constexpr size_t UNION_PREFIX_LEN = 2;  // CC-OFF(G.NAM.03-CPP) project code style
    static constexpr size_t RANK_STEP = 2;         // CC-OFF(G.NAM.03-CPP) project code style

    PANDA_PUBLIC_API std::string GetDescriptor(bool ignorePrimitive = false) const;
    PANDA_PUBLIC_API std::string GetName() const
    {
        return name_;
    }

    size_t SkipUnion(std::string_view name)
    {
        auto unionLength = 2;
        while (name[unionLength++] != '}') {
            if (name[unionLength] == '{') {
                unionLength += SkipUnion(name.substr(unionLength));
            }
        }
        return unionLength;
    }

    std::pair<std::string, size_t> GetComponentName(std::string_view name)
    {
        if (name[0] != '{') {
            auto delimPos = name.find(',');
            if (delimPos != std::string::npos) {
                return {std::string(name).substr(0, delimPos), delimPos};
            }
            return {std::string(name), name.length()};
        }

        auto unionLen = SkipUnion(name);
        while ((name.length() > unionLen) && (name[unionLen] == '[')) {
            unionLen += Type::RANK_STEP;
        }
        return {std::string(name).substr(0, unionLen), unionLen};
    }

    void FillComponentNames(std::string_view componentNamesStr)
    {
        if (!IsUnion()) {
            componentNames_.emplace_back(componentNamesStr);
            return;
        }
        componentNamesStr.remove_prefix(Type::UNION_PREFIX_LEN);
        componentNamesStr.remove_suffix(1);
        while (!componentNamesStr.empty()) {
            auto [component, length] = GetComponentName(componentNamesStr);
            componentNames_.push_back(component);
            if (componentNamesStr.length() == length || componentNamesStr[length] != ',') {
                break;
            }
            componentNamesStr.remove_prefix(length + 1);
        }
        ASSERT(componentNames_.size() > 1);
    }

    std::string GetNameWithoutRank() const
    {
        auto idx = name_.length() - 1;
        while (name_[idx] == ']') {
            idx -= Type::RANK_STEP;
        }
        return name_.substr(0, idx + 1);
    }

    std::string GetPandasmName() const
    {
        std::string namePa {name_};
        std::replace(namePa.begin(), namePa.end(), '/', '.');
        return namePa;
    }

    std::string GetComponentName() const
    {
        if (componentNames_.size() == 1) {
            return componentNames_[0];
        }
        std::string res = "{U";
        for (const auto &comp : componentNames_) {
            res += comp + ",";
        }
        res.pop_back();
        return res + "}";
    }

    const std::vector<std::string> &GetComponentNames() const
    {
        return componentNames_;
    }

    std::string GetComponentDescriptor(bool ignorePrimitive) const;

    size_t GetRank() const
    {
        return rank_;
    }

    Type GetComponentType() const
    {
        ASSERT(componentNames_.size() == 1);
        return Type(componentNames_[0], rank_ > 0 ? rank_ - 1 : 0);
    }

    panda_file::Type::TypeId GetId() const
    {
        return typeId_;
    }

    bool IsArrayContainsPrimTypes() const
    {
        ASSERT(componentNames_.size() == 1);
        auto elem = GetId(componentNames_[0]);
        return elem != panda_file::Type::TypeId::REFERENCE;
    }

    bool IsValid() const
    {
        return !componentNames_.empty();
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

    bool IsUnion() const
    {
        return (name_.substr(0, Type::UNION_PREFIX_LEN) == "{U") && (name_.back() == '}');
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

    PANDA_PUBLIC_API void Canonicalize();
    static PANDA_PUBLIC_API std::string CanonicalizeDescriptor(std::string_view descriptor);

private:
    static PANDA_PUBLIC_API std::string GetName(std::string_view componentName, size_t rank);

    std::vector<std::string> componentNames_;
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
