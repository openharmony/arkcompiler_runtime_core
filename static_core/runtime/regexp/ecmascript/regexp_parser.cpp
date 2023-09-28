/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "runtime/regexp/ecmascript/regexp_parser.h"
#include "runtime/regexp/ecmascript/regexp_opcode.h"
#include "runtime/include/coretypes/string-inl.h"

#include "libpandabase/utils/utils.h"
#include "libpandabase/utils/utf.h"

#include "securec.h"
#include "unicode/uchar.h"
#include "unicode/uniset.h"

#define _NO_DEBUG_
namespace {
constexpr int INVALID_UNICODE_FROM_UTF8 = -1;

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
constexpr int UICODE_FROM_UTF8[] = {
    0x80, 0xc0, 0xdf, 0xe0, 0xef, 0xf0, 0xf7, 0xf8, 0xfb, 0xfc, 0xfd,
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
constexpr int UTF8_MIN_CODE[] = {
    0x80, 0x800, 0x10000, 0x00200000, 0x04000000,
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
constexpr char UTF8_FIRST_CODE[] = {
    0x1f, 0xf, 0x7, 0x3, 0x1,
};

int FromUtf8(int c, int l, const uint8_t *p, const uint8_t **pp)
{
    uint32_t b;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    c &= UTF8_FIRST_CODE[l - 1];
    for (int i = 0; i < l; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        b = *p++;
        if (b < panda::utf::UTF8_2B_SECOND || b >= panda::utf::UTF8_2B_FIRST) {
            return INVALID_UNICODE_FROM_UTF8;
        }
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        c = (c << 6) | (b & panda::utf::UTF8_2B_THIRD);  // 6: Maximum Unicode range
    }
    if (c < UTF8_MIN_CODE[l - 1]) {
        return INVALID_UNICODE_FROM_UTF8;
    }
    *pp = p;
    return c;
}

int UnicodeFromUtf8(const uint8_t *p, int max_len, const uint8_t **pp)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    int c = *p++;
    if (c < UICODE_FROM_UTF8[0]) {
        *pp = p;
        return c;
    }
    int l = 0;
    if (c >= UICODE_FROM_UTF8[1U] && c <= UICODE_FROM_UTF8[2U]) {         // 1 - 2: 0000 0080 - 0000 07FF
        l = 1;                                                            // 1: 0000 0080 - 0000 07FF Unicode
    } else if (c >= UICODE_FROM_UTF8[3U] && c <= UICODE_FROM_UTF8[4U]) {  // 3 - 4: 0000 0800 - 0000 FFFF
        l = 2;                                                            // 2: 0000 0800 - 0000 FFFF Unicode
    } else if (c >= UICODE_FROM_UTF8[5U] && c <= UICODE_FROM_UTF8[6U]) {  // 5 - 6: 0001 0000 - 0010 FFFF
        l = 3;                                                            // 3: 0001 0000 - 0010 FFFF Unicode
    } else if (c >= UICODE_FROM_UTF8[7U] && c <= UICODE_FROM_UTF8[8U]) {  // 7 - 8: 0020 0000 - 03FF FFFF
        l = 4;                                                            // 4: 0020 0000 - 03FF FFFF Unicode
        // NOLINTNEXTLINE(readability-magic-numbers)
    } else if (c == UICODE_FROM_UTF8[9U] || c == UICODE_FROM_UTF8[10U]) {  // 9 - 10: 0400 0000 - 7FFF FFFF
        l = 5;                                                             // 5: 0400 0000 - 7FFF FFFF Unicode
    } else {
        return INVALID_UNICODE_FROM_UTF8;
    }
    /* check that we have enough characters */
    if (l > (max_len - 1)) {
        return INVALID_UNICODE_FROM_UTF8;
    }
    return FromUtf8(c, l, p, pp);
}
}  // namespace

namespace panda {
static constexpr uint32_t CACHE_SIZE = 128;
static constexpr uint32_t CHAR_MAXS = 128;
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static constexpr uint32_t ID_START_TABLE_ASCII[4] = {
    /* $ A-Z _ a-z */
    0x00000000, 0x00000010, 0x87FFFFFE, 0x07FFFFFE};
static RangeSet G_RANGE_D(0x30, 0x39);  // NOLINT(fuchsia-statically-constructed-objects, readability-magic-numbers)
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static RangeSet G_RANGE_S({
    std::pair<uint32_t, uint32_t>(0x0009, 0x000D),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0020, 0x0020),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x00A0, 0x00A0),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x1680, 0x1680),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x2000, 0x200A),  // NOLINT(readability-magic-numbers)
    /* 2028;LINE SEPARATOR;Zl;0;WS;;;;;N;;;;; */
    /* 2029;PARAGRAPH SEPARATOR;Zp;0;B;;;;;N;;;;; */
    std::pair<uint32_t, uint32_t>(0x2028, 0x2029),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x202F, 0x202F),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x205F, 0x205F),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x3000, 0x3000),  // NOLINT(readability-magic-numbers)
    /* FEFF;ZERO WIDTH NO-BREAK SPACE;Cf;0;BN;;;;;N;BYTE ORDER MARK;;;; */
    std::pair<uint32_t, uint32_t>(0xFEFF, 0xFEFF),  // NOLINT(readability-magic-numbers)
});

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static RangeSet G_RANGE_W({
    std::pair<uint32_t, uint32_t>(0x0030, 0x0039),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0041, 0x005A),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x005F, 0x005F),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0061, 0x007A),  // NOLINT(readability-magic-numbers)
});

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static RangeSet G_REGEXP_IDENTIFY_START({
    std::pair<uint32_t, uint32_t>(0x0024, 0x0024),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0041, 0x005A),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0061, 0x007A),  // NOLINT(readability-magic-numbers)
});

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static RangeSet G_REGEXP_IDENTIFY_CONTINUE({
    std::pair<uint32_t, uint32_t>(0x0024, 0x0024),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0030, 0x0039),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0041, 0x005A),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0061, 0x007A),  // NOLINT(readability-magic-numbers)
});

