/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "runtime/include/class_helper.h"

#include <algorithm>
#include <string>

#include "include/class_linker_extension.h"
#include "libarkbase/mem/mem.h"
#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/utils/utf.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/class_linker.h"

namespace ark {

/* static */
const uint8_t *ClassHelper::GetArrayDescriptor(const uint8_t *componentName, size_t rank, PandaString *storage)
{
    storage->clear();
    storage->append(rank, '[');
    storage->push_back('L');
    storage->append(utf::Mutf8AsCString(componentName));
    storage->push_back(';');
    std::replace(storage->begin(), storage->end(), '.', '/');
    return utf::CStringAsMutf8(storage->c_str());
}

/* static */
char ClassHelper::GetPrimitiveTypeDescriptorChar(panda_file::Type::TypeId typeId)
{
    // static_cast isn't necessary in most implementations but may be by standard
    return static_cast<char>(*GetPrimitiveTypeDescriptorStr(typeId));
}

/* static */
const uint8_t *ClassHelper::GetPrimitiveTypeDescriptorStr(panda_file::Type::TypeId typeId)
{
    if (typeId == panda_file::Type::TypeId::REFERENCE) {
        UNREACHABLE();
        return nullptr;
    }

    return utf::CStringAsMutf8(panda_file::Type::GetSignatureByTypeId(panda_file::Type(typeId)));
}

/* static */
const uint8_t *ClassHelper::GetPrimitiveDescriptor(panda_file::Type type, PandaString *storage)
{
    return GetPrimitiveArrayDescriptor(type, 0, storage);
}

/* static */
const uint8_t *ClassHelper::GetPrimitiveArrayDescriptor(panda_file::Type type, size_t rank, PandaString *storage)
{
    storage->clear();
    storage->append(rank, '[');
    storage->push_back(GetPrimitiveTypeDescriptorChar(type.GetId()));
    return utf::CStringAsMutf8(storage->c_str());
}

/* static */
const uint8_t *ClassHelper::GetReferenceDescriptor(const PandaString &name, PandaString *storage)
{
    *storage = "L" + name + ";";
    std::replace(storage->begin(), storage->end(), '.', '/');
    return utf::CStringAsMutf8(storage->c_str());
}

/* static */
bool ClassHelper::IsPrimitive(const uint8_t *descriptor)
{
    return (PRIMITIVE_RUNTIME_NAMES.count(*descriptor) > 0);
}

/* static */
bool ClassHelper::IsReference(const uint8_t *descriptor)
{
    Span<const uint8_t> sp(descriptor, 1);
    return sp[0] == 'L';
}

/* static */
bool ClassHelper::IsUnionDescriptor(const uint8_t *descriptor)
{
    Span<const uint8_t> sp(descriptor, 2);
    return sp[0] == '{' && sp[1] == 'U';
}

static size_t GetUnionTypeComponentsNumber(const uint8_t *descriptor)
{
    size_t length = 1;
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    while (descriptor[length] != '}') {
        if (descriptor[length] == '{') {
            length += GetUnionTypeComponentsNumber(&(descriptor[length]));
        } else {
            length += 1;
        }
    }
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return length + 1;
}

/* static */
Span<const uint8_t> ClassHelper::GetUnionComponent(const uint8_t *descriptor)
{
    if (IsPrimitive(descriptor)) {
        return Span(descriptor, 1);
    }
    if (IsArrayDescriptor(descriptor)) {
        auto dim = GetDimensionality(descriptor);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return Span(descriptor, dim + GetUnionComponent(&(descriptor[dim])).Size());
    }
    if (IsUnionDescriptor(descriptor)) {
        return Span(descriptor, GetUnionTypeComponentsNumber(descriptor));
    }
    ASSERT(IsReference(descriptor));
    return Span(descriptor, std::string(utf::Mutf8AsCString(descriptor)).find(';') + 1);
}

class RuntimeDescriptorParser final {
public:
    explicit RuntimeDescriptorParser(std::string_view inputStr) : name_(inputStr)
    {
        ASSERT(!name_.empty());
    }

    NO_COPY_SEMANTIC(RuntimeDescriptorParser);
    NO_MOVE_SEMANTIC(RuntimeDescriptorParser);

    ~RuntimeDescriptorParser() = default;

    std::string Parse() &&
    {
        result_.reserve(name_.size());
        ParseFixedArray();
        ASSERT(pos_ == name_.size());
        return result_;
    }

private:
    void ParseRef()
    {
        while (name_[pos_] != ';') {
            ASSERT(pos_ < name_.size());
            char curSym = name_[pos_];
            result_.push_back((curSym != '/') ? curSym : '.');
            pos_++;
            ASSERT(pos_ < name_.size());
        }
        pos_++;
    }

    void ParseUnion()
    {
        result_.append("{U");
        while (name_[pos_] != '}') {
            ASSERT(pos_ < name_.size());
            ParseFixedArray();
            if (name_[pos_] != '}') {
                result_.push_back(',');
            }
            ASSERT(pos_ < name_.size());
        }
        pos_++;
        result_.push_back('}');
    }

