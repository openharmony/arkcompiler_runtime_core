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

#include <cstddef>
#include <cstring>
#include <limits>

#include "libpandabase/utils/utf.h"
#include "libpandabase/utils/hash.h"
#include "libpandabase/utils/span.h"
#include "runtime/arch/memory_helpers.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/string-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/handle_base-inl.h"
#include "runtime/include/panda_vm.h"

namespace panda::coretypes {

bool String::compressed_strings_enabled_ = true;

/* static */
String *String::CreateFromString(String *str, const LanguageContext &ctx, PandaVM *vm)
{
    ASSERT(str != nullptr);
    // allocator may trig gc and move str, need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> str_handle(thread, str);
    auto string = AllocStringObject(str_handle->GetLength(), !str_handle->IsUtf16(), ctx, vm);
    if (string == nullptr) {
        return nullptr;
    }

    // retrive str after gc
    str = str_handle.GetPtr();
    string->hashcode_ = str->hashcode_;

    uint32_t length = str->GetLength();
    // After memcpy we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    if (str->IsUtf16()) {
        memcpy_s(string->GetDataUtf16(), ComputeDataSizeUtf16(string->GetLength()), str->GetDataUtf16(),
                 ComputeDataSizeUtf16(length));
    } else {
        memcpy_s(string->GetDataMUtf8(), string->GetLength(), str->GetDataMUtf8(), length);
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();

    return string;
}

/* static */
String *String::CreateFromMUtf8(const uint8_t *mutf8_data, size_t mutf8_length, uint32_t utf16_length,
                                bool can_be_compressed, const LanguageContext &ctx, PandaVM *vm, bool movable)
{
    auto string = AllocStringObject(utf16_length, can_be_compressed, ctx, vm, movable);
    if (string == nullptr) {
        return nullptr;
    }

    ASSERT(string->hashcode_ == 0);
    // After copying we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    if (can_be_compressed) {
        memcpy_s(string->GetDataMUtf8(), string->GetLength(), mutf8_data, utf16_length);
    } else {
        utf::ConvertMUtf8ToUtf16(mutf8_data, mutf8_length, string->GetDataUtf16());
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();
    return string;
}

/* static */
String *String::CreateFromMUtf8(const uint8_t *mutf8_data, uint32_t utf16_length, const LanguageContext &ctx,
                                PandaVM *vm, bool movable)
{
    bool can_be_compressed = CanBeCompressedMUtf8(mutf8_data);
    return CreateFromMUtf8(mutf8_data, utf::Mutf8Size(mutf8_data), utf16_length, can_be_compressed, ctx, vm, movable);
}

/* static */
String *String::CreateFromMUtf8(const uint8_t *mutf8_data, uint32_t utf16_length, bool can_be_compressed,
                                const LanguageContext &ctx, PandaVM *vm, bool movable)
{
    return CreateFromMUtf8(mutf8_data, utf::Mutf8Size(mutf8_data), utf16_length, can_be_compressed, ctx, vm, movable);
}

/* static */
String *String::CreateFromMUtf8(const uint8_t *mutf8_data, const LanguageContext &ctx, PandaVM *vm, bool movable)
{
    size_t mutf8_length = utf::Mutf8Size(mutf8_data);
    size_t utf16_length = utf::MUtf8ToUtf16Size(mutf8_data, mutf8_length);
    bool can_be_compressed = CanBeCompressedMUtf8(mutf8_data);
    return CreateFromMUtf8(mutf8_data, mutf8_length, utf16_length, can_be_compressed, ctx, vm, movable);
}

/* static */
String *String::CreateFromUtf8(const uint8_t *utf8_data, uint32_t utf8_length, const LanguageContext &ctx, PandaVM *vm,
                               bool movable)
{
    coretypes::String *s = nullptr;
    auto utf16_length = utf::Utf8ToUtf16Size(utf8_data, utf8_length);
    if (CanBeCompressedMUtf8(utf8_data, utf8_length)) {
        // ascii string have equal representation in utf8 and mutf8 formats
        s = coretypes::String::CreateFromMUtf8(utf8_data, utf8_length, utf16_length, true, ctx, vm, movable);
    } else {
        PandaVector<uint16_t> tmp_buffer(utf16_length);
        [[maybe_unused]] auto len =
            utf::ConvertRegionUtf8ToUtf16(utf8_data, tmp_buffer.data(), utf8_length, utf16_length, 0);
        ASSERT(len == utf16_length);
        s = coretypes::String::CreateFromUtf16(tmp_buffer.data(), utf16_length, ctx, vm, movable);
    }
    return s;
}

/* static */
String *String::CreateFromUtf16(const uint16_t *utf16_data, uint32_t utf16_length, const LanguageContext &ctx,
                                PandaVM *vm, bool movable)
{
    bool can_be_compressed = CanBeCompressed(utf16_data, utf16_length);
    auto string = AllocStringObject(utf16_length, can_be_compressed, ctx, vm, movable);
    if (string == nullptr) {
        return nullptr;
    }

    ASSERT(string->hashcode_ == 0);
    // After copying we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    if (can_be_compressed) {
        CopyUtf16AsMUtf8(utf16_data, string->GetDataMUtf8(), utf16_length);
    } else {
        memcpy_s(string->GetDataUtf16(), ComputeDataSizeUtf16(string->GetLength()), utf16_data, utf16_length << 1UL);
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();
    return string;
}

/* static */
String *String::CreateEmptyString(const LanguageContext &ctx, PandaVM *vm)
{
    uint16_t data = 0;
    return CreateFromUtf16(&data, 0, ctx, vm);
}

/* static */
void String::CopyUtf16AsMUtf8(const uint16_t *utf16_from, uint8_t *mutf8_to, uint32_t utf16_length)
{
    Span<const uint16_t> from(utf16_from, utf16_length);
    Span<uint8_t> to(mutf8_to, utf16_length);
    for (uint32_t i = 0; i < utf16_length; i++) {
        to[i] = from[i];
    }
}

// static
String *String::CreateNewStringFromChars(uint32_t offset, uint32_t length, Array *chararray, const LanguageContext &ctx,
                                         PandaVM *vm)
{
    ASSERT(chararray != nullptr);
    // allocator may trig gc and move array, need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<Array> array_handle(thread, chararray);

    // There is a potential data race between read of src in CanBeCompressed and write of destination buf
    // in CopyDataRegionUtf16. The src is a cast from chararray comming from managed object.
    // Hence the race is reported on managed object, which has a synchronization on a high level.
    // TSAN does not see such synchronization, thus we ignore such races here.
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    // NOLINTNEXTLINE(readability-identifier-naming)
    const uint16_t *src = reinterpret_cast<uint16_t *>(ToUintPtr<uint32_t>(chararray->GetData()) + (offset << 1UL));
    bool can_be_compressed = CanBeCompressed(src, length);
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    auto string = AllocStringObject(length, can_be_compressed, ctx, vm);
    if (string == nullptr) {
        return nullptr;
    }

    // retrieve src since gc may move it
    src = reinterpret_cast<uint16_t *>(ToUintPtr<uint32_t>(array_handle->GetData()) + (offset << 1UL));
    ASSERT(string->hashcode_ == 0);
    // After copying we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    if (can_be_compressed) {
        CopyUtf16AsMUtf8(src, string->GetDataMUtf8(), length);
    } else {
        memcpy_s(string->GetDataUtf16(), ComputeDataSizeUtf16(string->GetLength()), src, length << 1UL);
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();
    return string;
}

// static
String *String::CreateNewStringFromBytes(uint32_t offset, uint32_t length, uint32_t high_byte, Array *bytearray,
                                         const LanguageContext &ctx, PandaVM *vm)
{
    ASSERT(length != 0);
    ASSERT(bytearray != nullptr);
    // allocator may trig gc and move array, need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<Array> array_handle(thread, bytearray);

    constexpr size_t BYTE_MASK = 0xFF;

    // NOLINTNEXTLINE(readability-identifier-naming)
    const uint8_t *src = reinterpret_cast<uint8_t *>(ToUintPtr<uint32_t>(bytearray->GetData()) + offset);
    high_byte &= BYTE_MASK;
    bool can_be_compressed = CanBeCompressedMUtf8(src, length) && (high_byte == 0);
    auto string = AllocStringObject(length, can_be_compressed, ctx, vm);
    if (string == nullptr) {
        return nullptr;
    }

    // retrieve src since gc may move it
    src = reinterpret_cast<uint8_t *>(ToUintPtr<uint32_t>(array_handle->GetData()) + offset);
    ASSERT(string->hashcode_ == 0);
    // After copying we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    if (can_be_compressed) {
        Span<const uint8_t> from(src, length);
        Span<uint8_t> to(string->GetDataMUtf8(), length);
        for (uint32_t i = 0; i < length; ++i) {
            to[i] = (from[i] & BYTE_MASK);
        }
    } else {
        Span<const uint8_t> from(src, length);
        Span<uint16_t> to(string->GetDataUtf16(), length);
        for (uint32_t i = 0; i < length; ++i) {
            to[i] = (high_byte << 8U) + (from[i] & BYTE_MASK);
        }
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();

    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();
    return string;
}

template <typename T1, typename T2>
int32_t CompareStringSpan(Span<T1> &lhs_sp, Span<T2> &rhs_sp, int32_t count)
{
    for (int32_t i = 0; i < count; ++i) {
        int32_t char_diff = static_cast<int32_t>(lhs_sp[i]) - static_cast<int32_t>(rhs_sp[i]);
        if (char_diff != 0) {
            return char_diff;
        }
    }
    return 0;
}

template <typename T>
int32_t CompareBytesBlock(T *lstr_pt, T *rstr_pt, int32_t min_count)
{
    constexpr int32_t BYTES_CNT = sizeof(size_t);
    static_assert(BYTES_CNT >= sizeof(T));
    static_assert(BYTES_CNT % sizeof(T) == 0);
    int32_t total_bytes = min_count * sizeof(T);
    auto lhs_block = reinterpret_cast<size_t *>(lstr_pt);
    auto rhs_block = reinterpret_cast<size_t *>(rstr_pt);
    int32_t cur_byte_pos = 0;
    while (cur_byte_pos + BYTES_CNT <= total_bytes) {
        if (*lhs_block == *rhs_block) {
            cur_byte_pos += BYTES_CNT;
            lhs_block++;
            rhs_block++;
        } else {
            break;
        }
    }
    int32_t cur_element_pos = cur_byte_pos / sizeof(T);
    for (int32_t i = cur_element_pos; i < min_count; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        int32_t char_diff = static_cast<int32_t>(lstr_pt[i]) - static_cast<int32_t>(rstr_pt[i]);
        if (char_diff != 0) {
            return char_diff;
        }
    }

    return 0;
}

int32_t String::Compare(String *rstr)
{
    String *lstr = this;
    if (lstr == rstr) {
        return 0;
    }
    ASSERT(lstr->GetLength() <= static_cast<uint32_t>(std::numeric_limits<int32_t>::max()));
    ASSERT(rstr->GetLength() <= static_cast<uint32_t>(std::numeric_limits<int32_t>::max()));
    auto lstr_leng = static_cast<int32_t>(lstr->GetLength());
    auto rstr_leng = static_cast<int32_t>(rstr->GetLength());
    int32_t leng_ret = lstr_leng - rstr_leng;
    int32_t min_count = (leng_ret < 0) ? lstr_leng : rstr_leng;
    bool lstr_is_utf16 = lstr->IsUtf16();
    bool rstr_is_utf16 = rstr->IsUtf16();
    if (!lstr_is_utf16 && !rstr_is_utf16) {
        int32_t char_diff = CompareBytesBlock(lstr->GetDataMUtf8(), rstr->GetDataMUtf8(), min_count);
        if (char_diff != 0) {
            return char_diff;
        }
    } else if (!lstr_is_utf16) {
        Span<uint8_t> lhs_sp(lstr->GetDataMUtf8(), lstr_leng);
        Span<uint16_t> rhs_sp(rstr->GetDataUtf16(), rstr_leng);
        int32_t char_diff = CompareStringSpan(lhs_sp, rhs_sp, min_count);
        if (char_diff != 0) {
            return char_diff;
        }
    } else if (!rstr_is_utf16) {
        Span<uint16_t> lhs_sp(lstr->GetDataUtf16(), lstr_leng);
        Span<uint8_t> rhs_sp(rstr->GetDataMUtf8(), rstr_leng);
        int32_t char_diff = CompareStringSpan(lhs_sp, rhs_sp, min_count);
        if (char_diff != 0) {
            return char_diff;
        }
    } else {
        int32_t char_diff = CompareBytesBlock(lstr->GetDataUtf16(), rstr->GetDataUtf16(), min_count);
        if (char_diff != 0) {
            return char_diff;
        }
    }
    return leng_ret;
}

template <typename T1, typename T2>
static inline ALWAYS_INLINE int32_t SubstringEquals(Span<const T1> &string, Span<const T2> &pattern, int32_t pos)
{
    ASSERT(pos + pattern.size() <= string.size());
    if constexpr (std::is_same_v<T1, T2>) {
        return std::memcmp(string.begin() + pos, pattern.begin(), pattern.size()) == 0;
    }
    return std::equal(pattern.begin(), pattern.end(), string.begin() + pos);
}

/*
 * Tailed Substring method (based on D. Cantone and S. Faro: Searching for a substring with constant extra-space
 * complexity). O(nm) worst-case but reported to have good performance both on random and natural language data
 * Substring s of t is called tailed-substring, if the last character of s does not repeat elsewhere in s
 */
/* static */
template <typename T1, typename T2>
static int32_t IndexOf(Span<const T1> &lhs_sp, Span<const T2> &rhs_sp, int32_t pos, int32_t max)
{
    int32_t max_tailed_len = 1;
    int32_t tailed_end = rhs_sp.size() - 1;
    int32_t max_tailed_end = tailed_end;
    // Phase 1: search in the beginning of string while computing maximal tailed-substring length
    auto search_char = rhs_sp[tailed_end];
    auto *shifted_lhs = lhs_sp.begin() + tailed_end;
    while (pos <= max) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (search_char != shifted_lhs[pos]) {
            pos++;
            continue;
        }
        if (SubstringEquals(lhs_sp, rhs_sp, pos)) {
            return pos;
        }
        auto tailed_start = tailed_end - 1;
        while (tailed_start >= 0 && rhs_sp[tailed_start] != search_char) {
            tailed_start--;
        }
        if (max_tailed_len < tailed_end - tailed_start) {
            max_tailed_len = tailed_end - tailed_start;
            max_tailed_end = tailed_end;
        }
        if (max_tailed_len >= tailed_end) {
            break;
        }
        pos += tailed_end - tailed_start;
        tailed_end--;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        shifted_lhs--;
        search_char = rhs_sp[tailed_end];
    }
    // Phase 2: search in the remainder of string using computed maximal tailed-substring length
    search_char = rhs_sp[max_tailed_end];
    shifted_lhs = lhs_sp.begin() + max_tailed_end;
    while (pos <= max) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (search_char != shifted_lhs[pos]) {
            pos++;
            continue;
        }
        if (SubstringEquals(lhs_sp, rhs_sp, pos)) {
            return pos;
        }
        pos += max_tailed_len;
    }
    return -1;
}