void RegExpParser::Parse()
{
    // dynbuffer head init [size,capture_count,statck_count,flags]
    buffer_.EmitU32(0);
    buffer_.EmitU32(0);
    buffer_.EmitU32(0);
    buffer_.EmitU32(0);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse Pattern------\n");
    // Pattern[U, N]::
    //      Disjunction[?U, ?N]
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    Advance();
    SaveStartOpCode save_start_op;
    int capture_index = capture_count_++;
    save_start_op.EmitOpCode(&buffer_, capture_index);
    ParseDisjunction(false);
    if (c0_ != KEY_EOF) {
        ParseError("extraneous characters at the end");
        return;
    }
    SaveEndOpCode save_end_op;
    save_end_op.EmitOpCode(&buffer_, capture_index);
    MatchEndOpCode match_end_op;
    match_end_op.EmitOpCode(&buffer_, 0);
    // dynbuffer head assignments
    buffer_.PutU32(0, buffer_.size_);
    buffer_.PutU32(NUM_CAPTURE__OFFSET, capture_count_);
    buffer_.PutU32(NUM_STACK_OFFSET, stack_count_);
    buffer_.PutU32(FLAGS_OFFSET, flags_);
#ifndef _NO_DEBUG_
    RegExpOpCode::DumpRegExpOpCode(std::cout, buffer_);
#endif
}

void RegExpParser::ParseDisjunction(bool is_backward)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse Disjunction------\n");
    size_t start = buffer_.size_;
    ParseAlternative(is_backward);
    if (is_error_) {
        return;
    }
    do {
        if (c0_ == '|') {
            SplitNextOpCode split_op;
            uint32_t len = buffer_.size_ - start;
            GotoOpCode goto_op;
            split_op.InsertOpCode(&buffer_, start, len + goto_op.GetSize());
            uint32_t pos = goto_op.EmitOpCode(&buffer_, 0) - goto_op.GetSize();
            Advance();
            ParseAlternative(is_backward);
            goto_op.UpdateOpPara(&buffer_, pos, buffer_.size_ - pos - goto_op.GetSize());
        }
    } while (c0_ != KEY_EOF && c0_ != ')');
}

uint32_t RegExpParser::ParseOctalLiteral()
{
    // For compatibility with some other browsers (not all), we parse
    // up to three octal digits with a value below 256.
    // ES#prod-annexB-LegacyOctalEscapeSequence
    uint32_t value = c0_ - '0';
    Advance();
    if (c0_ >= '0' && c0_ <= '7') {
        value = value * OCTAL_VALUE + c0_ - '0';
        Advance();
        if (value < OCTAL_VALUE_RANGE && c0_ >= '0' && c0_ <= '7') {
            value = value * OCTAL_VALUE + c0_ - '0';
            Advance();
        }
    }
    return value;
}

bool RegExpParser::ParseUnlimitedLengthHexNumber(uint32_t max_value, uint32_t *value)
{
    uint32_t x = 0;
    int d = static_cast<int>(HexValue(c0_));
    if (d < 0) {
        return false;
    }
    while (d >= 0) {
        if (UNLIKELY(x > (std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(d)) / HEX_VALUE)) {
            LOG(FATAL, COMMON) << "value overflow";
            return false;
        }
        x = x * HEX_VALUE + static_cast<uint32_t>(d);
        if (x > max_value) {
            return false;
        }
        Advance();
        d = static_cast<int>(HexValue(c0_));
    }
    *value = x;
    return true;
}

// This parses RegExpUnicodeEscapeSequence as described in ECMA262.
bool RegExpParser::ParseUnicodeEscape(uint32_t *value)
{
    // Accept both \uxxxx and \u{xxxxxx} (if allowed).
    // In the latter case, the number of hex digits between { } is arbitrary.
    // \ and u have already been read.
    if (c0_ == '{' && IsUtf16()) {
        uint8_t *start = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        Advance();
        if (ParseUnlimitedLengthHexNumber(0x10FFFF, value)) {  // NOLINT(readability-magic-numbers)
            if (c0_ == '}') {
                Advance();
                return true;
            }
        }
        pc_ = start;
        Advance();
        return false;
    }
    // \u but no {, or \u{...} escapes not allowed.
    bool result = ParseHexEscape(UNICODE_HEX_VALUE, value);
    if (result && IsUtf16() && U16_IS_LEAD(*value) && c0_ == '\\') {
        // Attempt to read trail surrogate.
        uint8_t *start = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (*pc_ == 'u') {
            Advance(UNICODE_HEX_ADVANCE);
            uint32_t trail;
            if (ParseHexEscape(UNICODE_HEX_VALUE, &trail) && U16_IS_TRAIL(trail)) {
                *value = U16_GET_SUPPLEMENTARY((*value), (trail));  // NOLINT(hicpp-signed-bitwise)
                return true;
            }
        }
        pc_ = start;
        Advance();
    }
    return result;
}

bool RegExpParser::ParseHexEscape(int length, uint32_t *value)
{
    uint8_t *start = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint32_t val = 0;
    for (int i = 0; i < length; ++i) {
        uint32_t c = c0_;
        int d = static_cast<int>(HexValue(c));
        if (d < 0) {
            pc_ = start;
            Advance();
            return false;
        }
        val = val * HEX_VALUE + static_cast<uint32_t>(d);
        Advance();
    }
    *value = val;
    return true;
}