    void ParseFixedArray()
    {
        const size_t i = name_.find_first_not_of('[', pos_);
        ASSERT(i < name_.length());
        const size_t rank = i - pos_;
        pos_ = i;

        const char typeChar = name_[pos_];
        if (typeChar == 'L') {
            pos_++;
            ASSERT(pos_ < name_.size());
            ParseRef();
        } else if (typeChar == '{') {
            pos_++;
            ASSERT(pos_ < name_.length());
            ASSERT(name_[pos_] == 'U');
            pos_++;
            ASSERT(pos_ < name_.size());
            ParseUnion();
        } else {
            auto it = ClassHelper::PRIMITIVE_RUNTIME_NAMES.find(typeChar);
            ASSERT(it != ClassHelper::PRIMITIVE_RUNTIME_NAMES.end());
            result_.append(it->second);
            pos_++;
        }

        for (size_t k = 0; k < rank; ++k) {
            result_.append("[]");
        }
    }

    std::string result_;
    std::string_view name_;
    size_t pos_ {0};
};

/* static */
std::string ClassHelper::GetName(const uint8_t *descriptor)
{
    RuntimeDescriptorParser parser(utf::Mutf8AsCString(descriptor));
    return std::move(parser).Parse();
}

class ClassPublicNameParser final {
private:
    size_t left_ {0};
    size_t right_ {0};
    PandaString name_;
    inline static const std::map<PandaString, char> RUNTIME_NAME_MAPPING = {
        {"u16", 'C'}, {"i8", 'B'}, {"i16", 'S'}, {"i32", 'I'}, {"i64", 'J'}, {"f32", 'F'}, {"f64", 'D'}, {"u1", 'Z'}};

    static PandaVector<PandaString> SplitUnion(const PandaString &unionName)
    {
        size_t deep = 0;
        PandaVector<PandaString> res;
        for (size_t i = 0; i < unionName.length(); i++) {
            PandaString temp;
            bool start = true;
            while (i < unionName.length() && (unionName[i] != ',' || deep != 0)) {
                char sym = unionName[i];
                if (sym == '{') {
                    deep++;
                } else if (sym == '}') {
                    deep--;
                }

                temp += sym;
                i++;
                start = false;
            }
            if (!start) {
                res.push_back(temp);
            }
        }
        return res;
    }

    std::optional<PandaString> ResolveUnion()
    {
        PandaString res;
        PandaString unionContent = name_.substr(left_, right_ - left_ + 1);
        PandaVector<PandaString> typesArr = SplitUnion(unionContent);

        for (const PandaString &typeName : typesArr) {
            auto resolvedTypeNameOpt = ClassPublicNameParser(typeName).Resolve();
            if (UNLIKELY(!resolvedTypeNameOpt)) {
                return std::nullopt;
            }
            res += resolvedTypeNameOpt.value();
        }
        return res;
    }

    std::optional<PandaString> ResolveFixedArray()
    {
        size_t i = 0;
        const size_t step = 2;
        while (right_ - i > left_ && name_[right_ - i] == ']' && name_[right_ - i - 1] == '[') {
            i += step;
        }

        right_ -= i;

        PandaString brackets = PandaString(i / step, '[');
        if (left_ <= right_ && name_[left_] == '{') {
            left_ += 1;
            if (left_ <= right_ && name_[left_] == 'U' && name_[right_] == '}') {
                left_ += 1;
                right_ -= 1;
                auto resolvedUnionOpt = ResolveUnion();
                if (UNLIKELY(!resolvedUnionOpt)) {
                    return std::nullopt;
                }
                return brackets + "{U" + std::move(resolvedUnionOpt.value()) + "}";
            }
            // Not a recognized union format
            return "{}";
        }

        PandaString typeName;
        if (left_ <= right_) {
            typeName = name_.substr(left_, right_ - left_ + 1);
        } else {
            return brackets;
        }

        auto it = ClassPublicNameParser::RUNTIME_NAME_MAPPING.find(typeName);
        if (it != ClassPublicNameParser::RUNTIME_NAME_MAPPING.end()) {
            return brackets + PandaString(1, it->second);
        }
        PandaString refName = typeName;
        std::replace(refName.begin(), refName.end(), '.', '/');
        return brackets + 'L' + refName + ';';
    }

public:
    explicit ClassPublicNameParser(const PandaString &inputStr) : right_(inputStr.length() - 1), name_(inputStr)
    {
        ASSERT(!name_.empty());
    }

    DEFAULT_COPY_SEMANTIC(ClassPublicNameParser);
    DEFAULT_MOVE_SEMANTIC(ClassPublicNameParser);

    ~ClassPublicNameParser() = default;

    std::optional<PandaString> Resolve()
    {
        auto normNameOpt = signature::NormalizePackageSeparators<PandaString, '.'>(name_, 0, name_.size());
        if (UNLIKELY(!normNameOpt.has_value())) {
            return std::nullopt;
        }
        name_ = std::move(normNameOpt.value());
        if (std::find(name_.begin(), name_.end(), '/') != name_.end()) {
            return std::nullopt;
        }
        return ResolveFixedArray();
    }
};

/* static */
const uint8_t *ClassHelper::GetDescriptor(const uint8_t *name, PandaString *storage)
{
    auto pandaStr = utf::Mutf8AsCString(name);
    ark::ClassPublicNameParser parser(pandaStr);
    auto res = parser.Resolve();
    if (!res.has_value()) {
        return nullptr;
    }
    storage->assign(res.value());
    return utf::CStringAsMutf8(storage->c_str());
}

}  // namespace ark
