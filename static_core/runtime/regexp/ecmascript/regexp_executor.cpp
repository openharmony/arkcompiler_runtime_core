/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "runtime/regexp/ecmascript/regexp_executor.h"
#include "runtime/regexp/ecmascript/regexp_opcode.h"
#include "runtime/regexp/ecmascript/mem/dyn_chunk.h"
#include "utils/logger.h"
#include "securec.h"

namespace panda {
using RegExpState = RegExpExecutor::RegExpState;
bool RegExpExecutor::Execute(const uint8_t *input, uint32_t last_index, uint32_t length, uint8_t *buf,
                             bool is_wide_char)
{
    DynChunk buffer(buf);
    input_ = const_cast<uint8_t *>(input);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    input_end_ = const_cast<uint8_t *>(input + length * (is_wide_char ? WIDE_CHAR_SIZE : CHAR_SIZE));
    uint32_t size = buffer.GetU32(0);
    n_capture_ = buffer.GetU32(RegExpParser::NUM_CAPTURE__OFFSET);
    n_stack_ = buffer.GetU32(RegExpParser::NUM_STACK_OFFSET);
    flags_ = buffer.GetU32(RegExpParser::FLAGS_OFFSET);
    is_wide_char_ = is_wide_char;

    uint32_t capture_result_size = sizeof(CaptureState) * n_capture_;
    uint32_t stack_size = sizeof(uintptr_t) * n_stack_;
    state_size_ = sizeof(RegExpState) + capture_result_size + stack_size;
    state_stack_len_ = 0;

    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();

    if (capture_result_size != 0) {
        allocator->DeleteArray(capture_result_list_);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        capture_result_list_ = allocator->New<CaptureState[]>(n_capture_);
        if (memset_s(capture_result_list_, capture_result_size, 0, capture_result_size) != EOK) {
            LOG(FATAL, COMMON) << "memset_s failed";
            UNREACHABLE();
        }
    }
    if (stack_size != 0) {
        allocator->DeleteArray(stack_);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        stack_ = allocator->New<uintptr_t[]>(n_stack_);
        if (memset_s(stack_, stack_size, 0, stack_size) != EOK) {
            LOG(FATAL, COMMON) << "memset_s failed";
            UNREACHABLE();
        }
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    SetCurrentPtr(input + last_index * (is_wide_char ? WIDE_CHAR_SIZE : CHAR_SIZE));
    SetCurrentPC(RegExpParser::OP_START_OFFSET);

    // first split
    if ((flags_ & RegExpParser::FLAG_STICKY) == 0) {
        PushRegExpState(STATE_SPLIT, RegExpParser::OP_START_OFFSET);
    }
    return ExecuteInternal(buffer, size);
}

bool RegExpExecutor::MatchFailed(bool is_matched)
{
    while (true) {
        if (state_stack_len_ == 0) {
            return true;
        }
        RegExpState *state = PeekRegExpState();
        if (state->type == StateType::STATE_SPLIT) {
            if (!is_matched) {
                PopRegExpState();
                return false;
            }
        } else {
            is_matched = (state->type == StateType::STATE_MATCH_AHEAD && is_matched) ||
                         (state->type == StateType::STATE_NEGATIVE_MATCH_AHEAD && !is_matched);
            if (is_matched) {
                if (state->type == StateType::STATE_MATCH_AHEAD) {
                    PopRegExpState(false);
                    return false;
                }
                if (state->type == StateType::STATE_NEGATIVE_MATCH_AHEAD) {
                    PopRegExpState();
                    return false;
                }
            }
        }
        DropRegExpState();
    }

    return true;
}

// NOLINTNEXTLINE(readability-function-size)
bool RegExpExecutor::ExecuteInternal(const DynChunk &byte_code, uint32_t pc_end)
{
    while (GetCurrentPC() < pc_end) {
        // first split
        if (!HandleFirstSplit()) {
            return false;
        }
        uint8_t op_code = byte_code.GetU8(GetCurrentPC());
        switch (op_code) {
            case RegExpOpCode::OP_DOTS:
            case RegExpOpCode::OP_ALL: {
                if (!HandleOpAll(op_code)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_CHAR32:
            case RegExpOpCode::OP_CHAR: {
                if (!HandleOpChar(byte_code, op_code)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_NOT_WORD_BOUNDARY:
            case RegExpOpCode::OP_WORD_BOUNDARY: {
                if (!HandleOpWordBoundary(op_code)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_LINE_START: {
                if (!HandleOpLineStart(op_code)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_LINE_END: {
                if (!HandleOpLineEnd(op_code)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_SAVE_START:
                HandleOpSaveStart(byte_code, op_code);
                break;
            case RegExpOpCode::OP_SAVE_END:
                HandleOpSaveEnd(byte_code, op_code);
                break;
            case RegExpOpCode::OP_GOTO: {
                uint32_t offset = byte_code.GetU32(GetCurrentPC() + 1);
                Advance(op_code, offset);
                break;
            }
            case RegExpOpCode::OP_MATCH: {
                // jump to match ahead
                if (MatchFailed(true)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_MATCH_END:
                return true;
            case RegExpOpCode::OP_SAVE_RESET:
                HandleOpSaveReset(byte_code, op_code);
                break;
            case RegExpOpCode::OP_SPLIT_NEXT:
            case RegExpOpCode::OP_MATCH_AHEAD:
            case RegExpOpCode::OP_NEGATIVE_MATCH_AHEAD:
                HandleOpMatch(byte_code, op_code);
                break;
            case RegExpOpCode::OP_SPLIT_FIRST:
                HandleOpSplitFirst(byte_code, op_code);
                break;
            case RegExpOpCode::OP_PREV: {
                if (!HandleOpPrev(op_code)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_LOOP_GREEDY:
            case RegExpOpCode::OP_LOOP:
                HandleOpLoop(byte_code, op_code);
                break;
            case RegExpOpCode::OP_PUSH_CHAR: {
                PushStack(reinterpret_cast<uintptr_t>(GetCurrentPtr()));
                Advance(op_code);
                break;
            }
            case RegExpOpCode::OP_CHECK_CHAR: {
                if (PopStack() != reinterpret_cast<uintptr_t>(GetCurrentPtr())) {
                    Advance(op_code);
                } else {
                    uint32_t offset = byte_code.GetU32(GetCurrentPC() + 1);
                    Advance(op_code, offset);
                }
                break;
            }
            case RegExpOpCode::OP_PUSH: {
                PushStack(0);
                Advance(op_code);
                break;
            }
            case RegExpOpCode::OP_POP: {
                PopStack();
                Advance(op_code);
                break;
            }
            case RegExpOpCode::OP_RANGE32: {
                if (!HandleOpRange32(byte_code)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_RANGE: {
                if (!HandleOpRange(byte_code)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_BACKREFERENCE:
            case RegExpOpCode::OP_BACKWARD_BACKREFERENCE: {
                if (!HandleOpBackReference(byte_code, op_code)) {
                    return false;
                }
                break;
            }
            default:
                UNREACHABLE();
        }
    }
    // for loop match
    return true;
}

void RegExpExecutor::DumpResult(std::ostream &out) const
{
    out << "captures:" << std::endl;
    for (uint32_t i = 0; i < n_capture_; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CaptureState *capture_state = &capture_result_list_[i];
        int32_t len = capture_state->capture_end - capture_state->capture_start;
        if ((capture_state->capture_start != nullptr && capture_state->capture_end != nullptr) && (len >= 0)) {
            out << i << ":\t" << PandaString(reinterpret_cast<const char *>(capture_state->capture_start), len)
                << std::endl;
        } else {
            out << i << ":\t"
                << "undefined" << std::endl;
        }
    }
}

void RegExpExecutor::PushRegExpState(StateType type, uint32_t pc)
{
    ReAllocStack(state_stack_len_ + 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto state = reinterpret_cast<RegExpState *>(state_stack_ + state_stack_len_ * state_size_);
    state->type = type;
    state->current_pc = pc;
    state->current_stack = current_stack_;
    state->current_ptr = GetCurrentPtr();
    size_t list_size = sizeof(CaptureState) * n_capture_;
    if (memcpy_s(state->capture_result_list, list_size, GetCaptureResultList(), list_size) != EOK) {
        LOG(FATAL, COMMON) << "memcpy_s failed";
        UNREACHABLE();
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint8_t *stack_start = reinterpret_cast<uint8_t *>(state->capture_result_list) + sizeof(CaptureState) * n_capture_;
    if (stack_ != nullptr) {
        size_t stack_size = sizeof(uintptr_t) * n_stack_;
        if (memcpy_s(stack_start, stack_size, stack_, stack_size) != EOK) {
            LOG(FATAL, COMMON) << "memcpy_s failed";
            UNREACHABLE();
        }
    }
    state_stack_len_++;
}

RegExpState *RegExpExecutor::PopRegExpState(bool copy_captrue)
{
    if (state_stack_len_ != 0) {
        auto state = PeekRegExpState();
        size_t list_size = sizeof(CaptureState) * n_capture_;
        if (copy_captrue) {
            if (memcpy_s(GetCaptureResultList(), list_size, state->capture_result_list, list_size) != EOK) {
                LOG(FATAL, COMMON) << "memcpy_s failed";
                UNREACHABLE();
            }
        }
        SetCurrentPtr(state->current_ptr);
        SetCurrentPC(state->current_pc);
        current_stack_ = state->current_stack;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint8_t *stack_start = reinterpret_cast<uint8_t *>(state->capture_result_list) + list_size;
        if (stack_ != nullptr) {
            size_t stack_size = sizeof(uintptr_t) * n_stack_;
            if (memcpy_s(stack_, stack_size, stack_start, stack_size) != EOK) {
                LOG(FATAL, COMMON) << "memcpy_s failed";
                UNREACHABLE();
            }
        }
        state_stack_len_--;
        return state;
    }
    return nullptr;
}

void RegExpExecutor::ReAllocStack(uint32_t stack_len)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    if (stack_len > state_stack_size_) {
        uint32_t new_stack_size = std::max(state_stack_size_ * 2, MIN_STACK_SIZE);  // 2: double the size
        uint32_t stack_byte_size = new_stack_size * state_size_;
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        auto new_stack = allocator->New<uint8_t[]>(stack_byte_size);
        if (memset_s(new_stack, stack_byte_size, 0, stack_byte_size) != EOK) {
            LOG(FATAL, COMMON) << "memset_s failed";
            UNREACHABLE();
        }
        if (state_stack_ != nullptr) {
            size_t stack_size = state_stack_size_ * state_size_;
            if (memcpy_s(new_stack, stack_size, state_stack_, stack_size) != EOK) {
                return;
            }
        }
        allocator->DeleteArray(state_stack_);
        state_stack_ = new_stack;
        state_stack_size_ = new_stack_size;
    }
}
}  // namespace panda