// NOLINTNEXTLINE(readability-function-size)
void RegExpParser::ParseAlternative(bool is_backward)
{
    size_t start = buffer_.size_;
    while (c0_ != '|' && c0_ != KEY_EOF && c0_ != ')') {
        if (is_error_) {
            return;
        }
        size_t atom_bc_start = buffer_.GetSize();
        int capture_index = 0;
        bool is_atom = false;
        switch (c0_) {
            case '^': {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("Assertion %c line start \n", c0_);
                LineStartOpCode line_start_op;
                line_start_op.EmitOpCode(&buffer_, 0);
                Advance();
                break;
            }
            case '$': {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("Assertion %c line end \n", c0_);
                LineEndOpCode line_end_op;
                line_end_op.EmitOpCode(&buffer_, 0);
                Advance();
                break;
            }
            case '\\': {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("Escape %c \n", c0_);
                Advance();
                switch (c0_) {
                    case 'b': {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                        PrintF("Assertion %c \n", c0_);
                        WordBoundaryOpCode word_boundary_op;
                        word_boundary_op.EmitOpCode(&buffer_, 0);
                        Advance();
                        break;
                    }
                    case 'B': {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                        PrintF("Assertion %c \n", c0_);
                        NotWordBoundaryOpCode not_word_boundary_op;
                        not_word_boundary_op.EmitOpCode(&buffer_, 0);
                        Advance();
                        break;
                    }
                    default: {
                        is_atom = true;
                        int atom_value = ParseAtomEscape(is_backward);
                        if (atom_value != -1) {
                            if (IsIgnoreCase()) {
                                if (!IsUtf16()) {
                                    atom_value = Canonicalize(atom_value, false);
                                } else {
                                    icu::UnicodeSet set(atom_value, atom_value);
                                    set.closeOver(USET_CASE_INSENSITIVE);
                                    set.removeAllStrings();
                                    int32_t size = set.size();
                                    RangeOpCode range_op;
                                    RangeSet range_result;
                                    for (int32_t idx = 0; idx < size; idx++) {
                                        int32_t uc = set.charAt(idx);
                                        RangeSet cur_range(uc);
                                        range_result.Insert(cur_range);
                                    }
                                    range_op.InsertOpCode(&buffer_, range_result);
                                    break;
                                }
                            }
                            if (atom_value <= UINT16_MAX) {
                                CharOpCode char_op;
                                char_op.EmitOpCode(&buffer_, atom_value);
                            } else {
                                Char32OpCode char_op;
                                char_op.EmitOpCode(&buffer_, atom_value);
                            }
                        }
                        break;
                    }
                }
                break;
            }
            case '(': {
                Advance();
                is_atom = ParseAssertionCapture(&capture_index, is_backward);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                Advance();
                break;
            }
            case '.': {
                PrevOpCode prev_op;
                if (is_backward) {
                    prev_op.EmitOpCode(&buffer_, 0);
                }
                if (IsDotAll()) {
                    AllOpCode all_op;
                    all_op.EmitOpCode(&buffer_, 0);
                } else {
                    DotsOpCode dots_op;
                    dots_op.EmitOpCode(&buffer_, 0);
                }
                if (is_backward) {
                    prev_op.EmitOpCode(&buffer_, 0);
                }
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("Atom %c match any \n", c0_);
                is_atom = true;
                Advance();
                break;
            }
            case '[': {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("Atom %c match range \n", c0_);
                is_atom = true;
                PrevOpCode prev_op;
                Advance();
                if (is_backward) {
                    prev_op.EmitOpCode(&buffer_, 0);
                }
                bool is_invert = false;
                if (c0_ == '^') {
                    is_invert = true;
                    Advance();
                }
                RangeSet range_result;
                if (!ParseClassRanges(&range_result)) {
                    break;
                }
                if (is_invert) {
                    range_result.Invert(IsUtf16());
                }
                uint32_t high_value = range_result.HighestValue();
                if (high_value <= UINT16_MAX) {
                    RangeOpCode range_op;
                    range_op.InsertOpCode(&buffer_, range_result);
                } else {
                    Range32OpCode range_op;
                    range_op.InsertOpCode(&buffer_, range_result);
                }

                if (is_backward) {
                    prev_op.EmitOpCode(&buffer_, 0);
                }
                break;
            }
            case '*':
            case '+':
            case '?':
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                ParseError("nothing to repeat");
                return;
            case '{': {
                uint8_t *begin = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                int dummy;
                if (ParserIntervalQuantifier(&dummy, &dummy)) {
                    ParseError("nothing to repeat");
                    return;
                }
                pc_ = begin;
                Advance();
            }
                [[fallthrough]];
            case '}':
            case ']':
                if (IsUtf16()) {
                    ParseError("syntax error");
                    return;
                }
                [[fallthrough]];
            default: {
                // PatternCharacter
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("PatternCharacter %c\n", c0_);
                is_atom = true;
                {
                    PrevOpCode prev_op;
                    if (is_backward) {
                        prev_op.EmitOpCode(&buffer_, 0);
                    }
                    uint32_t matched_char = c0_;
                    if (c0_ > (INT8_MAX + 1)) {
                        Prev();
                        int i = 0;
                        UChar32 c;
                        int32_t length = end_ - pc_ + 1;
                        // NOLINTNEXTLINE(hicpp-signed-bitwise)
                        U8_NEXT(pc_, i, length, c);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                        matched_char = static_cast<uint32_t>(c);
                        pc_ += i;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    }
                    if (IsIgnoreCase()) {
                        matched_char = static_cast<uint32_t>(Canonicalize(static_cast<int>(matched_char), IsUtf16()));
                    }
                    if (matched_char > UINT16_MAX) {
                        Char32OpCode char_op;
                        char_op.EmitOpCode(&buffer_, matched_char);
                    } else {
                        CharOpCode char_op;
                        char_op.EmitOpCode(&buffer_, matched_char);
                    }
                    if (is_backward) {
                        prev_op.EmitOpCode(&buffer_, 0);
                    }
                }
                Advance();
                break;
            }
        }
        if (is_atom && !is_error_) {
            ParseQuantifier(atom_bc_start, capture_index, capture_count_ - 1);
        }
        if (is_backward) {
            size_t end = buffer_.GetSize();
            size_t term_size = end - atom_bc_start;
            size_t move_size = end - start;
            buffer_.Expand(end + term_size);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memmove_s(buffer_.buf_ + start + term_size, move_size, buffer_.buf_ + start, move_size) != EOK) {
                LOG(FATAL, COMMON) << "memmove_s failed";
                UNREACHABLE();
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(buffer_.buf_ + start, term_size, buffer_.buf_ + end, term_size) != EOK) {
                LOG(FATAL, COMMON) << "memcpy_s failed";
                UNREACHABLE();
            }
        }
    }
}

