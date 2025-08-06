/**
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

#include <algorithm>
#include <cstdint>
#include <regex>
#include "include/mem/panda_string.h"
#include "include/object_header.h"
#include "intrinsics.h"
#include "libpandabase/utils/logger.h"
#include "macros.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/entrypoints/string_index_of.h"
#include "runtime/arch/memory_helpers.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_language_context.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"

#include "unicode/locid.h"
#include "unicode/coll.h"
#include "unicode/unistr.h"
#include "unicode/normalizer2.h"
#include "utils/span.h"

using icu::Normalizer2;

namespace ark::ets::intrinsics {

constexpr const uint32_t CHAR0X1FFC00 = 0x1ffc00;
constexpr const uint16_t CHAR0XD800 = 0xd800;
constexpr const uint16_t CHAR0XDC00 = 0xdc00;

static bool CheckStringIndex(EtsString *s, EtsInt begin, EtsInt end)
{
    ASSERT(s != nullptr);
    EtsInt length = s->GetLength();
    if (UNLIKELY(begin > end)) {
        ark::ThrowStringIndexOutOfBoundsException(begin, length);
        return false;
    }
    if (UNLIKELY(begin > length || begin < 0)) {
        ark::ThrowStringIndexOutOfBoundsException(begin, length);
        return false;
    }
    if (UNLIKELY(end > length)) {
        ark::ThrowStringIndexOutOfBoundsException(end, length);
        return false;
    }
    return true;
}

static ObjectHeader *StdCoreStringGetDataAsArray(EtsString *s, EtsInt begin, EtsInt end, bool isUtf16)
{
    if (!CheckStringIndex(s, begin, end)) {
        return nullptr;
    }
    EtsInt length = s->GetLength();
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    VMHandle<EtsString> sHandle(coroutine, reinterpret_cast<ObjectHeader *>(s));
    EtsInt n = end - begin;
    void *array = nullptr;
    if (isUtf16) {
        array = EtsCharArray::Create(n);
    } else {
        array = EtsByteArray::Create(n);
    }
    if (array == nullptr || n == 0) {
        return reinterpret_cast<ObjectHeader *>(array);
    }
    if (isUtf16) {
        auto charArray = reinterpret_cast<EtsCharArray *>(array);
        Span<uint16_t> out(charArray->GetData<uint16_t>(), charArray->GetLength());
        sHandle->CopyDataRegionUtf16(&(out[0]), begin, n, n);
    } else {
        auto byteArray = reinterpret_cast<EtsByteArray *>(array);
        Span<uint8_t> out(byteArray->GetData<uint8_t>(), byteArray->GetLength());

        /* as we need only one LSB no sophisticated conversion is needed */
        if (sHandle->IsUtf16()) {
            PandaVector<uint16_t> in(length);
            sHandle->CopyDataUtf16(in.data(), length);
            for (int i = 0; i < n; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                out[i] = in[i + begin];
            }
        } else {
            PandaVector<uint8_t> in(length);
            sHandle->CopyDataRegionUtf8(in.data(), 0, length, length);
            for (int i = 0; i < n; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                out[i] = in[i + begin];
            }
        }
    }
    return reinterpret_cast<ObjectHeader *>(array);
}

ObjectHeader *StdCoreStringGetChars(EtsString *s, EtsInt begin, EtsInt end)
{
    return StdCoreStringGetDataAsArray(s, begin, end, true);
}

ObjectHeader *StdCoreStringGetBytes(EtsString *s, EtsInt begin, EtsInt end)
{
    return StdCoreStringGetDataAsArray(s, begin, end, false);
}

