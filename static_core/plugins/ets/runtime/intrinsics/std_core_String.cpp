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
#include <cstdint>
#include <regex>
#include "include/mem/panda_string.h"
#include "intrinsics.h"
#include "libpandabase/utils/logger.h"
#include "macros.h"
#include "napi/ets_napi.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/entrypoints/string_index_of.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_language_context.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"

#include "unicode/locid.h"
#include "unicode/coll.h"
#include "unicode/unistr.h"
#include "unicode/normalizer2.h"
#include "utils/span.h"

using icu::Normalizer2;

namespace panda::ets::intrinsics {

constexpr const uint32_t CHAR0X1FFC00 = 0x1ffc00;
constexpr const uint16_t CHAR0XD800 = 0xd800;
constexpr const uint16_t CHAR0XDC00 = 0xdc00;

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

EtsString *StdCoreStringSubstring(EtsString *str, ets_int begin, ets_int end)
{
    ASSERT(str != nullptr);
    auto indexes = coretypes::String::NormalizeSubStringIndexes(begin, end, str->GetCoreType());
    ets_int substr_length = indexes.second - indexes.first;
    return EtsString::FastSubString(str, static_cast<uint32_t>(indexes.first), static_cast<uint32_t>(substr_length));
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

int32_t StdCoreStringGetLength(EtsString *s)
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

EtsString *StringNormalize(EtsString *str, const Normalizer2 *normalizer)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    icu::UnicodeString utf16_str;
    if (str->IsUtf16()) {
        utf16_str = icu::UnicodeString {str->GetDataUtf16(), static_cast<int32_t>(str->GetUtf16Length())};
    } else {
        utf16_str =
            icu::UnicodeString {utf::Mutf8AsCString(str->GetDataMUtf8()), static_cast<int32_t>(str->GetLength())};
    }

    UErrorCode error_code = U_ZERO_ERROR;
    utf16_str = normalizer->normalize(utf16_str, error_code);

    if (UNLIKELY(U_FAILURE(error_code))) {
        std::string message = "Got error in process of normalization: '" + std::string(u_errorName(error_code)) + "'";
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }

    return EtsString::CreateFromUtf16(reinterpret_cast<const uint16_t *>(utf16_str.getTerminatedBuffer()),
                                      utf16_str.length());
}

EtsString *StdCoreStringNormalizeNFC(EtsString *this_str)
{
    UErrorCode error_code = U_ZERO_ERROR;
    auto normalizer = Normalizer2::getNFCInstance(error_code);
    if (UNLIKELY(U_FAILURE(error_code))) {
        std::string message = "Cannot get NFC normalizer: '" + std::string(u_errorName(error_code)) + "'";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return StringNormalize(this_str, normalizer);
}

EtsString *StdCoreStringNormalizeNFD(EtsString *this_str)
{
    UErrorCode error_code = U_ZERO_ERROR;
    auto normalizer = Normalizer2::getNFDInstance(error_code);
    if (UNLIKELY(U_FAILURE(error_code))) {
        std::string message = "Cannot get NFD normalizer: '" + std::string(u_errorName(error_code)) + "'";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return StringNormalize(this_str, normalizer);
}

EtsString *StdCoreStringNormalizeNFKC(EtsString *this_str)
{
    UErrorCode error_code = U_ZERO_ERROR;
    auto normalizer = Normalizer2::getNFKCInstance(error_code);
    if (UNLIKELY(U_FAILURE(error_code))) {
        std::string message = "Cannot get NFKC normalizer: '" + std::string(u_errorName(error_code)) + "'";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return StringNormalize(this_str, normalizer);
}

EtsString *StdCoreStringNormalizeNFKD(EtsString *this_str)
{
    UErrorCode error_code = U_ZERO_ERROR;
    auto normalizer = Normalizer2::getNFKDInstance(error_code);
    if (UNLIKELY(U_FAILURE(error_code))) {
        std::string message = "Cannot get NFKD normalizer: '" + std::string(u_errorName(error_code)) + "'";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return StringNormalize(this_str, normalizer);
}

uint8_t StdCoreStringIsWellFormed(EtsString *this_str)
{
    if (!this_str->IsUtf16()) {
        return UINT8_C(1);
    }
    auto length = this_str->GetUtf16Length();
    auto code_units = Span<uint16_t>(this_str->GetDataUtf16(), length);
    for (size_t i = 0; i < length; ++i) {
        uint16_t code_unit = code_units[i];
        if ((code_unit & CHAR0X1FFC00) == CHAR0XD800) {
            // Code unit is a leading surrogate
            if (i == length - 1) {
                return UINT8_C(0);
            }
            // Is not trail surrogate
            if ((code_units[i + 1] & CHAR0X1FFC00) != CHAR0XDC00) {
                return UINT8_C(0);
            }
            // Skip the paired trailing surrogate
            ++i;
            // Is trail surrogate
        } else if ((code_unit & CHAR0X1FFC00) == CHAR0XDC00) {
            return UINT8_C(0);
        }
    }
    return UINT8_C(1);
}

EtsString *StdCoreStringToLocaleLowerCase(EtsString *this_str, EtsString *locale)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    icu::Locale loc;
    UErrorCode status = U_ZERO_ERROR;
    if (locale != nullptr) {
        PandaVector<uint8_t> buf;
        std::string_view loc_tag = locale->ConvertToStringView(&buf);
        icu::StringPiece sp {loc_tag.data(), static_cast<int32_t>(loc_tag.size())};
        loc = icu::Locale::forLanguageTag(sp, status);
        if (UNLIKELY(U_FAILURE(status))) {
            std::string message = "Language tag '" + std::string(loc_tag) + "' is invalid or not supported";
            ThrowEtsException(coroutine, panda_file_items::class_descriptors::RANGE_ERROR, message);
            return nullptr;
        }
    }

    icu::UnicodeString utf16_str;
    if (this_str->IsUtf16()) {
        utf16_str = icu::UnicodeString {this_str->GetDataUtf16(), static_cast<int32_t>(this_str->GetUtf16Length())};
    } else {
        utf16_str = icu::UnicodeString {utf::Mutf8AsCString(this_str->GetDataMUtf8()),
                                        static_cast<int32_t>(this_str->GetLength())};
    }

    auto res = utf16_str.toLower(loc);
    return EtsString::CreateFromUtf16(reinterpret_cast<const uint16_t *>(res.getTerminatedBuffer()), res.length());
}

EtsString *StdCoreStringToLocaleUpperCase(EtsString *this_str, EtsString *locale)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    icu::Locale loc;
    UErrorCode status = U_ZERO_ERROR;
    if (locale != nullptr) {
        PandaVector<uint8_t> buf;
        std::string_view loc_tag = locale->ConvertToStringView(&buf);
        icu::StringPiece sp {loc_tag.data(), static_cast<int32_t>(loc_tag.size())};
        loc = icu::Locale::forLanguageTag(sp, status);
        if (UNLIKELY(U_FAILURE(status))) {
            std::string message = "Language tag '" + std::string(loc_tag) + "' is invalid or not supported";
            ThrowEtsException(coroutine, panda_file_items::class_descriptors::RANGE_ERROR, message);
            return nullptr;
        }
    }

    icu::UnicodeString utf16_str;
    if (this_str->IsUtf16()) {
        utf16_str = icu::UnicodeString {this_str->GetDataUtf16(), static_cast<int32_t>(this_str->GetUtf16Length())};
    } else {
        utf16_str = icu::UnicodeString {utf::Mutf8AsCString(this_str->GetDataMUtf8()),
                                        static_cast<int32_t>(this_str->GetLength())};
    }

    auto res = utf16_str.toUpper(loc);
    return EtsString::CreateFromUtf16(reinterpret_cast<const uint16_t *>(res.getTerminatedBuffer()), res.length());
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

ets_char StdCoreStringCodePointToChar(ets_int code_point)
{
    icu::UnicodeString uni_str((UChar32)code_point);
    return static_cast<ets_char>(uni_str.charAt(0));
}

int32_t StdCoreStringHashCode(EtsString *this_str)
{
    ASSERT(this_str != nullptr);
    return this_str->GetCoreType()->GetHashcode();
}

}  // namespace panda::ets::intrinsics
