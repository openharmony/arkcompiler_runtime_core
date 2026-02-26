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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_UTILS_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_UTILS_H

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/signature_utils.h"

#include <string>
#include <map>
#include <algorithm>
#include <vector>

namespace ark::ets {

// NOLINTBEGIN(modernize-avoid-c-arrays)
static constexpr char const ETSGLOBAL_CLASS_NAME[] = "ETSGLOBAL";
// NOLINTEND(modernize-avoid-c-arrays)

static constexpr std::string_view INDEXED_INT_GET_METHOD_SIGNATURE = "I:Lstd/core/Object;";
static constexpr std::string_view INDEXED_INT_SET_METHOD_SIGNATURE = "ILstd/core/Object;:V";

bool IsEtsGlobalClassName(const std::string &descriptor);

EtsObject *GetBoxedValue(EtsCoroutine *coro, Value value, EtsType type);

Value GetUnboxedValue(EtsCoroutine *coro, EtsObject *obj);

EtsObject *GetPropertyValue(EtsCoroutine *coro, const EtsObject *etsObj, EtsField *field);
bool SetPropertyValue(EtsCoroutine *coro, EtsObject *etsObj, EtsField *field, EtsObject *valToSet);

class LambdaUtils {
public:
    PANDA_PUBLIC_API static void InvokeVoid(EtsCoroutine *coro, EtsObject *lambda);

    NO_COPY_SEMANTIC(LambdaUtils);
    NO_MOVE_SEMANTIC(LambdaUtils);

private:
    LambdaUtils() = default;
    ~LambdaUtils() = default;
};

class ManglingUtils {
public:
    PANDA_PUBLIC_API static EtsString *GetDisplayNameStringFromField(EtsField *field);
    PANDA_PUBLIC_API static EtsField *GetFieldIDByDisplayName(EtsClass *klass, const PandaString &name,
                                                              const char *sig = nullptr);

    NO_COPY_SEMANTIC(ManglingUtils);
    NO_MOVE_SEMANTIC(ManglingUtils);

private:
    ManglingUtils() = default;
    ~ManglingUtils() = default;
};

PANDA_PUBLIC_API bool GetExportedClassDescriptorsFromModule(EtsClass *etsGlobalClass,
                                                            std::vector<std::string> &outDescriptors);

class RuntimeDescriptorParser final {
private:
    PandaString name_;
    size_t pos_ {0};
    static const std::map<char, PandaString> PRIMITIVE_NAME_MAPPING;

    PandaString ResolveRef()
    {
        PandaString res;
        while (name_[pos_] != ';') {
            char curSym = name_[pos_];
            res += (curSym != '/') ? curSym : '.';
            pos_ += 1;
        }
        pos_ += 1;
        return res;
    }

    PandaString ResolveUnion()
    {
        PandaString res = "{U";
        while (name_[pos_] != '}') {
            res += ResolveFixedArray();
            if (name_[pos_] != '}') {
                res += ',';
            }
        }
        pos_++;
        res += '}';
        return res;
    }

    PandaString ResolveFixedArray()
    {
        size_t i = 0;

        while (pos_ + i < name_.length() && name_[pos_ + i] == '[') {
            i++;
        }

        size_t rank = i;
        pos_ += i;
        PandaString brackets;
        for (size_t k = 0; k < rank; ++k) {
            brackets += "[]";
        }

        ASSERT(pos_ < name_.length());

        char typeChar = name_[pos_];
        if (typeChar == 'L') {
            pos_++;
            return ResolveRef() + brackets;
        }
        if (typeChar == '{') {
            pos_++;
            if (pos_ < name_.length() && name_[pos_] == 'U') {
                pos_++;
                return ResolveUnion() + brackets;
            }
            UNREACHABLE();
        }
        auto it = RuntimeDescriptorParser::PRIMITIVE_NAME_MAPPING.find(typeChar);
        ASSERT(it != RuntimeDescriptorParser::PRIMITIVE_NAME_MAPPING.end());
        PandaString primName = it->second;
        pos_++;
        return primName + brackets;
    }

public:
    explicit RuntimeDescriptorParser(PandaString inputStr) : name_(std::move(inputStr)) {}

    DEFAULT_COPY_SEMANTIC(RuntimeDescriptorParser);
    DEFAULT_MOVE_SEMANTIC(RuntimeDescriptorParser);

    ~RuntimeDescriptorParser() = default;

    PandaString Resolve()
    {
        PandaString res;
        if (name_.empty()) {
            return "";
        }
        if (!(name_[0] == '{' || name_[0] == '[')) {
            return name_;
        }

        while (pos_ < name_.length()) {
            res += ResolveFixedArray();
        }
        return res;
    }
};

class ClassPublicNameParser final {
private:
    size_t left_ {0};
    size_t right_ {0};
    PandaString name_;
    static const std::map<PandaString, char> PRIMITIVE_NAME_MAPPING;

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

        auto it = ClassPublicNameParser::PRIMITIVE_NAME_MAPPING.find(typeName);
        if (it != ClassPublicNameParser::PRIMITIVE_NAME_MAPPING.end()) {
            return brackets + PandaString(1, it->second);
        }
        PandaString refName = typeName;
        std::replace(refName.begin(), refName.end(), '.', '/');
        return brackets + 'L' + refName + ';';
    }

public:
    explicit ClassPublicNameParser(const PandaString &inputStr) : right_(inputStr.length() - 1), name_(inputStr) {}

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

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_UTILS_H