// Search of the last occurence is equivalent to search of the first occurence of
// reversed pattern in reversed string
template <typename T1, typename T2>
static int32_t LastIndexOf(Span<const T1> &lhs_sp, Span<const T2> &rhs_sp, int32_t pos)
{
    int32_t max_tailed_len = 1;
    int32_t tailed_start = 0;
    int32_t max_tailed_start = tailed_start;
    auto pattern_size = static_cast<int32_t>(rhs_sp.size());
    // Phase 1: search in the end of string while computing maximal tailed-substring length
    auto search_char = rhs_sp[tailed_start];
    auto *shifted_lhs = lhs_sp.begin() + tailed_start;
    while (pos >= 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (search_char != shifted_lhs[pos]) {
            pos--;
            continue;
        }
        if (SubstringEquals(lhs_sp, rhs_sp, pos)) {
            return pos;
        }
        auto tailed_end = tailed_start + 1;
        while (tailed_end < pattern_size && rhs_sp[tailed_end] != search_char) {
            tailed_end++;
        }
        if (max_tailed_len < tailed_end - tailed_start) {
            max_tailed_len = tailed_end - tailed_start;
            max_tailed_start = tailed_start;
        }
        if (max_tailed_len >= pattern_size - tailed_start) {
            break;
        }
        pos -= tailed_end - tailed_start;
        tailed_start++;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        shifted_lhs++;
        search_char = rhs_sp[tailed_start];
    }
    // Phase 2: search in the remainder of string using computed maximal tailed-substring length
    search_char = rhs_sp[max_tailed_start];
    shifted_lhs = lhs_sp.begin() + max_tailed_start;
    while (pos >= 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (search_char != shifted_lhs[pos]) {
            pos--;
            continue;
        }
        if (SubstringEquals(lhs_sp, rhs_sp, pos)) {
            return pos;
        }
        pos -= max_tailed_len;
    }
    return -1;
}