int RegExpParser::FindGroupName(const PandaString &name)
{
    size_t len = 0;
    size_t name_len = name.size();
    const char *p = reinterpret_cast<char *>(group_names_.buf_);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const char *buf_end = reinterpret_cast<char *>(group_names_.buf_) + group_names_.size_;
    int capture_index = 1;
    while (p < buf_end) {
        len = strlen(p);
        if (len == name_len && memcmp(name.c_str(), p, name_len) == 0) {
            return capture_index;
        }
        p += len + 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        capture_index++;
    }
    return -1;
}

bool RegExpParser::ParseAssertionCapture(int *capture_index, bool is_backward)
{
    bool is_atom = false;
    do {
        if (c0_ == '?') {
            Advance();
            switch (c0_) {
                // (?=Disjunction[?U, ?N])
                case '=': {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                    PrintF("Assertion(?= Disjunction)\n");
                    Advance();
                    uint32_t start = buffer_.size_;
                    ParseDisjunction(is_backward);
                    MatchOpCode match_op;
                    match_op.EmitOpCode(&buffer_, 0);
                    MatchAheadOpCode match_ahead_op;
                    uint32_t len = buffer_.size_ - start;
                    match_ahead_op.InsertOpCode(&buffer_, start, len);
                    break;
                }
                // (?!Disjunction[?U, ?N])
                case '!': {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                    PrintF("Assertion(?! Disjunction)\n");
                    uint32_t start = buffer_.size_;
                    Advance();
                    ParseDisjunction(is_backward);
                    MatchOpCode match_op;
                    match_op.EmitOpCode(&buffer_, 0);
                    NegativeMatchAheadOpCode match_ahead_op;
                    uint32_t len = buffer_.size_ - start;
                    match_ahead_op.InsertOpCode(&buffer_, start, len);
                    break;
                }
                case '<': {
                    Advance();
                    // (?<=Disjunction[?U, ?N])
                    if (c0_ == '=') {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                        PrintF("Assertion(?<= Disjunction)\n");
                        Advance();
                        uint32_t start = buffer_.size_;
                        ParseDisjunction(true);
                        MatchOpCode match_op;
                        match_op.EmitOpCode(&buffer_, 0);
                        MatchAheadOpCode match_ahead_op;
                        uint32_t len = buffer_.size_ - start;
                        match_ahead_op.InsertOpCode(&buffer_, start, len);
                        // (?<!Disjunction[?U, ?N])
                    } else if (c0_ == '!') {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                        PrintF("Assertion(?<! Disjunction)\n");
                        Advance();
                        uint32_t start = buffer_.size_;
                        ParseDisjunction(true);
                        MatchOpCode match_op;
                        match_op.EmitOpCode(&buffer_, 0);
                        NegativeMatchAheadOpCode match_ahead_op;
                        uint32_t len = buffer_.size_ - start;
                        match_ahead_op.InsertOpCode(&buffer_, start, len);
                    } else {
                        Prev();
                        PandaString name;
                        auto **pp = const_cast<const uint8_t **>(&pc_);
                        if (!ParseGroupSpecifier(pp, name)) {
                            ParseError("GroupName Syntax error.");
                            return false;
                        }
                        if (FindGroupName(name) > 0) {
                            ParseError("Duplicate GroupName error.");
                            return false;
                        }
                        group_names_.EmitStr(name.c_str());
                        new_group_names_.push_back(name);
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                        PrintF("group name %s", name.c_str());
                        Advance();
                        goto parseCapture;  // NOLINT(cppcoreguidelines-avoid-goto)
                    }
                    break;
                }
                // (?:Disjunction[?U, ?N])
                case ':':
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                    PrintF("Atom(?<: Disjunction)\n");
                    is_atom = true;
                    Advance();
                    ParseDisjunction(is_backward);
                    break;
                default:
                    Advance();
                    ParseError("? Syntax error.");
                    return false;
            }
        } else {
            group_names_.EmitChar(0);
        parseCapture:
            is_atom = true;
            *capture_index = capture_count_++;
            SaveEndOpCode save_end_op;
            SaveStartOpCode save_start_op;
            if (is_backward) {
                save_end_op.EmitOpCode(&buffer_, *capture_index);
            } else {
                save_start_op.EmitOpCode(&buffer_, *capture_index);
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("capture start %d \n", *capture_index);
            ParseDisjunction(is_backward);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("capture end %d \n", *capture_index);
            if (is_backward) {
                save_start_op.EmitOpCode(&buffer_, *capture_index);
            } else {
                save_end_op.EmitOpCode(&buffer_, *capture_index);
            }
        }
    } while (c0_ != ')' && c0_ != KEY_EOF);
    if (c0_ != ')') {
        ParseError("capture syntax error");
        return false;
    }
    return is_atom;
}

int RegExpParser::ParseDecimalDigits()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse DecimalDigits------\n");
    uint32_t result = 0;
    bool overflow = false;
    while (true) {
        if (c0_ < '0' || c0_ > '9') {
            break;
        }
        if (!overflow) {
            if (UNLIKELY(result > (INT32_MAX - c0_ + '0') / DECIMAL_DIGITS_ADVANCE)) {
                overflow = true;
            } else {
                result = result * DECIMAL_DIGITS_ADVANCE + c0_ - '0';
            }
        }
        Advance();
    }
    if (overflow) {
        return INT32_MAX;
    }
    return result;
}

