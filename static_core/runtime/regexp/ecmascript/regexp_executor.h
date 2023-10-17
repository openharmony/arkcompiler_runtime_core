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

#ifndef PANDA_RUNTIME_REGEXP_EXECUTOR_H
#define PANDA_RUNTIME_REGEXP_EXECUTOR_H

#include "runtime/regexp/ecmascript/regexp_parser.h"

namespace panda {

template <class T>
struct RegExpMatchResult {
    uint32_t end_index = 0;
    uint32_t index = 0;
    // first value is true if result is undefined
    PandaVector<std::pair<bool, T>> captures;
    bool is_success = false;
};

class RegExpExecutor {
public:
    struct CaptureState {
        const uint8_t *capture_start;
        const uint8_t *capture_end;
    };

    enum StateType : uint8_t {
        STATE_SPLIT = 0,
        STATE_MATCH_AHEAD,
        STATE_NEGATIVE_MATCH_AHEAD,
    };

    struct RegExpState {
        StateType type = STATE_SPLIT;
        uint32_t current_pc = 0;
        uint32_t current_stack = 0;
        const uint8_t *current_ptr = nullptr;
        __extension__ CaptureState *capture_result_list[0];  // NOLINT(modernize-avoid-c-arrays)
    };

    explicit RegExpExecutor() = default;

    ~RegExpExecutor()
    {
        auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
        allocator->DeleteArray(stack_);
        allocator->DeleteArray(capture_result_list_);
        allocator->DeleteArray(state_stack_);
    }

    NO_COPY_SEMANTIC(RegExpExecutor);
    NO_MOVE_SEMANTIC(RegExpExecutor);

    PANDA_PUBLIC_API bool Execute(const uint8_t *input, uint32_t last_index, uint32_t length, uint8_t *buf,
                                  bool is_wide_char = false);

    bool ExecuteInternal(const DynChunk &byte_code, uint32_t pc_end);
    inline bool HandleFirstSplit()
    {
        if (GetCurrentPC() == RegExpParser::OP_START_OFFSET && state_stack_len_ == 0 &&
            (flags_ & RegExpParser::FLAG_STICKY) == 0) {
            if (IsEOF()) {
                if (MatchFailed()) {
                    return false;
                }
            } else {
                AdvanceCurrentPtr();
                PushRegExpState(STATE_SPLIT, RegExpParser::OP_START_OFFSET);
            }
        }
        return true;
    }

    inline bool HandleOpAll(uint8_t op_code)
    {
        if (IsEOF()) {
            return !MatchFailed();
        }
        uint32_t current_char = GetCurrentChar();
        if ((op_code == RegExpOpCode::OP_DOTS) && IsTerminator(current_char)) {
            return !MatchFailed();
        }
        Advance(op_code);
        return true;
    }