static inline ALWAYS_INLINE std::pair<bool, int32_t> GetCompressionAndLength(panda::coretypes::String *string)
{
    ASSERT(string->GetLength() <= static_cast<uint32_t>(std::numeric_limits<int32_t>::max()));
    ASSERT(string != nullptr);
    return {string->IsMUtf8(), static_cast<int32_t>(string->GetLength())};
}

int32_t String::IndexOf(String *rhs, int32_t pos)
{
    String *lhs = this;
    auto [lhs_utf8, lhs_count] = GetCompressionAndLength(lhs);
    auto [rhs_utf8, rhs_count] = GetCompressionAndLength(rhs);

    if (pos < 0) {
        pos = 0;
    }

    if (rhs_count == 0) {
        return std::min(lhs_count, pos);
    }

    int32_t max = lhs_count - rhs_count;
    // for pos > max IndexOf impl will return -1
    if (lhs_utf8 && rhs_utf8) {
        Span<const uint8_t> lhs_sp(lhs->GetDataMUtf8(), lhs_count);
        Span<const uint8_t> rhs_sp(rhs->GetDataMUtf8(), rhs_count);
        return panda::coretypes::IndexOf(lhs_sp, rhs_sp, pos, max);
    } else if (!lhs_utf8 && !rhs_utf8) {  // NOLINT(readability-else-after-return)
        Span<const uint16_t> lhs_sp(lhs->GetDataUtf16(), lhs_count);
        Span<const uint16_t> rhs_sp(rhs->GetDataUtf16(), rhs_count);
        return panda::coretypes::IndexOf(lhs_sp, rhs_sp, pos, max);
    } else if (rhs_utf8) {
        Span<const uint16_t> lhs_sp(lhs->GetDataUtf16(), lhs_count);
        Span<const uint8_t> rhs_sp(rhs->GetDataMUtf8(), rhs_count);
        return panda::coretypes::IndexOf(lhs_sp, rhs_sp, pos, max);
    } else {  // NOLINT(readability-else-after-return)
        Span<const uint8_t> lhs_sp(lhs->GetDataMUtf8(), lhs_count);
        Span<const uint16_t> rhs_sp(rhs->GetDataUtf16(), rhs_count);
        return panda::coretypes::IndexOf(lhs_sp, rhs_sp, pos, max);
    }
}