static std::pair<int32_t, int32_t> NormalizeSubStringIndexes(int32_t beginIndex, int32_t endIndex, EtsString *str)
{
    auto strLen = str->GetLength();
    std::pair<int32_t, int32_t> normIndexes = {beginIndex, endIndex};

    // If begin_index < 0, then it is assumed to be equal to zero.
    if (normIndexes.first < 0) {
        normIndexes.first = 0;
    } else if (static_cast<decltype(strLen)>(normIndexes.first) > strLen) {
        // If begin_index > str_len, then it is assumed to be equal to str_len.
        normIndexes.first = static_cast<int32_t>(strLen);
    }
    // If end_index < 0, then it is assumed to be equal to zero.
    if (normIndexes.second < 0) {
        normIndexes.second = 0;
    } else if (static_cast<decltype(strLen)>(normIndexes.second) > strLen) {
        // If end_index > str_len, then it is assumed to be equal to str_len.
        normIndexes.second = static_cast<int32_t>(strLen);
    }
    // If begin_index > end_index, then these are swapped.
    if (normIndexes.first > normIndexes.second) {
        std::swap(normIndexes.first, normIndexes.second);
    }
    ASSERT((normIndexes.second - normIndexes.first) >= 0);
    return normIndexes;
}

EtsString *StdCoreStringSubstring(EtsString *str, EtsInt begin, EtsInt end)
{
    ASSERT(str != nullptr);
    auto indexes = NormalizeSubStringIndexes(begin, end, str);
    if (UNLIKELY(indexes.first == 0 && indexes.second == str->GetLength())) {
        return str;
    }
    EtsInt substrLength = indexes.second - indexes.first;
    return EtsString::FastSubString(str, static_cast<uint32_t>(indexes.first), static_cast<uint32_t>(substrLength));
}

uint16_t StdCoreStringCharAt(EtsString *s, int32_t index)
{
    ASSERT(s != nullptr);

    int32_t length = s->GetLength();
    if (UNLIKELY(index >= length || index < 0)) {
        ark::ThrowStringIndexOutOfBoundsException(index, length);
        return 0;
    }
    return s->At(index);
}

int32_t StdCoreStringGetLength(EtsString *s)
{
    ASSERT(s != nullptr);
    return s->GetLength();
}

double StdCoreStringLength(EtsString *s)
{
    ASSERT(s != nullptr);
    return static_cast<double>(s->GetLength());
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

EtsString *StringNormalize(EtsString *str, const Normalizer2 *normalizer)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    VMHandle<coretypes::String> strHandle(coroutine, reinterpret_cast<ObjectHeader *>(str));
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto flatStringInfo = coretypes::FlatStringInfo::FlattenAllString(strHandle, ctx);
    icu::UnicodeString utf16Str;
    if (flatStringInfo.IsUtf16()) {
        utf16Str =
            icu::UnicodeString {flatStringInfo.GetDataUtf16(), static_cast<int32_t>(strHandle->GetUtf16Length())};
    } else {
        utf16Str = icu::UnicodeString {utf::Mutf8AsCString(flatStringInfo.GetDataUtf8()),
                                       static_cast<int32_t>(flatStringInfo.GetLength())};
    }

    UErrorCode errorCode = U_ZERO_ERROR;
    utf16Str = normalizer->normalize(utf16Str, errorCode);

    if (UNLIKELY(U_FAILURE(errorCode))) {
        std::string message = "Got error in process of normalization: '" + std::string(u_errorName(errorCode)) + "'";
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }

    return EtsString::CreateFromUtf16(reinterpret_cast<const uint16_t *>(utf16Str.getTerminatedBuffer()),
                                      utf16Str.length());
}

