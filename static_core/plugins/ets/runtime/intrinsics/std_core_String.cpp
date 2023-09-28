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

#include <algorithm>
#include <regex>
#include "intrinsics.h"
#include "libpandabase/utils/logger.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/entrypoints/string_index_of.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_language_context.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"

#include "unicode/locid.h"
#include "unicode/coll.h"
#include "unicode/unistr.h"

namespace panda::ets::intrinsics {

constexpr const wchar_t CHAR0XDFFF = 0xdfff;
constexpr const wchar_t CHAR0XD800 = 0xd800;

EtsCharArray *StdCoreStringGetChars(EtsString *s, ets_int begin, ets_int end)
{
    ASSERT(s != nullptr);
    ets_int length = s->GetLength();
    if (UNLIKELY(begin > end)) {
        panda::ThrowStringIndexOutOfBoundsException(begin, length);
        return nullptr;
    }
    if (UNLIKELY(begin > length || begin < 0)) {
        panda::ThrowStringIndexOutOfBoundsException(begin, length);
        return nullptr;
    }
    if (UNLIKELY(end > length)) {
        panda::ThrowStringIndexOutOfBoundsException(end, length);
        return nullptr;
    }

    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::String> s_handle(thread, s->GetCoreType());
    ets_int n = end - begin;
    EtsCharArray *char_array = EtsCharArray::Create(n);
    if (char_array == nullptr || n == 0) {
        return char_array;
    }
    Span<ets_char> out(char_array->GetData<ets_char>(), char_array->GetLength());
    s_handle.GetPtr()->CopyDataRegionUtf16(&out[0], begin, char_array->GetLength(), s_handle.GetPtr()->GetLength());
    return char_array;
}

uint16_t StdCoreStringCharAt(EtsString *s, int32_t index)
{
    ASSERT(s != nullptr);

    int32_t length = s->GetLength();

    if (UNLIKELY(index >= length || index < 0)) {
        panda::ThrowStringIndexOutOfBoundsException(index, length);
        return 0;
    }

    if (s->IsUtf16()) {
        Span<uint16_t> sp(s->GetDataUtf16(), length);
        return sp[index];
    }

    Span<uint8_t> sp(s->GetDataMUtf8(), length);
    return sp[index];
}

int32_t StdCoreStringLength(EtsString *s)
{
    ASSERT(s != nullptr);
    return s->GetLength();
}

EtsBoolean StdCoreStringIsEmpty(EtsString *s)
{
    ASSERT(s != nullptr);
    return ToEtsBoolean(s->IsEmpty());
}

uint8_t StdCoreStringEquals(EtsString *owner, EtsObject *s)
{
    if ((owner->AsObject()) == s) {
        return UINT8_C(1);
    }
    if (s == nullptr || !(s->GetClass()->IsStringClass())) {
        return UINT8_C(0);
    }
    return static_cast<uint8_t>(owner->StringsAreEqual(s));
}

int32_t StdCoreStringSearch(EtsString *this_str, EtsString *reg)
{
    PandaVector<uint8_t> buf;
    auto this_s = std::string(this_str->ConvertToStringView(&buf));
    auto regex = std::string(reg->ConvertToStringView(&buf));

    std::regex e(regex);
    return std::sregex_iterator(this_s.begin(), this_s.end(), e)->position();
}

EtsString *StdCoreStringMatch(EtsString *this_str, EtsString *reg)
{
    PandaVector<uint8_t> buf;
    auto this_s = std::string(this_str->ConvertToStringView(&buf));
    auto regex = std::string(reg->ConvertToStringView(&buf));

    std::regex e(regex);
    return EtsString::CreateFromMUtf8(std::sregex_iterator(this_s.begin(), this_s.end(), e)->str().c_str());
}

EtsString *StdCoreStringNormalize(EtsString *this_str)
{
    PandaVector<uint8_t> buf;
    auto str = std::string(this_str->ConvertToStringView(&buf));

    str.erase(
        std::remove_if(str.begin(), str.end(), [](char16_t ch) { return (ch >= CHAR0XD800) && (ch <= CHAR0XDFFF); }),
        str.end());
    return EtsString::CreateFromMUtf8(str.c_str());
}

uint8_t StdCoreStringIsWellFormed(EtsString *this_str)
{
    PandaVector<uint8_t> buf;
    auto str = std::string(this_str->ConvertToStringView(&buf));

    for (char16_t ch : str) {
        if ((ch >= CHAR0XD800) && (ch <= CHAR0XDFFF)) {
            return UINT8_C(0);
        }
    }
    return UINT8_C(1);
}

EtsString *StdCoreStringToLocaleLowerCase(EtsString *this_str, EtsString *locale)
{
    PandaVector<uint8_t> buf;
    auto this_s = std::string(this_str->ConvertToStringView(&buf));
    auto loc = std::string(locale->ConvertToStringView(&buf));
    std::string new_str;
    std::stringstream ss;
    for (auto elem : this_s) {
        new_str.push_back(std::tolower(elem, std::locale(ss.getloc(), new std::time_put_byname<char>(loc.c_str()))));
    }
    return EtsString::CreateFromMUtf8(new_str.c_str());
}

EtsString *StdCoreStringToLocaleUpperCase(EtsString *this_str, EtsString *locale)
{
    PandaVector<uint8_t> buf;
    auto this_s = std::string(this_str->ConvertToStringView(&buf));
    auto loc = std::string(locale->ConvertToStringView(&buf));
    std::string new_str;
    std::stringstream ss;
    for (auto elem : this_s) {
        new_str.push_back(std::toupper(elem, std::locale(ss.getloc(), new std::time_put_byname<char>(loc.c_str()))));
    }
    return EtsString::CreateFromMUtf8(new_str.c_str());
}

ets_short StdCoreStringLocaleCmp(EtsString *this_str, EtsString *cmp_str, EtsString *locale_str)
{
    ASSERT(this_str != nullptr && cmp_str != nullptr);
    icu::Locale loc;
    UErrorCode status = U_ZERO_ERROR;
    if (locale_str != nullptr) {
        PandaVector<uint8_t> buf;
        std::string_view loc_tag = locale_str->ConvertToStringView(&buf);
        icu::StringPiece sp {loc_tag.data(), static_cast<int32_t>(loc_tag.size())};
        loc = icu::Locale::forLanguageTag(sp, status);
        if (UNLIKELY(U_FAILURE(status))) {
            EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
            std::string message = "Language tag '" + std::string(loc_tag) + "' is invalid or not supported";
            ThrowEtsException(coroutine, panda_file_items::class_descriptors::RANGE_ERROR, message);
            return 0;
        }
    }
    icu::UnicodeString source;
    if (this_str->IsUtf16()) {
        source = icu::UnicodeString {this_str->GetDataUtf16(), static_cast<int32_t>(this_str->GetUtf16Length())};
    } else {
        source = icu::UnicodeString {utf::Mutf8AsCString(this_str->GetDataMUtf8()),
                                     static_cast<int32_t>(this_str->GetLength())};
    }
    icu::UnicodeString target;
    if (cmp_str->IsUtf16()) {
        target = icu::UnicodeString {cmp_str->GetDataUtf16(), static_cast<int32_t>(cmp_str->GetUtf16Length())};
    } else {
        target = icu::UnicodeString {utf::Mutf8AsCString(cmp_str->GetDataMUtf8()),
                                     static_cast<int32_t>(cmp_str->GetLength())};
    }
    status = U_ZERO_ERROR;
    std::unique_ptr<icu::Collator> my_collator(icu::Collator::createInstance(loc, status));
    if (UNLIKELY(U_FAILURE(status))) {
        icu::UnicodeString disp_name;
        loc.getDisplayName(disp_name);
        std::string locale_name;
        disp_name.toUTF8String(locale_name);
        LOG(FATAL, ETS) << "Failed to create the collator for " << locale_name;
    }
    return my_collator->compare(source, target);
}

ets_int StdCoreStringIndexOfAfter(EtsString *s, uint16_t ch, ets_int from_index)
{
    return panda::intrinsics::StringIndexOfU16(s, ch, from_index);
}

ets_int StdCoreStringIndexOf(EtsString *s, uint16_t ch)
{
    return StdCoreStringIndexOfAfter(s, ch, 0);
}

ets_int StdCoreStringIndexOfString(EtsString *this_str, EtsString *pattern_str, ets_int from_index)
{
    ASSERT(this_str != nullptr && pattern_str != nullptr);
    return this_str->GetCoreType()->IndexOf(pattern_str->GetCoreType(), from_index);
}

ets_int StdCoreStringLastIndexOfString(EtsString *this_str, EtsString *pattern_str, ets_int from_index)
{
    ASSERT(this_str != nullptr && pattern_str != nullptr);
    // "abc".lastIndexOf("ab", -10) will return 0
    return this_str->GetCoreType()->LastIndexOf(pattern_str->GetCoreType(), std::max(from_index, 0));
}

}  // namespace panda::ets::intrinsics