int32_t String::LastIndexOf(String *rhs, int32_t pos)
{
    String *lhs = this;
    auto [lhs_utf8, lhs_count] = GetCompressionAndLength(lhs);
    auto [rhs_utf8, rhs_count] = GetCompressionAndLength(rhs);

    int32_t max = lhs_count - rhs_count;

    if (pos > max) {
        pos = max;
    }

    if (pos < 0) {
        return -1;
    }

    if (rhs_count == 0) {
        return pos;
    }

    if (lhs_utf8 && rhs_utf8) {
        Span<const uint8_t> lhs_sp(lhs->GetDataMUtf8(), lhs_count);
        Span<const uint8_t> rhs_sp(rhs->GetDataMUtf8(), rhs_count);
        return panda::coretypes::LastIndexOf(lhs_sp, rhs_sp, pos);
    } else if (!lhs_utf8 && !rhs_utf8) {  // NOLINT(readability-else-after-return)
        Span<const uint16_t> lhs_sp(lhs->GetDataUtf16(), lhs_count);
        Span<const uint16_t> rhs_sp(rhs->GetDataUtf16(), rhs_count);
        return panda::coretypes::LastIndexOf(lhs_sp, rhs_sp, pos);
    } else if (rhs_utf8) {
        Span<const uint16_t> lhs_sp(lhs->GetDataUtf16(), lhs_count);
        Span<const uint8_t> rhs_sp(rhs->GetDataMUtf8(), rhs_count);
        return panda::coretypes::LastIndexOf(lhs_sp, rhs_sp, pos);
    } else {  // NOLINT(readability-else-after-return)
        Span<const uint8_t> lhs_sp(lhs->GetDataMUtf8(), lhs_count);
        Span<const uint16_t> rhs_sp(rhs->GetDataUtf16(), rhs_count);
        return panda::coretypes::LastIndexOf(lhs_sp, rhs_sp, pos);
    }
}