bool RegExpParser::ParserIntervalQuantifier(int *pmin, int *pmax)
{
    // Quantifier::
    //     QuantifierPrefix
    //     QuantifierPrefix?
    // QuantifierPrefix::
    // *
    // +
    // ?
    // {DecimalDigits}
    // {DecimalDigits,}
    // {DecimalDigits,DecimalDigits}
    Advance();
    *pmin = ParseDecimalDigits();
    *pmax = *pmin;
    switch (c0_) {
        case ',': {
            Advance();
            if (c0_ == '}') {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("QuantifierPrefix{DecimalDigits,}\n");
                *pmax = INT32_MAX;
                Advance();
            } else {
                *pmax = ParseDecimalDigits();
                if (c0_ == '}') {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                    PrintF("QuantifierPrefix{DecimalDigits,DecimalDigits}\n");
                    Advance();
                } else {
                    return false;
                }
            }
            break;
        }
        case '}':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("QuantifierPrefix{DecimalDigits}\n");
            Advance();
            break;
        default:
            Advance();
            return false;
    }
    return true;
}

void RegExpParser::ParseQuantifier(size_t atom_bc_start, int capture_start, int capture_end)
{
    int min = -1;
    int max = -1;
    bool is_greedy = true;
    switch (c0_) {
        case '*':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("QuantifierPrefix %c\n", c0_);
            min = 0;
            max = INT32_MAX;
            Advance();
            break;
        case '+':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("QuantifierPrefix %c\n", c0_);
            min = 1;
            max = INT32_MAX;
            Advance();
            break;
        case '?':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("QuantifierPrefix %c\n", c0_);
            Advance();
            min = 0;
            max = 1;
            break;
        case '{': {
            uint8_t *start = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (!ParserIntervalQuantifier(&min, &max)) {
                pc_ = start;
                Advance();  // back to '{'
                return;
            }
            if (min > max) {
                ParseError("Invalid repetition count");
                return;
            }
            break;
        }
        default:
            break;
    }
    if (c0_ == '?') {
        is_greedy = false;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        PrintF("Quantifier::QuantifierPrefix?\n");
        Advance();
    } else if (c0_ == '?' || c0_ == '+' || c0_ == '*' || c0_ == '{') {
        ParseError("nothing to repeat");
        return;
    }
    if (min != -1 && max != -1) {
        stack_count_++;
        PushOpCode push_op;
        push_op.InsertOpCode(&buffer_, atom_bc_start);
        atom_bc_start += push_op.GetSize();

        if (capture_start != 0) {
            SaveResetOpCode save_reset_op;
            save_reset_op.InsertOpCode(&buffer_, atom_bc_start, capture_start, capture_end);
        }

        // zero advance check
        if (max == INT32_MAX) {
            stack_count_++;
            PushCharOpCode push_char_op;
            push_char_op.InsertOpCode(&buffer_, atom_bc_start);
            CheckCharOpCode check_char_op;
            // NOLINTNEXTLINE(readability-magic-numbers)
            check_char_op.EmitOpCode(&buffer_, RegExpOpCode::GetRegExpOpCode(RegExpOpCode::OP_LOOP)->GetSize());
        }

        if (is_greedy) {
            LoopGreedyOpCode loop_op;
            loop_op.EmitOpCode(&buffer_, atom_bc_start - buffer_.GetSize() - loop_op.GetSize(), min, max);
        } else {
            LoopOpCode loop_op;
            loop_op.EmitOpCode(&buffer_, atom_bc_start - buffer_.GetSize() - loop_op.GetSize(), min, max);
        }

        if (min == 0) {
            if (is_greedy) {
                SplitNextOpCode split_next_op;
                split_next_op.InsertOpCode(&buffer_, atom_bc_start, buffer_.GetSize() - atom_bc_start);
            } else {
                SplitFirstOpCode split_first_op;
                split_first_op.InsertOpCode(&buffer_, atom_bc_start, buffer_.GetSize() - atom_bc_start);
            }
        }

        PopOpCode pop_op;
        pop_op.EmitOpCode(&buffer_);
    }
}

bool RegExpParser::ParseGroupSpecifier(const uint8_t **pp, PandaString &name)
{
    const uint8_t *p = *pp;
    uint32_t c;
    std::array<char, CACHE_SIZE> buffer {};
    char *q = buffer.data();
    while (true) {
        if (p <= end_) {
            c = *p;
        } else {
            c = KEY_EOF;
        }
        if (c == '\\') {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            p++;
            if (*p != 'u') {
                return false;
            }
            if (!ParseUnicodeEscape(&c)) {
                return false;
            }
        } else if (c == '>') {
            break;
        } else if (c > CACHE_SIZE && c != KEY_EOF) {
            c = static_cast<uint32_t>(UnicodeFromUtf8(p, UTF8_CHAR_LEN_MAX, &p));
        } else if (c != KEY_EOF) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            p++;
        } else {
            return false;
        }
        if (q == buffer.data()) {
            if (IsIdentFirst(c) != 0) {
                return false;
            }
        } else {
            if (!u_isIDPart(c)) {
                return false;
            }
        }
        if (q != nullptr) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *q++ = c;
        }
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    p++;
    *pp = p;
    name = buffer.data();
    return true;
}

