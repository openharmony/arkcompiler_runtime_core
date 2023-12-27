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
bool RegExpExecutor::Execute(const uint8_t *input, uint32_t lastIndex, uint32_t length, uint8_t *buf, bool isWideChar)
{
    DynChunk buffer(buf);
    input_ = const_cast<uint8_t *>(input);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    inputEnd_ = const_cast<uint8_t *>(input + length * (isWideChar ? WIDE_CHAR_SIZE : CHAR_SIZE));
    uint32_t size = buffer.GetU32(0);
    nCapture_ = buffer.GetU32(RegExpParser::NUM_CAPTURE__OFFSET);
    nStack_ = buffer.GetU32(RegExpParser::NUM_STACK_OFFSET);
    flags_ = buffer.GetU32(RegExpParser::FLAGS_OFFSET);
    isWideChar_ = isWideChar;

    uint32_t captureResultSize = sizeof(CaptureState) * nCapture_;
    uint32_t stackSize = sizeof(uintptr_t) * nStack_;
    stateSize_ = sizeof(RegExpState) + captureResultSize + stackSize;
    stateStackLen_ = 0;

    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();

    if (captureResultSize != 0) {
        allocator->DeleteArray(captureResultList_);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        captureResultList_ = allocator->New<CaptureState[]>(nCapture_);
        if (memset_s(captureResultList_, captureResultSize, 0, captureResultSize) != EOK) {
            LOG(FATAL, COMMON) << "memset_s failed";
            UNREACHABLE();
        }
    }
    if (stackSize != 0) {
        allocator->DeleteArray(stack_);
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        stack_ = allocator->New<uintptr_t[]>(nStack_);
        if (memset_s(stack_, stackSize, 0, stackSize) != EOK) {
            LOG(FATAL, COMMON) << "memset_s failed";
            UNREACHABLE();
        }
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    SetCurrentPtr(input + lastIndex * (isWideChar ? WIDE_CHAR_SIZE : CHAR_SIZE));
    SetCurrentPC(RegExpParser::OP_START_OFFSET);

    // first split
    if ((flags_ & RegExpParser::FLAG_STICKY) == 0) {
        PushRegExpState(STATE_SPLIT, RegExpParser::OP_START_OFFSET);
    }
    return ExecuteInternal(buffer, size);
}

bool RegExpExecutor::MatchFailed(bool isMatched)
{
    while (true) {
        if (stateStackLen_ == 0) {
            return true;
        }
        RegExpState *state = PeekRegExpState();
        if (state->type == StateType::STATE_SPLIT) {
            if (!isMatched) {
                PopRegExpState();
                return false;
            }
        } else {
            isMatched = (state->type == StateType::STATE_MATCH_AHEAD && isMatched) ||
                        (state->type == StateType::STATE_NEGATIVE_MATCH_AHEAD && !isMatched);
            if (isMatched) {
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
bool RegExpExecutor::ExecuteInternal(const DynChunk &byteCode, uint32_t pcEnd)
{
    while (GetCurrentPC() < pcEnd) {
        // first split
        if (!HandleFirstSplit()) {
            return false;
        }
        uint8_t opCode = byteCode.GetU8(GetCurrentPC());
        switch (opCode) {
            case RegExpOpCode::OP_DOTS:
            case RegExpOpCode::OP_ALL: {
                if (!HandleOpAll(opCode)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_CHAR32:
            case RegExpOpCode::OP_CHAR: {
                if (!HandleOpChar(byteCode, opCode)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_NOT_WORD_BOUNDARY:
            case RegExpOpCode::OP_WORD_BOUNDARY: {
                if (!HandleOpWordBoundary(opCode)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_LINE_START: {
                if (!HandleOpLineStart(opCode)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_LINE_END: {
                if (!HandleOpLineEnd(opCode)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_SAVE_START:
                HandleOpSaveStart(byteCode, opCode);
                break;
            case RegExpOpCode::OP_SAVE_END:
                HandleOpSaveEnd(byteCode, opCode);
                break;
            case RegExpOpCode::OP_GOTO: {
                uint32_t offset = byteCode.GetU32(GetCurrentPC() + 1);
                Advance(opCode, offset);
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
                HandleOpSaveReset(byteCode, opCode);
                break;
            case RegExpOpCode::OP_SPLIT_NEXT:
            case RegExpOpCode::OP_MATCH_AHEAD:
            case RegExpOpCode::OP_NEGATIVE_MATCH_AHEAD:
                HandleOpMatch(byteCode, opCode);
                break;
            case RegExpOpCode::OP_SPLIT_FIRST:
                HandleOpSplitFirst(byteCode, opCode);
                break;
            case RegExpOpCode::OP_PREV: {
                if (!HandleOpPrev(opCode)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_LOOP_GREEDY:
            case RegExpOpCode::OP_LOOP:
                HandleOpLoop(byteCode, opCode);
                break;
            case RegExpOpCode::OP_PUSH_CHAR: {
                PushStack(reinterpret_cast<uintptr_t>(GetCurrentPtr()));
                Advance(opCode);
                break;
            }
            case RegExpOpCode::OP_CHECK_CHAR: {
                if (PopStack() != reinterpret_cast<uintptr_t>(GetCurrentPtr())) {
                    Advance(opCode);
                } else {
                    uint32_t offset = byteCode.GetU32(GetCurrentPC() + 1);
                    Advance(opCode, offset);
                }
                break;
            }
            case RegExpOpCode::OP_PUSH: {
                PushStack(0);
                Advance(opCode);
                break;
            }
            case RegExpOpCode::OP_POP: {
                PopStack();
                Advance(opCode);
                break;
            }
            case RegExpOpCode::OP_RANGE32: {
                if (!HandleOpRange32(byteCode)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_RANGE: {
                if (!HandleOpRange(byteCode)) {
                    return false;
                }
                break;
            }
            case RegExpOpCode::OP_BACKREFERENCE:
            case RegExpOpCode::OP_BACKWARD_BACKREFERENCE: {
                if (!HandleOpBackReference(byteCode, opCode)) {
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
    for (uint32_t i = 0; i < nCapture_; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CaptureState *captureState = &captureResultList_[i];
        int32_t len = captureState->captureEnd - captureState->captureStart;
        if ((captureState->captureStart != nullptr && captureState->captureEnd != nullptr) && (len >= 0)) {
            out << i << ":\t" << PandaString(reinterpret_cast<const char *>(captureState->captureStart), len)
                << std::endl;
        } else {
            out << i << ":\t"
                << "undefined" << std::endl;
        }
    }
}

void RegExpExecutor::PushRegExpState(StateType type, uint32_t pc)
{
    ReAllocStack(stateStackLen_ + 1);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto state = reinterpret_cast<RegExpState *>(stateStack_ + stateStackLen_ * stateSize_);
    state->type = type;
    state->currentPc = pc;
    state->currentStack = currentStack_;
    state->currentPtr = GetCurrentPtr();
    size_t listSize = sizeof(CaptureState) * nCapture_;
    if (memcpy_s(state->captureResultList, listSize, GetCaptureResultList(), listSize) != EOK) {
        LOG(FATAL, COMMON) << "memcpy_s failed";
        UNREACHABLE();
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint8_t *stackStart = reinterpret_cast<uint8_t *>(state->captureResultList) + sizeof(CaptureState) * nCapture_;
    if (stack_ != nullptr) {
        size_t stackSize = sizeof(uintptr_t) * nStack_;
        if (memcpy_s(stackStart, stackSize, stack_, stackSize) != EOK) {
            LOG(FATAL, COMMON) << "memcpy_s failed";
            UNREACHABLE();
        }
    }
    stateStackLen_++;
}

RegExpState *RegExpExecutor::PopRegExpState(bool copyCaptrue)
{
    if (stateStackLen_ != 0) {
        auto state = PeekRegExpState();
        size_t listSize = sizeof(CaptureState) * nCapture_;
        if (copyCaptrue) {
            if (memcpy_s(GetCaptureResultList(), listSize, state->captureResultList, listSize) != EOK) {
                LOG(FATAL, COMMON) << "memcpy_s failed";
                UNREACHABLE();
            }
        }
        SetCurrentPtr(state->currentPtr);
        SetCurrentPC(state->currentPc);
        currentStack_ = state->currentStack;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        uint8_t *stackStart = reinterpret_cast<uint8_t *>(state->captureResultList) + listSize;
        if (stack_ != nullptr) {
            size_t stackSize = sizeof(uintptr_t) * nStack_;
            if (memcpy_s(stack_, stackSize, stackStart, stackSize) != EOK) {
                LOG(FATAL, COMMON) << "memcpy_s failed";
                UNREACHABLE();
            }
        }
        stateStackLen_--;
        return state;
    }
    return nullptr;
}

void RegExpExecutor::ReAllocStack(uint32_t stackLen)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    if (stackLen > stateStackSize_) {
        uint32_t newStackSize = std::max(stateStackSize_ * 2, MIN_STACK_SIZE);  // 2: double the size
        uint32_t stackByteSize = newStackSize * stateSize_;
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        auto newStack = allocator->New<uint8_t[]>(stackByteSize);
        if (memset_s(newStack, stackByteSize, 0, stackByteSize) != EOK) {
            LOG(FATAL, COMMON) << "memset_s failed";
            UNREACHABLE();
        }
        if (stateStack_ != nullptr) {
            size_t stackSize = stateStackSize_ * stateSize_;
            if (memcpy_s(newStack, stackSize, stateStack_, stackSize) != EOK) {
                return;
            }
        }
        allocator->DeleteArray(stateStack_);
        stateStack_ = newStack;
        stateStackSize_ = newStackSize;
    }
}
}  // namespace panda