/* static */
bool String::CanBeCompressed(const uint16_t *utf16_data, uint32_t utf16_length)
{
    if (!compressed_strings_enabled_) {
        return false;
    }
    bool is_compressed = true;
    Span<const uint16_t> data(utf16_data, utf16_length);
    for (uint32_t i = 0; i < utf16_length; i++) {
        if (!IsASCIICharacter(data[i])) {
            is_compressed = false;
            break;
        }
    }
    return is_compressed;
}

// static
bool String::CanBeCompressedMUtf8(const uint8_t *mutf8_data, uint32_t mutf8_length)
{
    if (!compressed_strings_enabled_) {
        return false;
    }
    bool is_compressed = true;
    Span<const uint8_t> data(mutf8_data, mutf8_length);
    for (uint32_t i = 0; i < mutf8_length; i++) {
        if (!IsASCIICharacter(data[i])) {
            is_compressed = false;
            break;
        }
    }
    return is_compressed;
}

// static
bool String::CanBeCompressedMUtf8(const uint8_t *mutf8_data)
{
    return compressed_strings_enabled_ ? utf::IsMUtf8OnlySingleBytes(mutf8_data) : false;
}

/* static */
bool String::CanBeCompressedUtf16(const uint16_t *utf16_data, uint32_t utf16_length, uint16_t non)
{
    if (!compressed_strings_enabled_) {
        return false;
    }
    bool is_compressed = true;
    Span<const uint16_t> data(utf16_data, utf16_length);
    for (uint32_t i = 0; i < utf16_length; i++) {
        if (!IsASCIICharacter(data[i]) && data[i] != non) {
            is_compressed = false;
            break;
        }
    }
    return is_compressed;
}

/* static */
bool String::CanBeCompressedMUtf8(const uint8_t *mutf8_data, uint32_t mutf8_length, uint16_t non)
{
    if (!compressed_strings_enabled_) {
        return false;
    }
    bool is_compressed = true;
    Span<const uint8_t> data(mutf8_data, mutf8_length);
    for (uint32_t i = 0; i < mutf8_length; i++) {
        if (!IsASCIICharacter(data[i]) && data[i] != non) {
            is_compressed = false;
            break;
        }
    }
    return is_compressed;
}

/* static */
bool String::StringsAreEqual(String *str1, String *str2)
{
    ASSERT(str1 != nullptr);
    ASSERT(str2 != nullptr);

    if ((str1->IsUtf16() != str2->IsUtf16()) || (str1->GetLength() != str2->GetLength())) {
        return false;
    }

    if (str1->IsUtf16()) {
        Span<const uint16_t> data1(str1->GetDataUtf16(), str1->GetLength());
        Span<const uint16_t> data2(str2->GetDataUtf16(), str1->GetLength());
        return String::StringsAreEquals(data1, data2);
    } else {  // NOLINT(readability-else-after-return)
        Span<const uint8_t> data1(str1->GetDataMUtf8(), str1->GetLength());
        Span<const uint8_t> data2(str2->GetDataMUtf8(), str1->GetLength());
        return String::StringsAreEquals(data1, data2);
    }
}