int RegExpParser::ParseCaptureCount(const char *group_name)
{
    const uint8_t *p = nullptr;
    int capture_index = 1;
    PandaString name;
    has_named_captures_ = 0;
    for (p = base_; p < end_; p++) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        switch (*p) {
            case '(': {
                if (p[1] == '?') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    if (p[CAPTURE_CONUT_ADVANCE - 1] == '<' && p[CAPTURE_CONUT_ADVANCE] != '!' &&
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                        p[CAPTURE_CONUT_ADVANCE] != '=') {
                        has_named_captures_ = 1;
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                        p += CAPTURE_CONUT_ADVANCE;
                        if (group_name != nullptr) {
                            if (ParseGroupSpecifier(&p, name)) {
                                if (strcmp(name.c_str(), group_name) == 0) {
                                    return capture_index;
                                }
                            }
                        }
                        capture_index++;
                    }
                } else {
                    capture_index++;
                }
                break;
            }
            case '\\':
                p++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                break;
            case '[': {
                while (p < end_ && *p != ']') {
                    if (*p == '\\') {
                        p++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    }
                    p++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                }
                break;
            }
            default:
                break;
        }
    }
    return capture_index;
}

// NOLINTNEXTLINE(readability-function-size)
int RegExpParser::ParseAtomEscape(bool is_backward)
{
    // AtomEscape[U, N]::
    //     DecimalEscape
    //     CharacterClassEscape[?U]
    //     CharacterEscape[?U]
    //     [+N]kGroupName[?U]
    int result = -1;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse AtomEscape------\n");
    PrevOpCode prev_op;
    switch (c0_) {
        case KEY_EOF:
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            ParseError("unexpected end");
            break;
        // DecimalEscape
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("NonZeroDigit %c\n", c0_);
            int capture = ParseDecimalDigits();
            if (capture > capture_count_ - 1 && capture > ParseCaptureCount(nullptr) - 1) {
                ParseError("invalid backreference count");
                break;
            }
            if (is_backward) {
                BackwardBackReferenceOpCode back_reference_op;
                back_reference_op.EmitOpCode(&buffer_, capture);
            } else {
                BackReferenceOpCode back_reference_op;
                back_reference_op.EmitOpCode(&buffer_, capture);
            }
            break;
        }
        // CharacterClassEscape
        case 'd': {
            // [0-9]
            RangeOpCode range_op;
            if (is_backward) {
                prev_op.EmitOpCode(&buffer_, 0);
            }
            range_op.InsertOpCode(&buffer_, G_RANGE_D);
            goto parseLookBehind;  // NOLINT(cppcoreguidelines-avoid-goto)
            break;
        }
        case 'D': {
            // [^0-9]
            RangeSet atom_range(G_RANGE_D);
            atom_range.Invert(IsUtf16());
            Range32OpCode range_op;
            if (is_backward) {
                prev_op.EmitOpCode(&buffer_, 0);
            }
            range_op.InsertOpCode(&buffer_, atom_range);
            goto parseLookBehind;  // NOLINT(cppcoreguidelines-avoid-goto)
            break;
        }
        case 's': {
            // [\f\n\r\t\v]
            RangeOpCode range_op;
            if (is_backward) {
                prev_op.EmitOpCode(&buffer_, 0);
            }
            range_op.InsertOpCode(&buffer_, G_RANGE_S);
            goto parseLookBehind;  // NOLINT(cppcoreguidelines-avoid-goto)
            break;
        }
        case 'S': {
            RangeSet atom_range(G_RANGE_S);
            Range32OpCode range_op;
            atom_range.Invert(IsUtf16());
            if (is_backward) {
                prev_op.EmitOpCode(&buffer_, 0);
            }
            range_op.InsertOpCode(&buffer_, atom_range);
            goto parseLookBehind;  // NOLINT(cppcoreguidelines-avoid-goto)
            break;
        }
        case 'w': {
            // [A-Za-z0-9]
            RangeOpCode range_op;
            if (is_backward) {
                prev_op.EmitOpCode(&buffer_, 0);
            }
            range_op.InsertOpCode(&buffer_, G_RANGE_W);
            goto parseLookBehind;  // NOLINT(cppcoreguidelines-avoid-goto)
            break;
        }
        case 'W': {
            // [^A-Za-z0-9]
            RangeSet atom_range(G_RANGE_W);
            atom_range.Invert(IsUtf16());
            Range32OpCode range_op;
            if (is_backward) {
                prev_op.EmitOpCode(&buffer_, 0);
            }
            range_op.InsertOpCode(&buffer_, atom_range);
            goto parseLookBehind;  // NOLINT(cppcoreguidelines-avoid-goto)
            break;
        }
        // P{UnicodePropertyValueExpression}
        // p{UnicodePropertyValueExpression}
        case 'P':
        case 'p':
        // [+N]kGroupName[?U]
        case 'k': {
            Advance();
            if (c0_ != '<') {
                if (!IsUtf16() || HasNamedCaptures()) {
                    ParseError("expecting group name.");
                    break;
                }
            }
            Advance();
            Prev();
            PandaString name;
            auto **pp = const_cast<const uint8_t **>(&pc_);
            if (!ParseGroupSpecifier(pp, name)) {
                ParseError("GroupName Syntax error.");
                break;
            }
            int postion = FindGroupName(name);
            if (postion < 0) {
                postion = ParseCaptureCount(name.c_str());
                if (postion < 0 && (!IsUtf16() || HasNamedCaptures())) {
                    ParseError("group name not defined");
                    break;
                }
            }
            if (is_backward) {
                BackwardBackReferenceOpCode back_reference_op;
                back_reference_op.EmitOpCode(&buffer_, postion);
            } else {
                BackReferenceOpCode back_reference_op;
                back_reference_op.EmitOpCode(&buffer_, postion);
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            Advance();
            break;
        }
        parseLookBehind : {
            if (is_backward) {
                prev_op.EmitOpCode(&buffer_, 0);
            }
            Advance();
            break;
        }
        default:
            result = ParseCharacterEscape();
            break;
    }
    return result;
}