EtsString *StdCoreStringNormalizeNFC(EtsString *thisStr)
{
    UErrorCode errorCode = U_ZERO_ERROR;
    auto normalizer = Normalizer2::getNFCInstance(errorCode);
    if (UNLIKELY(U_FAILURE(errorCode))) {
        std::string message = "Cannot get NFC normalizer: '" + std::string(u_errorName(errorCode)) + "'";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return StringNormalize(thisStr, normalizer);
}

EtsString *StdCoreStringNormalizeNFD(EtsString *thisStr)
{
    UErrorCode errorCode = U_ZERO_ERROR;
    auto normalizer = Normalizer2::getNFDInstance(errorCode);
    if (UNLIKELY(U_FAILURE(errorCode))) {
        std::string message = "Cannot get NFD normalizer: '" + std::string(u_errorName(errorCode)) + "'";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return StringNormalize(thisStr, normalizer);
}

EtsString *StdCoreStringNormalizeNFKC(EtsString *thisStr)
{
    UErrorCode errorCode = U_ZERO_ERROR;
    auto normalizer = Normalizer2::getNFKCInstance(errorCode);
    if (UNLIKELY(U_FAILURE(errorCode))) {
        std::string message = "Cannot get NFKC normalizer: '" + std::string(u_errorName(errorCode)) + "'";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return StringNormalize(thisStr, normalizer);
}

EtsString *StdCoreStringNormalizeNFKD(EtsString *thisStr)
{
    UErrorCode errorCode = U_ZERO_ERROR;
    auto normalizer = Normalizer2::getNFKDInstance(errorCode);
    if (UNLIKELY(U_FAILURE(errorCode))) {
        std::string message = "Cannot get NFKD normalizer: '" + std::string(u_errorName(errorCode)) + "'";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return StringNormalize(thisStr, normalizer);
}

uint8_t StdCoreStringIsWellFormed(EtsString *thisStr)
{
    if (!thisStr->IsUtf16()) {
        return UINT8_C(1);
    }
    auto length = thisStr->GetUtf16Length();
    for (size_t i = 0; i < length; ++i) {
        uint16_t codeUnit = thisStr->At(i);
        if ((codeUnit & CHAR0X1FFC00) == CHAR0XD800) {
            // Code unit is a leading surrogate
            if (i == length - 1) {
                return UINT8_C(0);
            }
            // Is not trail surrogate
            if ((thisStr->At(i + 1) & CHAR0X1FFC00) != CHAR0XDC00) {
                return UINT8_C(0);
            }
            // Skip the paired trailing surrogate
            ++i;
            // Is trail surrogate
        } else if ((codeUnit & CHAR0X1FFC00) == CHAR0XDC00) {
            return UINT8_C(0);
        }
    }
    return UINT8_C(1);
}

EtsString *ToLowerCase(EtsString *thisStr, const icu::Locale &locale)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    VMHandle<coretypes::String> strHandle(coroutine, thisStr->GetCoreType());
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto flatStringInfo = coretypes::FlatStringInfo::FlattenAllString(strHandle, ctx);
    icu::UnicodeString utf16Str;
    if (flatStringInfo.IsUtf16()) {
        utf16Str =
            icu::UnicodeString {flatStringInfo.GetDataUtf16(), static_cast<int32_t>(strHandle->GetUtf16Length())};
    } else {
        utf16Str = icu::UnicodeString {utf::Mutf8AsCString(flatStringInfo.GetDataUtf8()),
                                       static_cast<int32_t>(flatStringInfo.GetLength())};
    }
    auto res = utf16Str.toLower(locale);
    return EtsString::CreateFromUtf16(reinterpret_cast<const uint16_t *>(res.getTerminatedBuffer()), res.length());
}

EtsString *ToUpperCase(EtsString *thisStr, const icu::Locale &locale)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    VMHandle<coretypes::String> strHandle(coroutine, thisStr->GetCoreType());
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto flatStringInfo = coretypes::FlatStringInfo::FlattenAllString(strHandle, ctx);
    icu::UnicodeString utf16Str;
    if (flatStringInfo.IsUtf16()) {
        utf16Str =
            icu::UnicodeString {flatStringInfo.GetDataUtf16(), static_cast<int32_t>(strHandle->GetUtf16Length())};
    } else {
        utf16Str = icu::UnicodeString {utf::Mutf8AsCString(flatStringInfo.GetDataUtf8()),
                                       static_cast<int32_t>(flatStringInfo.GetLength())};
    }
    auto res = utf16Str.toUpper(locale);
    return EtsString::CreateFromUtf16(reinterpret_cast<const uint16_t *>(res.getTerminatedBuffer()), res.length());
}

UErrorCode ParseSingleBCP47LanguageTag(EtsString *langTag, icu::Locale &locale)
{
    if (langTag == nullptr) {
        locale = icu::Locale::getDefault();
        return U_ZERO_ERROR;
    }
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    VMHandle<coretypes::String> langTagHandle(coroutine, reinterpret_cast<ObjectHeader *>(langTag));
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto flatStringInfo = coretypes::FlatStringInfo::FlattenAllString(langTagHandle, ctx);

    PandaVector<uint8_t> buf;
    std::string_view locTag = EtsString::FromCoreType(flatStringInfo.GetString())->ConvertToStringView(&buf);
    icu::StringPiece sp {locTag.data(), static_cast<int32_t>(locTag.size())};
    UErrorCode status = U_ZERO_ERROR;
    locale = icu::Locale::forLanguageTag(sp, status);
    return status;
}

EtsString *StdCoreStringToUpperCase(EtsString *thisStr)
{
    return ToUpperCase(thisStr, icu::Locale::getDefault());
}

EtsString *StdCoreStringToLowerCase(EtsString *thisStr)
{
    return ToLowerCase(thisStr, icu::Locale::getDefault());
}

EtsString *StdCoreStringToLocaleUpperCase(EtsString *thisStr, EtsString *langTag)
{
    ASSERT(langTag != nullptr);
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    icu::Locale locale;
    VMHandle<EtsString> langTagHandle(coroutine, langTag->GetCoreType());
    VMHandle<EtsString> thisStrHandle(coroutine, thisStr->GetCoreType());
    auto localeParseStatus = ParseSingleBCP47LanguageTag(langTag, locale);
    if (UNLIKELY(U_FAILURE(localeParseStatus))) {
        auto message =
            "Language tag '" + ConvertToString(langTagHandle->GetCoreType()) + "' is invalid or not supported";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return ToUpperCase(thisStrHandle.GetPtr(), locale);
}

EtsString *StdCoreStringToLocaleLowerCase(EtsString *thisStr, EtsString *langTag)
{
    ASSERT(langTag != nullptr);
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    icu::Locale locale;
    VMHandle<EtsString> langTagHandle(coroutine, langTag->GetCoreType());
    VMHandle<EtsString> thisStrHandle(coroutine, thisStr->GetCoreType());
    auto localeParseStatus = ParseSingleBCP47LanguageTag(langTag, locale);
    if (UNLIKELY(U_FAILURE(localeParseStatus))) {
        auto message =
            "Language tag '" + ConvertToString(langTagHandle->GetCoreType()) + "' is invalid or not supported";
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }
    return ToLowerCase(thisStrHandle.GetPtr(), locale);
}

EtsInt StdCoreStringIndexOfAfter(EtsString *s, uint16_t ch, EtsInt fromIndex)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    return ark::intrinsics::StringIndexOfU16(s, ch, fromIndex, ctx);
}

EtsInt StdCoreStringIndexOf(EtsString *s, uint16_t ch)
{
    return StdCoreStringIndexOfAfter(s, ch, 0);
}

EtsInt StdCoreStringIndexOfString(EtsString *thisStr, EtsString *patternStr, EtsInt fromIndex)
{
    ASSERT(thisStr != nullptr && patternStr != nullptr);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    return thisStr->GetCoreType()->IndexOf(patternStr->GetCoreType(), ctx, fromIndex);
}

EtsInt StdCoreStringLastIndexOfString(EtsString *thisStr, EtsString *patternStr, EtsInt fromIndex)
{
    ASSERT(thisStr != nullptr && patternStr != nullptr);
    // "abc".lastIndexOf("ab", -10) will return 0
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    return thisStr->GetCoreType()->LastIndexOf(patternStr->GetCoreType(), ctx, std::max(fromIndex, 0));
}

EtsInt StdCoreStringCodePointToChar(EtsInt codePoint)
{
    icu::UnicodeString uniStr((UChar32)codePoint);
    uint32_t ret = bit_cast<uint16_t>(uniStr.charAt(0));
    // if codepoint contains a surrogate pair
    // encode it into int with higher bits being second char
    if (uniStr.length() > 1) {
        constexpr uint32_t BITS_IN_CHAR = 16;
        ret |= static_cast<uint32_t>(bit_cast<uint16_t>(uniStr.charAt(1))) << BITS_IN_CHAR;
    }
    return bit_cast<EtsInt>(ret);
}

int32_t StdCoreStringHashCode(EtsString *thisStr)
{
    ASSERT(thisStr != nullptr);
    return thisStr->GetHashcode();
}

EtsBoolean StdCoreStringIsCompressed(EtsString *thisStr)
{
    ASSERT(thisStr != nullptr);
    return ToEtsBoolean(thisStr->IsMUtf8());
}

static coretypes::String *StringConcat2(coretypes::String *str1, coretypes::String *str2)
{
    auto *vm = Runtime::GetCurrent()->GetPandaVM();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    return coretypes::String::Concat(str1, str2, ctx, vm);
}

static coretypes::String *StringConcat3(coretypes::String *str1, coretypes::String *str2, coretypes::String *str3)
{
    auto *vm = Runtime::GetCurrent()->GetPandaVM();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    VMHandle<coretypes::String> str3Handle(coroutine, str3);
    auto str = coretypes::String::Concat(str1, str2, ctx, vm);
    if (UNLIKELY(str == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    str = coretypes::String::Concat(str, str3Handle.GetPtr(), ctx, vm);
    return str;
}

static coretypes::String *StringConcat4(coretypes::String *str1, coretypes::String *str2, coretypes::String *str3,
                                        coretypes::String *str4)
{
    auto *vm = Runtime::GetCurrent()->GetPandaVM();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    VMHandle<coretypes::String> str3Handle(coroutine, str3);
    VMHandle<coretypes::String> str4Handle(coroutine, str4);
    auto str = coretypes::String::Concat(str1, str2, ctx, vm);
    if (UNLIKELY(str == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    str3 = str3Handle.GetPtr();
    str = coretypes::String::Concat(str, str3, ctx, vm);
    if (UNLIKELY(str == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    str4 = str4Handle.GetPtr();
    str = coretypes::String::Concat(str, str4, ctx, vm);
    return str;
}

EtsString *StdCoreStringConcat2(EtsString *str1, EtsString *str2)
{
    auto s1 = str1->GetCoreType();
    auto s2 = str2->GetCoreType();
    return reinterpret_cast<EtsString *>(StringConcat2(s1, s2));
}

EtsString *StdCoreStringConcat3(EtsString *str1, EtsString *str2, EtsString *str3)
{
    auto s1 = str1->GetCoreType();
    auto s2 = str2->GetCoreType();
    auto s3 = str3->GetCoreType();
    return reinterpret_cast<EtsString *>(StringConcat3(s1, s2, s3));
}

EtsString *StdCoreStringConcat4(EtsString *str1, EtsString *str2, EtsString *str3, EtsString *str4)
{
    auto s1 = str1->GetCoreType();
    auto s2 = str2->GetCoreType();
    auto s3 = str3->GetCoreType();
    auto s4 = str4->GetCoreType();
    return reinterpret_cast<EtsString *>(StringConcat4(s1, s2, s3, s4));
}

EtsInt StdCoreStringCompareTo(EtsString *str1, EtsString *str2)
{
    /* corner cases */
    if (str1->GetLength() == 0) {
        return -str2->GetLength();
    }
    if (str2->GetLength() == 0) {
        return str1->GetLength();
    }

    /* use the default implementation otherwise */
    return str1->Compare(str2);
}

EtsString *StdCoreStringTrimLeft(EtsString *thisStr)
{
    return thisStr->TrimLeft();
}

EtsString *StdCoreStringTrimRight(EtsString *thisStr)
{
    return thisStr->TrimRight();
}

EtsString *StdCoreStringTrim(EtsString *thisStr)
{
    return thisStr->Trim();
}

EtsBoolean StdCoreStringStartsWith(EtsString *thisStr, EtsString *prefix, EtsInt fromIndex)
{
    ASSERT(thisStr != nullptr);
    return thisStr->StartsWith(prefix, fromIndex);
}

EtsBoolean StdCoreStringEndsWith(EtsString *thisStr, EtsString *suffix, EtsInt endIndex)
{
    ASSERT(thisStr != nullptr);
    return thisStr->EndsWith(suffix, endIndex);
}

EtsString *StdCoreStringFromCharCode(EtsEscompatArray *charCodes)
{
    ASSERT(charCodes != nullptr);
    ASSERT(charCodes->GetData() != nullptr);
    return EtsString::CreateNewStringFromCharCode(charCodes->GetData(), charCodes->GetActualLength());
}

EtsString *StdCoreStringFromCharCodeSingle(EtsDouble charCode)
{
    if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
        constexpr double UTF16_CHAR_DIVIDER = 0x10000;
        auto character = static_cast<uint16_t>(static_cast<int64_t>(std::fmod(charCode, UTF16_CHAR_DIVIDER)));
        if (character < EtsPlatformTypes::ASCII_CHAR_TABLE_SIZE && coretypes::String::IsASCIICharacter(character)) {
            auto *cache = PlatformTypes()->GetAsciiCacheTable();
            return static_cast<EtsString *>(cache->Get(character));
        }
    }
    return EtsString::CreateNewStringFromCharCode(charCode);
}

/* the allocation routine to create an unitialized string of the given size */
extern "C" EtsString *AllocateStringObject(size_t length, bool compressed)
{
    return EtsString::AllocateNonInitializedString(length, compressed);
}

EtsString *StdCoreStringRepeat(EtsString *str, EtsInt count)
{
    auto length = str->GetLength();
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    if (UNLIKELY(count < 0)) {
        PandaString message = "repeat: count is negative";
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::RANGE_ERROR, message);
        return nullptr;
    }

    if (length == 0 || count == 0) {
        return EtsString::CreateNewEmptyString();
    }
    VMHandle<EtsString> sHandle(coroutine, reinterpret_cast<ObjectHeader *>(str));

    int size = length * count;
    auto compressed = str->IsMUtf8();
    auto result = AllocateStringObject(size, compressed);
    if (UNLIKELY(result == nullptr)) {
        PandaString message = "repeat: memory allocation failed";
        auto coro = EtsCoroutine::GetCurrent();
        ThrowEtsException(coro, panda_file_items::class_descriptors::OUT_OF_MEMORY_ERROR, message);
        return nullptr;
    }

    for (uint32_t i = 0; i < static_cast<uint32_t>(count); i++) {
        EtsString::ReadData(result, sHandle.GetPtr(), i * length, (count - i) * length, length);
    }
    return result;
}

EtsString *StdCoreStringGet(EtsString *str, EtsInt index)
{
    auto strData = static_cast<uint16_t>(StdCoreStringCharAt(str, index));
    auto *coro = EtsCoroutine::GetCurrent();
    if (coro->HasPendingException()) {
        return nullptr;
    }
    if (strData < EtsPlatformTypes::ASCII_CHAR_TABLE_SIZE && coretypes::String::IsASCIICharacter(strData)) {
        auto *cache = PlatformTypes()->GetAsciiCacheTable();
        return static_cast<EtsString *>(cache->Get(strData));
    }
    return EtsString::CreateFromUtf16(&strData, 1);
}
}  // namespace ark::ets::intrinsics