/* static */
bool String::StringsAreEqualMUtf8(String *str1, const uint8_t *mutf8_data, uint32_t utf16_length)
{
    if (str1->GetLength() != utf16_length) {
        return false;
    }
    bool can_be_compressed = CanBeCompressedMUtf8(mutf8_data);
    return StringsAreEqualMUtf8(str1, mutf8_data, utf16_length, can_be_compressed);
}

/* static */
bool String::StringsAreEqualMUtf8(String *str1, const uint8_t *mutf8_data, uint32_t utf16_length,
                                  bool can_be_compressed)
{
    bool result = true;
    if (str1->GetLength() != utf16_length) {
        result = false;
    } else {
        bool str1_can_be_compressed = !str1->IsUtf16();
        bool data2_can_be_compressed = can_be_compressed;
        if (str1_can_be_compressed != data2_can_be_compressed) {
            return false;
        }

        ASSERT(str1_can_be_compressed == data2_can_be_compressed);
        if (str1_can_be_compressed) {
            Span<const uint8_t> data1(str1->GetDataMUtf8(), str1->GetLength());
            Span<const uint8_t> data2(mutf8_data, utf16_length);
            result = String::StringsAreEquals(data1, data2);
        } else {
            result = IsMutf8EqualsUtf16(mutf8_data, str1->GetDataUtf16(), str1->GetLength());
        }
    }
    return result;
}

/* static */
bool String::StringsAreEqualUtf16(String *str1, const uint16_t *utf16_data, uint32_t utf16_data_length)
{
    bool result = true;
    if (str1->GetLength() != utf16_data_length) {
        result = false;
    } else if (!str1->IsUtf16()) {
        result = IsMutf8EqualsUtf16(str1->GetDataMUtf8(), str1->GetLength(), utf16_data, utf16_data_length);
    } else {
        Span<const uint16_t> data1(str1->GetDataUtf16(), str1->GetLength());
        Span<const uint16_t> data2(utf16_data, utf16_data_length);
        result = String::StringsAreEquals(data1, data2);
    }
    return result;
}

/* static */
bool String::IsMutf8EqualsUtf16(const uint8_t *utf8_data, uint32_t utf8_data_length, const uint16_t *utf16_data,
                                uint32_t utf16_data_length)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto tmp_buffer = allocator->AllocArray<uint16_t>(utf16_data_length);
    [[maybe_unused]] auto converted_string_size =
        utf::ConvertRegionMUtf8ToUtf16(utf8_data, tmp_buffer, utf8_data_length, utf16_data_length, 0);
    ASSERT(converted_string_size == utf16_data_length);

    Span<const uint16_t> data1(tmp_buffer, utf16_data_length);
    Span<const uint16_t> data2(utf16_data, utf16_data_length);
    bool result = String::StringsAreEquals(data1, data2);
    allocator->Delete(tmp_buffer);
    return result;
}

/* static */
bool String::IsMutf8EqualsUtf16(const uint8_t *utf8_data, const uint16_t *utf16_data, uint32_t utf16_data_length)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto tmp_buffer = allocator->AllocArray<uint16_t>(utf16_data_length);
    utf::ConvertMUtf8ToUtf16(utf8_data, utf::Mutf8Size(utf8_data), tmp_buffer);

    Span<const uint16_t> data1(tmp_buffer, utf16_data_length);
    Span<const uint16_t> data2(utf16_data, utf16_data_length);
    bool result = String::StringsAreEquals(data1, data2);
    allocator->Delete(tmp_buffer);
    return result;
}

/* static */
template <typename T>
bool String::StringsAreEquals(Span<const T> &str1, Span<const T> &str2)
{
    return 0 == std::memcmp(str1.Data(), str2.Data(), str1.SizeBytes());
}

Array *String::ToCharArray(const LanguageContext &ctx)
{
    // allocator may trig gc and move 'this', need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> str(thread, this);
    auto *klass = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_U16);
    Array *array = Array::Create(klass, GetLength());
    if (array == nullptr) {
        return nullptr;
    }

    if (str->IsUtf16()) {
        Span<uint16_t> sp(str->GetDataUtf16(), str->GetLength());
        for (size_t i = 0; i < sp.size(); i++) {
            array->Set<uint16_t>(i, sp[i]);
        }
    } else {
        Span<uint8_t> sp(str->GetDataMUtf8(), str->GetLength());
        for (size_t i = 0; i < sp.size(); i++) {
            array->Set<uint16_t>(i, sp[i]);
        }
    }

    return array;
}

/* static */
Array *String::GetChars(String *src, uint32_t start, uint32_t utf16_length, const LanguageContext &ctx)
{
    // allocator may trig gc and move 'src', need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> str(thread, src);
    auto *klass = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_U16);
    Array *array = Array::Create(klass, utf16_length);
    if (array == nullptr) {
        return nullptr;
    }

    if (str->IsUtf16()) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        Span<uint16_t> sp(str->GetDataUtf16() + start, utf16_length);
        for (size_t i = 0; i < sp.size(); i++) {
            array->Set<uint16_t>(i, sp[i]);
        }
    } else {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        Span<uint8_t> sp(str->GetDataMUtf8() + start, utf16_length);
        for (size_t i = 0; i < sp.size(); i++) {
            array->Set<uint16_t>(i, sp[i]);
        }
    }

    return array;
}