int RegExpParser::RecountCaptures()
{
    if (total_capture_count_ < 0) {
        const char *name = reinterpret_cast<const char *>(group_names_.buf_);
        total_capture_count_ = ParseCaptureCount(name);
    }
    return total_capture_count_;
}
bool RegExpParser::HasNamedCaptures()
{
    if (has_named_captures_ < 0) {
        RecountCaptures();
    }
    return false;
}

int RegExpParser::ParseCharacterEscape()
{
    // CharacterEscape[U]::
    //     ControlEscape
    //     c ControlLetter
    //     0 [lookahead ∉ DecimalDigit]
    //     HexEscapeSequence
    //     RegExpUnicodeEscapeSequence[?U]
    //     IdentityEscape[?U]
    uint32_t result = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    switch (c0_) {
        // ControlEscape
        case 'f':
            result = '\f';
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ControlEscape %c\n", c0_);
            Advance();
            break;
        case 'n':
            result = '\n';
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ControlEscape %c\n", c0_);
            Advance();
            break;
        case 'r':
            result = '\r';
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ControlEscape %c\n", c0_);
            Advance();
            break;
        case 't':
            result = '\t';
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ControlEscape %c\n", c0_);
            Advance();
            break;
        case 'v':
            result = '\v';
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ControlEscape %c\n", c0_);
            Advance();
            break;
        // c ControlLetter
        case 'c': {
            Advance();
            if ((c0_ >= 'A' && c0_ <= 'Z') || (c0_ >= 'a' && c0_ <= 'z')) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("ControlLetter %c\n", c0_);
                result = static_cast<uint32_t>(c0_) & 0x1f;  // NOLINT(readability-magic-numbers, hicpp-signed-bitwise)
                Advance();
            } else {
                if (!IsUtf16()) {
                    pc_--;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    result = '\\';
                } else {
                    ParseError("Invalid control letter");
                    return -1;
                }
            }
            break;
        }
        case '0': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("CharacterEscape 0 [lookahead ∉ DecimalDigit]\n");
            if (IsUtf16() && !(*pc_ >= '0' && *pc_ <= '9')) {  // NOLINT(readability-magic-numbers)
                Advance();
                result = 0;
                break;
            }
            [[fallthrough]];
        }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            if (IsUtf16()) {
                // With /u, decimal escape is not interpreted as octal character code.
                ParseError("Invalid class escape");
                return 0;
            }
            result = ParseOctalLiteral();
            break;
        }
        // ParseHexEscapeSequence
        // ParseRegExpUnicodeEscapeSequence
        case 'x': {
            Advance();
            if (ParseHexEscape(UNICODE_HEX_ADVANCE, &result)) {
                return result;
            }
            if (IsUtf16()) {
                ParseError("Invalid class escape");
                return -1;
            }
            result = 'x';
            break;
        }
        case 'u': {
            Advance();
            if (ParseUnicodeEscape(&result)) {
                return result;
            }
            if (IsUtf16()) {
                // With /u, invalid escapes are not treated as identity escapes.
                ParseError("Invalid unicode escape");
                return 0;
            }
            // If \u is not followed by a two-digit hexadecimal, treat it
            // as an identity escape.
            result = 'u';
            break;
        }
        // IdentityEscape[?U]
        case '$':
        case '(':
        case ')':
        case '*':
        case '+':
        case '.':
        case '/':
        case '?':
        case '[':
        case '\\':
        case ']':
        case '^':
        case '{':
        case '|':
        case '}':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("IdentityEscape %c\n", c0_);
            result = c0_;
            Advance();
            break;
        default: {
            if (IsUtf16()) {
                ParseError("Invalid unicode escape");
                return 0;
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("SourceCharacter %c\n", c0_);
            result = c0_;
            if (result < CHAR_MAXS) {
                Advance();
            }
            break;
        }
    }
    return result;
}

bool RegExpParser::ParseClassRanges(RangeSet *result)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse ClassRanges------\n");
    while (c0_ != ']') {
        RangeSet s1;
        uint32_t c1 = ParseClassAtom(&s1);
        if (c1 == UINT32_MAX) {
            ParseError("invalid class range");
            return false;
        }

        int next_c0 = *pc_;
        if (c0_ == '-' && next_c0 != ']') {
            if (c1 == CLASS_RANGE_BASE) {
                if (IsUtf16()) {
                    ParseError("invalid class range");
                    return false;
                }
                result->Insert(s1);
                continue;
            }
            Advance();
            RangeSet s2;
            uint32_t c2 = ParseClassAtom(&s2);
            if (c2 == UINT32_MAX) {
                ParseError("invalid class range");
                return false;
            }
            if (c2 == CLASS_RANGE_BASE) {
                if (IsUtf16()) {
                    ParseError("invalid class range");
                    return false;
                }
                result->Insert(s2);
                continue;
            }
            if (c1 < INT8_MAX) {
                if (c1 > c2) {
                    ParseError("invalid class range");
                    return false;
                }
            }
            if (IsIgnoreCase()) {
                c1 = static_cast<uint32_t>(Canonicalize(c1, IsUtf16()));
                c2 = static_cast<uint32_t>(Canonicalize(c2, IsUtf16()));
            }

            result->Insert(c1, c2);
        } else {
            result->Insert(s1);
        }
    }
    Advance();
    return true;
}

