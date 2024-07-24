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

#include <unordered_map>

#include "assembly-type.h"

namespace ark::pandasm {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::unordered_map<std::string_view, std::string_view> g_primitiveTypes = {
    {"u1", "Z"},  {"i8", "B"},  {"u8", "H"},  {"i16", "S"}, {"u16", "C"},  {"i32", "I"}, {"u32", "U"},
    {"f32", "F"}, {"f64", "D"}, {"i64", "J"}, {"u64", "Q"}, {"void", "V"}, {"any", "A"}};

std::string Type::GetDescriptor(bool ignorePrimitive) const
{
    if (!ignorePrimitive) {
        auto it = g_primitiveTypes.find(componentName_);
        if (it != g_primitiveTypes.cend()) {
            return std::string(rank_, '[') + it->second.data();
        }
    }

    std::string res = std::string(rank_, '[') + "L" + componentName_ + ";";
    std::replace(res.begin(), res.end(), '.', '/');
    return res;
}

/* static */
panda_file::Type::TypeId Type::GetId(std::string_view name, bool ignorePrimitive)
{
    static std::unordered_map<std::string_view, panda_file::Type::TypeId> pandaTypes = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PANDATYPE(name, inst_code) {std::string_view(name), panda_file::Type::TypeId::inst_code},
        PANDA_ASSEMBLER_TYPES(PANDATYPE)
#undef PANDATYPE
    };

    if (!ignorePrimitive) {
        auto iter = pandaTypes.find(name);
        if (iter == pandaTypes.end()) {
            return panda_file::Type::TypeId::REFERENCE;
        }
        return iter->second;
    }

    return panda_file::Type::TypeId::REFERENCE;
}

/* static */
pandasm::Type Type::FromPrimitiveId(panda_file::Type::TypeId id)
{
    static std::unordered_map<panda_file::Type::TypeId, std::string_view> pandaTypes = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PANDATYPE(name, inst_code) {panda_file::Type::TypeId::inst_code, std::string_view(name)},
        PANDA_ASSEMBLER_TYPES(PANDATYPE)
#undef PANDATYPE
    };
    auto iter = pandaTypes.find(id);
    ASSERT(iter != pandaTypes.end());
    pandasm::Type ret {iter->second, 0};
    ASSERT(ret.typeId_ == id);
    return ret;
}

/* static */
std::string Type::GetName(std::string_view componentName, size_t rank)
{
    std::string name(componentName);
    while (rank-- > 0) {
        name += "[]";
    }
    return name;
}

/* static */
Type Type::FromDescriptor(std::string_view descriptor)
{
    static std::unordered_map<std::string_view, std::string_view> reversePrimitiveTypes = {
        {"Z", "u1"},  {"B", "i8"},  {"H", "u8"},  {"S", "i16"}, {"C", "u16"},  {"I", "i32"}, {"U", "u32"},
        {"F", "f32"}, {"D", "f64"}, {"J", "i64"}, {"Q", "u64"}, {"V", "void"}, {"A", "any"}};

    size_t i = 0;
    while (descriptor[i] == '[') {
        ++i;
    }

    size_t rank = i;
    bool isRefType = descriptor[i] == 'L';
    if (isRefType) {
        descriptor.remove_suffix(1); /* Remove semicolon */
        ++i;
    }

    descriptor.remove_prefix(i);

    if (isRefType) {
        return Type(descriptor, rank);
    }

    return Type(reversePrimitiveTypes[descriptor], rank);
}

/* static */
Type Type::FromName(std::string_view name, bool ignorePrimitive)
{
    constexpr size_t STEP = 2;

    size_t size = name.size();
    size_t i = 0;

    while (name[size - i - 1] == ']') {
        i += STEP;
    }

    name.remove_suffix(i);

    return Type(name, i / STEP, ignorePrimitive);
}

/* static */
bool Type::IsStringType(const std::string &name, ark::panda_file::SourceLang lang)
{
    auto stringType = Type::FromDescriptor(ark::panda_file::GetStringClassDescriptor(lang));
    return name == stringType.GetName();
}

/* static */
bool Type::IsPandaPrimitiveType(const std::string &name)
{
    auto it = g_primitiveTypes.find(name);
    return it != g_primitiveTypes.cend();
}

}  // namespace ark::pandasm