template <class T>
static int32_t ComputeHashForData(const T *data, size_t size)
{
    uint32_t hash = 0;
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wignored-attributes"
    Span<const T> sp(data, size);
#pragma GCC diagnostic pop
#endif
    for (auto c : sp) {
        constexpr size_t SHIFT = 5;
        hash = (hash << SHIFT) - hash + c;
    }
    return static_cast<int32_t>(hash);
}

static int32_t ComputeHashForMutf8(const uint8_t *mutf8_data)
{
    uint32_t hash = 0;
    while (*mutf8_data != '\0') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        constexpr size_t SHIFT = 5;
        hash = (hash << SHIFT) - hash + *mutf8_data++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    return static_cast<int32_t>(hash);
}

uint32_t String::ComputeHashcode()
{
    uint32_t hash;
    if (compressed_strings_enabled_) {
        if (!IsUtf16()) {
            hash = static_cast<uint32_t>(ComputeHashForData(GetDataMUtf8(), GetLength()));
        } else {
            hash = static_cast<uint32_t>(ComputeHashForData(GetDataUtf16(), GetLength()));
        }
    } else {
        ASSERT(static_cast<size_t>(GetLength()) > (std::numeric_limits<size_t>::max() >> 1U));
        hash = static_cast<uint32_t>(ComputeHashForData(GetDataUtf16(), GetLength()));
    }
    return hash;
}

/* static */
uint32_t String::ComputeHashcodeMutf8(const uint8_t *mutf8_data, uint32_t utf16_length)
{
    bool can_be_compressed = CanBeCompressedMUtf8(mutf8_data);
    return ComputeHashcodeMutf8(mutf8_data, utf16_length, can_be_compressed);
}

/* static */
uint32_t String::ComputeHashcodeMutf8(const uint8_t *mutf8_data, uint32_t utf16_length, bool can_be_compressed)
{
    uint32_t hash;
    if (can_be_compressed) {
        hash = static_cast<uint32_t>(ComputeHashForMutf8(mutf8_data));
    } else {
        // NOTE(alovkov): optimize it without allocation a temporary buffer
        auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
        auto tmp_buffer = allocator->AllocArray<uint16_t>(utf16_length);
        utf::ConvertMUtf8ToUtf16(mutf8_data, utf::Mutf8Size(mutf8_data), tmp_buffer);
        hash = static_cast<uint32_t>(ComputeHashForData(tmp_buffer, utf16_length));
        allocator->Delete(tmp_buffer);
    }
    return hash;
}

/* static */
uint32_t String::ComputeHashcodeUtf16(const uint16_t *utf16_data, uint32_t length)
{
    return ComputeHashForData(utf16_data, length);
}

/* static */
String *String::DoReplace(String *src, uint16_t old_c, uint16_t new_c, const LanguageContext &ctx, PandaVM *vm)
{
    ASSERT(src != nullptr);
    auto length = static_cast<int32_t>(src->GetLength());
    bool can_be_compressed = IsASCIICharacter(new_c);
    if (src->IsUtf16()) {
        can_be_compressed = can_be_compressed && CanBeCompressedUtf16(src->GetDataUtf16(), length, old_c);
    } else {
        can_be_compressed = can_be_compressed && CanBeCompressedMUtf8(src->GetDataMUtf8(), length, old_c);
    }

    // allocator may trig gc and move src, need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> src_handle(thread, src);
    auto string = AllocStringObject(length, can_be_compressed, ctx, vm);
    if (string == nullptr) {
        return nullptr;
    }

    // retrieve src after gc
    src = src_handle.GetPtr();
    ASSERT(string->hashcode_ == 0);

    // After replacing we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    if (src->IsUtf16()) {
        if (can_be_compressed) {
            auto replace = [old_c, new_c](uint16_t c) { return static_cast<uint8_t>((old_c != c) ? c : new_c); };
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            std::transform(src->GetDataUtf16(), src->GetDataUtf16() + length, string->GetDataMUtf8(), replace);
        } else {
            auto replace = [old_c, new_c](uint16_t c) { return (old_c != c) ? c : new_c; };
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            std::transform(src->GetDataUtf16(), src->GetDataUtf16() + length, string->GetDataUtf16(), replace);
        }
    } else {
        if (can_be_compressed) {
            auto replace = [old_c, new_c](uint16_t c) { return static_cast<uint8_t>((old_c != c) ? c : new_c); };
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            std::transform(src->GetDataMUtf8(), src->GetDataMUtf8() + length, string->GetDataMUtf8(), replace);
        } else {
            auto replace = [old_c, new_c](uint16_t c) { return (old_c != c) ? c : new_c; };
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            std::transform(src->GetDataMUtf8(), src->GetDataMUtf8() + length, string->GetDataUtf16(), replace);
        }
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();
    return string;
}