uint32_t RegExpParser::ParseClassAtom(RangeSet *atom)
{
    uint32_t ret = UINT32_MAX;
    switch (c0_) {
        case '\\': {
            Advance();
            ret = static_cast<uint32_t>(ParseClassEscape(atom));
            break;
        }
        case KEY_EOF:
            break;
        case 0: {
            if (pc_ >= end_) {
                return UINT32_MAX;
            }
            [[fallthrough]];
        }
        default: {
            uint32_t value = c0_;
            size_t u16_size = 0;
            if (c0_ > INT8_MAX) {
                pc_ -= 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                auto u16_result = utf::ConvertUtf8ToUtf16Pair(pc_, true);
                value = u16_result.first;
                u16_size = u16_result.second;
                Advance(u16_size + 1);
            } else {
                Advance();
            }
            if (IsIgnoreCase()) {
                value = static_cast<uint32_t>(Canonicalize(value, IsUtf16()));
            }
            atom->Insert(RangeSet(value));
            ret = value;
            break;
        }
    }
    return ret;
}

int RegExpParser::ParseClassEscape(RangeSet *atom)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse ClassEscape------\n");
    int result = -1;
    switch (c0_) {
        case 'b':
            Advance();
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ClassEscape %c", 'b');
            result = '\b';
            atom->Insert(RangeSet(static_cast<uint32_t>('\b')));
            break;
        case '-':
            Advance();
            result = '-';
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ClassEscape %c", '-');
            atom->Insert(RangeSet(static_cast<uint32_t>('-')));
            break;
        // CharacterClassEscape
        case 'd':
        case 'D':
            result = CLASS_RANGE_BASE;
            atom->Insert(G_RANGE_D);
            if (c0_ == 'D') {
                atom->Invert(IsUtf16());
            }
            Advance();
            break;
        case 's':
        case 'S':
            result = CLASS_RANGE_BASE;
            atom->Insert(G_RANGE_S);
            if (c0_ == 'S') {
                atom->Invert(IsUtf16());
            }
            Advance();
            break;
        case 'w':
        case 'W':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ClassEscape::CharacterClassEscape %c\n", c0_);
            result = CLASS_RANGE_BASE;
            atom->Insert(G_RANGE_W);
            if (c0_ == 'W') {
                atom->Invert(IsUtf16());
            }
            Advance();
            break;
        // P{UnicodePropertyValueExpression}
        // p{UnicodePropertyValueExpression}
        case 'P':
        case 'p':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Warning: \\p is not supported in ECMA 2015!");
            Advance();
            if (c0_ == '{') {
                Advance();
                if (c0_ == '}') {
                    break;  // p{}, invalid
                }
                bool is_value = false;
                ParseUnicodePropertyValueCharacters(&is_value);
                if (!is_value && c0_ == '=') {
                    // UnicodePropertyName = UnicodePropertyValue
                    Advance();
                    if (c0_ == '}') {
                        break;  // p{xxx=}, invalid
                    }
                    ParseUnicodePropertyValueCharacters(&is_value);
                }
                if (c0_ != '}') {
                    break;  // p{xxx, invalid
                }
                // should do atom->Invert() here after ECMA 9.0
                Advance();
                result = CLASS_RANGE_BASE;
            }
            break;
        default:
            result = ParseCharacterEscape();
            int value = result;
            if (IsIgnoreCase()) {
                value = Canonicalize(value, IsUtf16());
            }
            atom->Insert(RangeSet(static_cast<uint32_t>(value)));
            break;
    }
    return result;
}

void RegExpParser::ParseUnicodePropertyValueCharacters(bool *is_value)
{
    if ((c0_ >= 'A' && c0_ <= 'Z') || (c0_ >= 'a' && c0_ <= 'z')) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        PrintF("UnicodePropertyCharacter::ControlLetter %c\n", c0_);
    } else if (c0_ == '_') {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        PrintF("UnicodePropertyCharacter:: _ \n");
    } else if (c0_ >= '0' && c0_ <= '9') {
        *is_value = true;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        PrintF("UnicodePropertyValueCharacter::DecimalDigit %c\n", c0_);
    } else {
        return;
    }
    Advance();
    ParseUnicodePropertyValueCharacters(is_value);
}

// NOLINTNEXTLINE(cert-dcl50-cpp)
void RegExpParser::PrintF(const char *fmt, ...)
{
#ifndef _NO_DEBUG_
    va_list args;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,)
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
#else
    (void)fmt;
#endif
}

void RegExpParser::ParseError(const char *error_message)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("error: ");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF(error_message);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("\n");
    SetIsError();
    size_t length = strlen(error_message) + 1;
    if (memcpy_s(error_msg_, length, error_message, length) != EOK) {
        LOG(FATAL, COMMON) << "memcpy_s failed";
        UNREACHABLE();
    }
}

int RegExpParser::IsIdentFirst(uint32_t c)
{
    if (c < CACHE_SIZE) {
        // NOLINTNEXTLINE(hicpp-signed-bitwise
        return (ID_START_TABLE_ASCII[c >> 5] >> (c & 31)) & 1;  // 5: Shift five bits 31: and operation binary of 31
    }
    return static_cast<int>(u_isIDStart(c));
}
}  // namespace panda