    inline bool HandleOpChar(const DynChunk &byte_code, uint8_t op_code)
    {
        uint32_t expected_char;
        if (op_code == RegExpOpCode::OP_CHAR32) {
            expected_char = byte_code.GetU32(GetCurrentPC() + 1);
        } else {
            expected_char = byte_code.GetU16(GetCurrentPC() + 1);
        }
        if (IsEOF()) {
            return !MatchFailed();
        }
        uint32_t current_char = GetCurrentChar();
        if (IsIgnoreCase()) {
            current_char = static_cast<uint32_t>(RegExpParser::Canonicalize(current_char, IsUtf16()));
        }
        if (current_char == expected_char) {
            Advance(op_code);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    inline bool HandleOpWordBoundary(uint8_t op_code)
    {
        if (IsEOF()) {
            if (op_code == RegExpOpCode::OP_WORD_BOUNDARY) {
                Advance(op_code);
            } else {
                if (MatchFailed()) {
                    return false;
                }
            }
            return true;
        }
        bool pre_is_word = false;
        if (GetCurrentPtr() != input_) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            pre_is_word = IsWordChar(PeekPrevChar(current_ptr_, input_));
        }
        bool current_is_word = IsWordChar(PeekChar(current_ptr_, input_end_));
        if (((op_code == RegExpOpCode::OP_WORD_BOUNDARY) &&
             ((!pre_is_word && current_is_word) || (pre_is_word && !current_is_word))) ||
            ((op_code == RegExpOpCode::OP_NOT_WORD_BOUNDARY) &&
             ((pre_is_word && current_is_word) || (!pre_is_word && !current_is_word)))) {
            Advance(op_code);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    inline bool HandleOpLineStart(uint8_t op_code)
    {
        if (IsEOF()) {
            return !MatchFailed();
        }
        if ((GetCurrentPtr() == input_) ||
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ((flags_ & RegExpParser::FLAG_MULTILINE) != 0 && PeekPrevChar(current_ptr_, input_) == '\n')) {
            Advance(op_code);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    inline bool HandleOpLineEnd(uint8_t op_code)
    {
        if (IsEOF() ||
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            ((flags_ & RegExpParser::FLAG_MULTILINE) != 0 && PeekChar(current_ptr_, input_end_) == '\n')) {
            Advance(op_code);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    inline void HandleOpSaveStart(const DynChunk &byte_code, uint8_t op_code)
    {
        uint32_t capture_index = byte_code.GetU8(GetCurrentPC() + 1);
        ASSERT(capture_index < n_capture_);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CaptureState *capture_state = &capture_result_list_[capture_index];
        capture_state->capture_start = GetCurrentPtr();
        Advance(op_code);
    }

    inline void HandleOpSaveEnd(const DynChunk &byte_code, uint8_t op_code)
    {
        uint32_t capture_index = byte_code.GetU8(GetCurrentPC() + 1);
        ASSERT(capture_index < n_capture_);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CaptureState *capture_state = &capture_result_list_[capture_index];
        capture_state->capture_end = GetCurrentPtr();
        Advance(op_code);
    }

    inline void HandleOpSaveReset(const DynChunk &byte_code, uint8_t op_code)
    {
        uint32_t catpure_start_index = byte_code.GetU8(GetCurrentPC() + SAVE_RESET_START);
        uint32_t catpure_end_index = byte_code.GetU8(GetCurrentPC() + SAVE_RESET_END);
        for (uint32_t i = catpure_start_index; i <= catpure_end_index; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            CaptureState *capture_state = &capture_result_list_[i];
            capture_state->capture_start = nullptr;
            capture_state->capture_end = nullptr;
        }
        Advance(op_code);
    }

    inline void HandleOpMatch(const DynChunk &byte_code, uint8_t op_code)
    {
        auto type = static_cast<StateType>(op_code - RegExpOpCode::OP_SPLIT_NEXT);
        ASSERT(type == STATE_SPLIT || type == STATE_MATCH_AHEAD || type == STATE_NEGATIVE_MATCH_AHEAD);
        uint32_t offset = byte_code.GetU32(GetCurrentPC() + 1);
        Advance(op_code);
        uint32_t split_pc = GetCurrentPC() + offset;
        PushRegExpState(type, split_pc);
    }

    inline void HandleOpSplitFirst(const DynChunk &byte_code, uint8_t op_code)
    {
        uint32_t offset = byte_code.GetU32(GetCurrentPC() + 1);
        Advance(op_code);
        PushRegExpState(STATE_SPLIT, GetCurrentPC());
        AdvanceOffset(offset);
    }

    inline bool HandleOpPrev(uint8_t op_code)
    {
        if (GetCurrentPtr() == input_) {
            if (MatchFailed()) {
                return false;
            }
        } else {
            PrevPtr(&current_ptr_, input_);
            Advance(op_code);
        }
        return true;
    }

    inline void HandleOpLoop(const DynChunk &byte_code, uint8_t op_code)
    {
        uint32_t quantify_min = byte_code.GetU32(GetCurrentPC() + LOOP_MIN_OFFSET);
        uint32_t quantify_max = byte_code.GetU32(GetCurrentPC() + LOOP_MAX_OFFSET);
        uint32_t pc_offset = byte_code.GetU32(GetCurrentPC() + LOOP_PC_OFFSET);
        Advance(op_code);
        uint32_t loop_pc_end = GetCurrentPC();
        uint32_t loop_pc_start = GetCurrentPC() + pc_offset;
        bool is_greedy = op_code == RegExpOpCode::OP_LOOP_GREEDY;
        uint32_t loop_max = is_greedy ? quantify_max : quantify_min;

        uint32_t loop_count = PeekStack();
        SetStackValue(++loop_count);
        if (loop_count < loop_max) {
            // greedy failed, goto next
            if (loop_count >= quantify_min) {
                PushRegExpState(STATE_SPLIT, loop_pc_end);
            }
            // Goto loop start
            SetCurrentPC(loop_pc_start);
        } else {
            if (!is_greedy && (loop_count < quantify_max)) {
                PushRegExpState(STATE_SPLIT, loop_pc_start);
            }
        }
    }

    inline bool HandleOpRange32(const DynChunk &byte_code)
    {
        if (IsEOF()) {
            return !MatchFailed();
        }
        uint32_t current_char = GetCurrentChar();
        if (IsIgnoreCase()) {
            current_char = static_cast<uint32_t>(RegExpParser::Canonicalize(current_char, IsUtf16()));
        }
        uint16_t range_count = byte_code.GetU16(GetCurrentPC() + 1);
        bool is_found = false;
        int32_t idx_min = 0;
        int32_t idx_max = static_cast<int32_t>(range_count) - 1;
        int32_t idx = 0;
        uint32_t low = 0;
        uint32_t high = byte_code.GetU32(GetCurrentPC() + RANGE32_HEAD_OFFSET + idx_max * RANGE32_MAX_OFFSET +
                                         RANGE32_MAX_HALF_OFFSET);
        if (current_char <= high) {
            while (idx_min <= idx_max) {
                idx = (idx_min + idx_max) / RANGE32_OFFSET;
                low = byte_code.GetU32(GetCurrentPC() + RANGE32_HEAD_OFFSET +
                                       static_cast<uint32_t>(idx) * RANGE32_MAX_OFFSET);
                high = byte_code.GetU32(GetCurrentPC() + RANGE32_HEAD_OFFSET +
                                        static_cast<uint32_t>(idx) * RANGE32_MAX_OFFSET + RANGE32_MAX_HALF_OFFSET);
                if (current_char < low) {
                    idx_max = idx - 1;
                } else if (current_char > high) {
                    idx_min = idx + 1;
                } else {
                    is_found = true;
                    break;
                }
            }
        }
        if (is_found) {
            AdvanceOffset(range_count * RANGE32_MAX_OFFSET + RANGE32_HEAD_OFFSET);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    inline bool HandleOpRange(const DynChunk &byte_code)
    {
        if (IsEOF()) {
            return !MatchFailed();
        }
        uint32_t current_char = GetCurrentChar();
        if (IsIgnoreCase()) {
            current_char = static_cast<uint32_t>(RegExpParser::Canonicalize(current_char, IsUtf16()));
        }
        uint16_t range_count = byte_code.GetU16(GetCurrentPC() + 1);
        bool is_found = false;
        int32_t idx_min = 0;
        int32_t idx_max = range_count - 1;
        int32_t idx = 0;
        uint32_t low = 0;
        uint32_t high =
            byte_code.GetU16(GetCurrentPC() + RANGE32_HEAD_OFFSET + idx_max * RANGE32_MAX_HALF_OFFSET + RANGE32_OFFSET);
        if (current_char <= high) {
            while (idx_min <= idx_max) {
                idx = (idx_min + idx_max) / RANGE32_OFFSET;
                low = byte_code.GetU16(GetCurrentPC() + RANGE32_HEAD_OFFSET +
                                       static_cast<uint32_t>(idx) * RANGE32_MAX_HALF_OFFSET);
                high = byte_code.GetU16(GetCurrentPC() + RANGE32_HEAD_OFFSET +
                                        static_cast<uint32_t>(idx) * RANGE32_MAX_HALF_OFFSET + RANGE32_OFFSET);
                if (current_char < low) {
                    idx_max = idx - 1;
                } else if (current_char > high) {
                    idx_min = idx + 1;
                } else {
                    is_found = true;
                    break;
                }
            }
        }
        if (is_found) {
            AdvanceOffset(range_count * RANGE32_MAX_HALF_OFFSET + RANGE32_HEAD_OFFSET);
        } else {
            if (MatchFailed()) {
                return false;
            }
        }
        return true;
    }

    inline bool HandleOpBackReference(const DynChunk &byte_code, uint8_t op_code)
    {
        uint32_t capture_index = byte_code.GetU8(GetCurrentPC() + 1);
        if (capture_index >= n_capture_) {
            return !MatchFailed();
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const uint8_t *capture_start = capture_result_list_[capture_index].capture_start;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const uint8_t *capture_end = capture_result_list_[capture_index].capture_end;
        if (capture_start == nullptr || capture_end == nullptr) {
            Advance(op_code);
            return true;
        }
        bool is_matched = true;
        if (op_code == RegExpOpCode::OP_BACKREFERENCE) {
            const uint8_t *ref_cptr = capture_start;
            while (ref_cptr < capture_end) {
                if (IsEOF()) {
                    is_matched = false;
                    break;
                }
                // NOLINTNEXTLINE(readability-identifier-naming)
                uint32_t c1 = GetChar(&ref_cptr, capture_end);
                // NOLINTNEXTLINE(readability-identifier-naming)
                uint32_t c2 = GetChar(&current_ptr_, input_end_);
                if (IsIgnoreCase()) {
                    c1 = static_cast<uint32_t>(RegExpParser::Canonicalize(c1, IsUtf16()));
                    c2 = static_cast<uint32_t>(RegExpParser::Canonicalize(c2, IsUtf16()));
                }
                if (c1 != c2) {
                    is_matched = false;
                    break;
                }
            }
            if (!is_matched) {
                if (MatchFailed()) {
                    return false;
                }
            } else {
                Advance(op_code);
            }
        } else {
            const uint8_t *ref_cptr = capture_end;
            while (ref_cptr > capture_start) {
                if (GetCurrentPtr() == input_) {
                    is_matched = false;
                    break;
                }
                // NOLINTNEXTLINE(readability-identifier-naming)
                uint32_t c1 = GetPrevChar(&ref_cptr, capture_start);
                // NOLINTNEXTLINE(readability-identifier-naming)
                uint32_t c2 = GetPrevChar(&current_ptr_, input_);
                if (IsIgnoreCase()) {
                    c1 = static_cast<uint32_t>(RegExpParser::Canonicalize(c1, IsUtf16()));
                    c2 = static_cast<uint32_t>(RegExpParser::Canonicalize(c2, IsUtf16()));
                }
                if (c1 != c2) {
                    is_matched = false;
                    break;
                }
            }
            if (!is_matched) {
                if (MatchFailed()) {
                    return false;
                }
            } else {
                Advance(op_code);
            }
        }
        return true;
    }

    inline void Advance(uint8_t op_code, uint32_t offset = 0)
    {
        current_pc_ += offset + static_cast<uint32_t>(RegExpOpCode::GetRegExpOpCode(op_code)->GetSize());
    }

    inline void AdvanceOffset(uint32_t offset)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        current_pc_ += offset;
    }

    inline uint32_t GetCurrentChar()
    {
        return GetChar(&current_ptr_, input_end_);
    }

    inline void AdvanceCurrentPtr()
    {
        AdvancePtr(&current_ptr_, input_end_);
    }

    uint32_t GetChar(const uint8_t **pp, const uint8_t *end) const
    {
        uint32_t c;
        const uint8_t *cptr = *pp;
        if (!is_wide_char_) {
            c = *cptr;
            *pp += 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else {
            uint16_t c1 = *(reinterpret_cast<const uint16_t *>(cptr));
            c = c1;
            cptr += WIDE_CHAR_SIZE;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (U16_IS_LEAD(c) && IsUtf16() && cptr < end) {
                c1 = *(reinterpret_cast<const uint16_t *>(cptr));
                if (U16_IS_TRAIL(c1)) {
                    c = static_cast<uint32_t>(U16_GET_SUPPLEMENTARY(c, c1));  // NOLINT(hicpp-signed-bitwise)
                    cptr += WIDE_CHAR_SIZE;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                }
            }
            *pp = cptr;
        }
        return c;
    }

    uint32_t PeekChar(const uint8_t *p, const uint8_t *end) const
    {
        uint32_t c;
        const uint8_t *cptr = p;
        if (!is_wide_char_) {
            c = *cptr;
        } else {
            uint16_t c1 = *reinterpret_cast<const uint16_t *>(cptr);
            c = c1;
            cptr += WIDE_CHAR_SIZE;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (U16_IS_LEAD(c) && IsUtf16() && cptr < end) {
                c1 = *reinterpret_cast<const uint16_t *>(cptr);
                if (U16_IS_TRAIL(c1)) {
                    c = static_cast<uint32_t>(U16_GET_SUPPLEMENTARY(c, c1));  // NOLINT(hicpp-signed-bitwise)
                }
            }
        }
        return c;
    }

    void AdvancePtr(const uint8_t **pp, const uint8_t *end) const
    {
        const uint8_t *cptr = *pp;
        if (!is_wide_char_) {
            *pp += 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else {
            uint16_t c1 = *reinterpret_cast<const uint16_t *>(cptr);
            cptr += WIDE_CHAR_SIZE;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (U16_IS_LEAD(c1) && IsUtf16() && cptr < end) {
                c1 = *reinterpret_cast<const uint16_t *>(cptr);
                if (U16_IS_TRAIL(c1)) {
                    cptr += WIDE_CHAR_SIZE;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                }
            }
            *pp = cptr;
        }
    }

    uint32_t PeekPrevChar(const uint8_t *p, const uint8_t *start) const
    {
        uint32_t c;
        const uint8_t *cptr = p;
        if (!is_wide_char_) {
            c = cptr[-1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        } else {
            cptr -= WIDE_CHAR_SIZE;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            uint16_t c1 = *reinterpret_cast<const uint16_t *>(cptr);
            c = c1;
            if (U16_IS_TRAIL(c) && IsUtf16() && cptr > start) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                c1 = reinterpret_cast<const uint16_t *>(cptr)[-1];
                if (U16_IS_LEAD(c1)) {
                    c = static_cast<uint32_t>(U16_GET_SUPPLEMENTARY(c1, c));  // NOLINT(hicpp-signed-bitwise)
                }
            }
        }
        return c;
    }

    uint32_t GetPrevChar(const uint8_t **pp, const uint8_t *start) const
    {
        uint32_t c;
        const uint8_t *cptr = *pp;
        if (!is_wide_char_) {
            c = cptr[-1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            cptr -= 1;     // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *pp = cptr;
        } else {
            cptr -= WIDE_CHAR_SIZE;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            uint16_t c1 = *reinterpret_cast<const uint16_t *>(cptr);
            c = c1;
            if (U16_IS_TRAIL(c) && IsUtf16() && cptr > start) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                c1 = reinterpret_cast<const uint16_t *>(cptr)[-1];
                if (U16_IS_LEAD(c1)) {
                    c = static_cast<uint32_t>(U16_GET_SUPPLEMENTARY(c1, c));  // NOLINT(hicpp-signed-bitwise)
                    cptr -= WIDE_CHAR_SIZE;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                }
            }
            *pp = cptr;
        }
        return c;
    }

    void PrevPtr(const uint8_t **pp, const uint8_t *start) const
    {
        const uint8_t *cptr = *pp;
        if (!is_wide_char_) {
            cptr -= 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *pp = cptr;
        } else {
            cptr -= WIDE_CHAR_SIZE;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            uint16_t c1 = *reinterpret_cast<const uint16_t *>(cptr);
            if (U16_IS_TRAIL(c1) && IsUtf16() && cptr > start) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                c1 = reinterpret_cast<const uint16_t *>(cptr)[-1];
                if (U16_IS_LEAD(c1)) {
                    cptr -= WIDE_CHAR_SIZE;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                }
            }
            *pp = cptr;
        }
    }

    bool MatchFailed(bool is_matched = false);

    void SetCurrentPC(uint32_t pc)
    {
        current_pc_ = pc;
    }

    void SetCurrentPtr(const uint8_t *ptr)
    {
        current_ptr_ = ptr;
    }

    bool IsEOF() const
    {
        return current_ptr_ >= input_end_;
    }

    uint32_t GetCurrentPC() const
    {
        return current_pc_;
    }

    void PushStack(uintptr_t val)
    {
        ASSERT(current_stack_ < n_stack_);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        stack_[current_stack_++] = val;
    }

    void SetStackValue(uintptr_t val) const
    {
        ASSERT(current_stack_ >= 1);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        stack_[current_stack_ - 1] = val;
    }

    uintptr_t PopStack()
    {
        ASSERT(current_stack_ >= 1);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return stack_[--current_stack_];
    }

    uintptr_t PeekStack() const
    {
        ASSERT(current_stack_ >= 1);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return stack_[current_stack_ - 1];
    }

    const uint8_t *GetCurrentPtr() const
    {
        return current_ptr_;
    }

    CaptureState *GetCaptureResultList() const
    {
        return capture_result_list_;
    }

    uint32_t GetCaptureCount() const
    {
        return n_capture_;
    }

    void DumpResult(std::ostream &out) const;

    void PushRegExpState(StateType type, uint32_t pc);

    RegExpState *PopRegExpState(bool copy_captrue = true);

    void DropRegExpState()
    {
        state_stack_len_--;
    }

    RegExpState *PeekRegExpState() const
    {
        ASSERT(state_stack_len_ >= 1);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<RegExpState *>(state_stack_ + (state_stack_len_ - 1) * state_size_);
    }

    void ReAllocStack(uint32_t stack_len);

    inline bool IsWordChar(uint8_t value) const
    {
        return ((value >= '0' && value <= '9') || (value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
                (value == '_'));
    }

    inline bool IsTerminator(uint32_t value) const
    {
        // NOLINTNEXTLINE(readability-magic-numbers)
        return (value == '\n' || value == '\r' || value == 0x2028 || value == 0x2029);
    }

    inline bool IsIgnoreCase() const
    {
        return (flags_ & RegExpParser::FLAG_IGNORECASE) != 0;
    }

    inline bool IsUtf16() const
    {
        return (flags_ & RegExpParser::FLAG_UTF16) != 0;
    }

    inline bool IsWideChar() const
    {
        return is_wide_char_;
    }

    inline uint8_t *GetInputPtr() const
    {
        return input_;
    }

    static constexpr size_t CHAR_SIZE = 1;
    static constexpr size_t WIDE_CHAR_SIZE = 2;
    static constexpr size_t SAVE_RESET_START = 1;
    static constexpr size_t SAVE_RESET_END = 2;
    static constexpr size_t LOOP_MIN_OFFSET = 5;
    static constexpr size_t LOOP_MAX_OFFSET = 9;
    static constexpr size_t LOOP_PC_OFFSET = 1;
    static constexpr size_t RANGE32_HEAD_OFFSET = 3;
    static constexpr size_t RANGE32_MAX_HALF_OFFSET = 4;
    static constexpr size_t RANGE32_MAX_OFFSET = 8;
    static constexpr size_t RANGE32_OFFSET = 2;
    static constexpr uint32_t STACK_MULTIPLIER = 2;
    static constexpr uint32_t MIN_STACK_SIZE = 8;

private:
    uint8_t *input_ = nullptr;
    uint8_t *input_end_ = nullptr;
    bool is_wide_char_ = false;

    uint32_t current_pc_ = 0;
    const uint8_t *current_ptr_ = nullptr;
    CaptureState *capture_result_list_ = nullptr;
    uintptr_t *stack_ = nullptr;
    uint32_t current_stack_ = 0;

    uint32_t n_capture_ = 0;
    uint32_t n_stack_ = 0;

    uint32_t flags_ = 0;
    uint32_t state_stack_len_ = 0;
    uint32_t state_stack_size_ = 0;
    uint32_t state_size_ = 0;
    uint8_t *state_stack_ = nullptr;
};
}  // namespace panda
#endif  // core_REGEXP_REGEXP_EXECUTOR_H