/* static */
String *String::FastSubString(String *src, uint32_t start, uint32_t utf16_length, const LanguageContext &ctx,
                              PandaVM *vm)
{
    ASSERT(src != nullptr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    bool can_be_compressed = !src->IsUtf16() || CanBeCompressed(src->GetDataUtf16() + start, utf16_length);

    // allocator may trig gc and move src, need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> src_handle(thread, src);
    auto string = AllocStringObject(utf16_length, can_be_compressed, ctx, vm);
    if (string == nullptr) {
        return nullptr;
    }

    // retrieve src after gc
    src = src_handle.GetPtr();
    ASSERT(string->hashcode_ == 0);

    // After copying we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    if (src->IsUtf16()) {
        if (can_be_compressed) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            CopyUtf16AsMUtf8(src->GetDataUtf16() + start, string->GetDataMUtf8(), utf16_length);
        } else {
            memcpy_s(string->GetDataUtf16(), ComputeDataSizeUtf16(string->GetLength()),
                     // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                     src->GetDataUtf16() + start, utf16_length << 1UL);
        }
    } else {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        memcpy_s(string->GetDataMUtf8(), string->GetLength(), src->GetDataMUtf8() + start, utf16_length);
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();
    return string;
}

/* static */
String *String::Concat(String *string1, String *string2, const LanguageContext &ctx, PandaVM *vm)
{
    ASSERT(string1 != nullptr);
    ASSERT(string2 != nullptr);
    // allocator may trig gc and move src, need to hold it
    auto thread = ManagedThread::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<String> str1_handle(thread, string1);
    VMHandle<String> str2_handle(thread, string2);

    uint32_t length1 = string1->GetLength();
    uint32_t length2 = string2->GetLength();
    uint32_t new_length = length1 + length2;
    bool compressed = compressed_strings_enabled_ && (!string1->IsUtf16() && !string2->IsUtf16());
    auto new_string = AllocStringObject(new_length, compressed, ctx, vm);
    if (UNLIKELY(new_string == nullptr)) {
        return nullptr;
    }

    ASSERT(new_string->hashcode_ == 0);

    // retrieve strings after gc
    string1 = str1_handle.GetPtr();
    string2 = str2_handle.GetPtr();

    // After copying we should have a full barrier, so this writes should happen-before barrier
    TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
    if (compressed) {
        Span<uint8_t> sp(new_string->GetDataMUtf8(), new_length);
        memcpy_s(sp.Data(), sp.SizeBytes(), string1->GetDataMUtf8(), length1);
        sp = sp.SubSpan(length1);
        memcpy_s(sp.Data(), sp.SizeBytes(), string2->GetDataMUtf8(), length2);
    } else {
        Span<uint16_t> sp(new_string->GetDataUtf16(), new_length);
        if (!string1->IsUtf16()) {
            for (uint32_t i = 0; i < length1; ++i) {
                sp[i] = string1->At<false>(i);
            }
        } else {
            memcpy_s(sp.Data(), sp.SizeBytes(), string1->GetDataUtf16(), length1 << 1U);
        }
        sp = sp.SubSpan(length1);
        if (!string2->IsUtf16()) {
            for (uint32_t i = 0; i < length2; ++i) {
                sp[i] = string2->At<false>(i);
            }
        } else {
            memcpy_s(sp.Data(), sp.SizeBytes(), string2->GetDataUtf16(), length2 << 1U);
        }
    }
    TSAN_ANNOTATE_IGNORE_WRITES_END();
    // String is supposed to be a constant object, so all its data should be visible by all threads
    arch::FullMemoryBarrier();

    return new_string;
}

/* static */
String *String::AllocStringObject(size_t length, bool compressed, const LanguageContext &ctx, PandaVM *vm, bool movable)
{
    ASSERT(vm != nullptr);
    auto *thread = ManagedThread::GetCurrent();
    auto *string_class = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::STRING);
    size_t size = compressed ? String::ComputeSizeMUtf8(length) : String::ComputeSizeUtf16(length);
    auto string =
        movable
            ? reinterpret_cast<String *>(vm->GetHeapManager()->AllocateObject(
                  string_class, size, DEFAULT_ALIGNMENT, thread, mem::ObjectAllocatorBase::ObjMemInitPolicy::NO_INIT))
            : reinterpret_cast<String *>(vm->GetHeapManager()->AllocateNonMovableObject(
                  string_class, size, DEFAULT_ALIGNMENT, thread, mem::ObjectAllocatorBase::ObjMemInitPolicy::NO_INIT));
    if (string != nullptr) {
        // After setting length we should have a full barrier, so this write should happens-before barrier
        TSAN_ANNOTATE_IGNORE_WRITES_BEGIN();
        string->SetLength(length, compressed);
        string->SetHashcode(0);
        TSAN_ANNOTATE_IGNORE_WRITES_END();
        // Witout full memory barrier it is possible that architectures with weak memory order can try fetching string
        // legth before it's set
        arch::FullMemoryBarrier();
    }
    return string;
}

}  // namespace panda::coretypes
