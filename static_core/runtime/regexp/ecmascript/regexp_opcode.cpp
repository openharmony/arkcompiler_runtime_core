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

#include "runtime/regexp/ecmascript/regexp_opcode.h"
#include "runtime/regexp/ecmascript/regexp_executor.h"

namespace panda {
using CaptureState = RegExpExecutor::CaptureState;

static SaveStartOpCode G_SAVE_START_OPCODE = SaveStartOpCode();     // NOLINT(fuchsia-statically-constructed-objects)
static SaveEndOpCode G_SAVE_END_OPCODE = SaveEndOpCode();           // NOLINT(fuchsia-statically-constructed-objects)
static CharOpCode G_CHAR_OPCODE = CharOpCode();                     // NOLINT(fuchsia-statically-constructed-objects)
static GotoOpCode G_GOTO_OPCODE = GotoOpCode();                     // NOLINT(fuchsia-statically-constructed-objects)
static SplitNextOpCode G_SPLIT_NEXT_OPCODE = SplitNextOpCode();     // NOLINT(fuchsia-statically-constructed-objects)
static SplitFirstOpCode G_SPLIT_FIRST_OPCODE = SplitFirstOpCode();  // NOLINT(fuchsia-statically-constructed-objects)
static MatchOpCode G_MATCH_OPCODE = MatchOpCode();                  // NOLINT(fuchsia-statically-constructed-objects)
static LoopOpCode G_LOOP_OPCODE = LoopOpCode();                     // NOLINT(fuchsia-statically-constructed-objects)
static LoopGreedyOpCode G_LOOP_GREEDY_OPCODE = LoopGreedyOpCode();  // NOLINT(fuchsia-statically-constructed-objects)
static PushCharOpCode G_PUSH_CHAR_OPCODE = PushCharOpCode();        // NOLINT(fuchsia-statically-constructed-objects)
static CheckCharOpCode G_CHECK_CHAR_OPCODE = CheckCharOpCode();     // NOLINT(fuchsia-statically-constructed-objects)
static PushOpCode G_PUSH_OPCODE = PushOpCode();                     // NOLINT(fuchsia-statically-constructed-objects)
static PopOpCode G_POP_OPCODE = PopOpCode();                        // NOLINT(fuchsia-statically-constructed-objects)
static SaveResetOpCode G_SAVE_RESET_OPCODE = SaveResetOpCode();     // NOLINT(fuchsia-statically-constructed-objects)
static LineStartOpCode G_LINE_START_OPCODE = LineStartOpCode();     // NOLINT(fuchsia-statically-constructed-objects)
static LineEndOpCode G_LINE_END_OPCODE = LineEndOpCode();           // NOLINT(fuchsia-statically-constructed-objects)
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static WordBoundaryOpCode G_WORD_BOUNDARY_OPCODE = WordBoundaryOpCode();
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static NotWordBoundaryOpCode G_NOT_WORD_BOUNDARY_OPCODE = NotWordBoundaryOpCode();
static AllOpCode G_ALL_OPCODE = AllOpCode();                        // NOLINT(fuchsia-statically-constructed-objects)
static DotsOpCode G_DOTS_OPCODE = DotsOpCode();                     // NOLINT(fuchsia-statically-constructed-objects)
static MatchAheadOpCode G_MATCH_AHEAD_OPCODE = MatchAheadOpCode();  // NOLINT(fuchsia-statically-constructed-objects)
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static NegativeMatchAheadOpCode G_NEGATIVE_MATCH_AHEAD_OPCODE = NegativeMatchAheadOpCode();
static MatchEndOpCode G_MATCH_END_OPCODE = MatchEndOpCode();  // NOLINT(fuchsia-statically-constructed-objects)
static PrevOpCode G_PREV_OPCODE = PrevOpCode();               // NOLINT(fuchsia-statically-constructed-objects)
static RangeOpCode G_RANGE_OPCODE = RangeOpCode();            // NOLINT(fuchsia-statically-constructed-objects)
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static BackReferenceOpCode G_BACKREFERENCE_OPCODE = BackReferenceOpCode();
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static BackwardBackReferenceOpCode G_BACKWARD_BACKREFERENCE_OPCODE = BackwardBackReferenceOpCode();
static Char32OpCode G_CHAR32_OPCODE = Char32OpCode();     // NOLINT(fuchsia-statically-constructed-objects)
static Range32OpCode G_RANGE32_OPCODE = Range32OpCode();  // NOLINT(fuchsia-statically-constructed-objects)
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::vector<RegExpOpCode *> G_INTRINSIC_SET = {
    &G_SAVE_START_OPCODE,
    &G_SAVE_END_OPCODE,
    &G_CHAR_OPCODE,
    &G_GOTO_OPCODE,
    &G_SPLIT_FIRST_OPCODE,
    &G_SPLIT_NEXT_OPCODE,
    &G_MATCH_AHEAD_OPCODE,
    &G_NEGATIVE_MATCH_AHEAD_OPCODE,
    &G_MATCH_OPCODE,
    &G_LOOP_OPCODE,
    &G_LOOP_GREEDY_OPCODE,
    &G_PUSH_CHAR_OPCODE,
    &G_CHECK_CHAR_OPCODE,
    &G_PUSH_OPCODE,
    &G_POP_OPCODE,
    &G_SAVE_RESET_OPCODE,
    &G_LINE_START_OPCODE,
    &G_LINE_END_OPCODE,
    &G_WORD_BOUNDARY_OPCODE,
    &G_NOT_WORD_BOUNDARY_OPCODE,
    &G_ALL_OPCODE,
    &G_DOTS_OPCODE,
    &G_MATCH_END_OPCODE,
    &G_PREV_OPCODE,
    &G_RANGE_OPCODE,
    &G_BACKREFERENCE_OPCODE,
    &G_BACKWARD_BACKREFERENCE_OPCODE,
    &G_CHAR32_OPCODE,
    &G_RANGE32_OPCODE,
};

RegExpOpCode::RegExpOpCode(uint8_t op_code, int size) : op_code_(op_code), size_(size) {}

/* static */
RegExpOpCode *RegExpOpCode::GetRegExpOpCode(const DynChunk &buf, int pc)
{
    uint8_t op_code = buf.GetU8(pc);
    ASSERT_PRINT(op_code <= G_INTRINSIC_SET.size(), "invalid op code");
    return G_INTRINSIC_SET.at(op_code);
}

/* static */
RegExpOpCode *RegExpOpCode::GetRegExpOpCode(uint8_t op_code)
{
    ASSERT_PRINT(op_code <= G_INTRINSIC_SET.size(), "invalid op code");
    return G_INTRINSIC_SET.at(op_code);
}

/* static */
void RegExpOpCode::DumpRegExpOpCode(std::ostream &out, const DynChunk &buf)
{
    out << "OpCode:\t" << std::endl;
    uint32_t pc = RegExpParser::OP_START_OFFSET;
    do {
        RegExpOpCode *byte_code = GetRegExpOpCode(buf, pc);
        pc = byte_code->DumpOpCode(out, buf, pc);
    } while (pc < buf.size_);
}

uint32_t SaveStartOpCode::EmitOpCode(DynChunk *buf, uint32_t para) const
{
    auto capture = static_cast<uint8_t>(para & 0xffU);  // NOLINT(readability-magic-numbers)
    buf->EmitChar(GetOpCode());
    buf->EmitChar(capture);
    return GetDynChunkfSize(*buf);
}

uint32_t SaveStartOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "save_start\t" << buf.GetU8(offset + 1) << std::endl;
    return offset + GetSize();
}

uint32_t SaveEndOpCode::EmitOpCode(DynChunk *buf, uint32_t para) const
{
    auto capture = static_cast<uint8_t>(para & 0xffU);  // NOLINT(readability-magic-numbers)
    buf->EmitChar(GetOpCode());
    buf->EmitChar(capture);
    return GetDynChunkfSize(*buf);
}

uint32_t SaveEndOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "save_end\t" << buf.GetU8(offset + 1) << std::endl;
    return offset + GetSize();
}

uint32_t CharOpCode::EmitOpCode(DynChunk *buf, uint32_t para) const
{
    auto para_char = static_cast<uint16_t>(para & 0xffffU);  // NOLINT(readability-magic-numbers)
    buf->EmitChar(GetOpCode());
    buf->EmitU16(para_char);
    return GetDynChunkfSize(*buf);
}

uint32_t CharOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "char\t" << static_cast<char>(buf.GetU16(offset + 1)) << std::endl;
    return offset + GetSize();
}

uint32_t Char32OpCode::EmitOpCode(DynChunk *buf, uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    buf->EmitU32(para);
    return GetDynChunkfSize(*buf);
}

uint32_t Char32OpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "char32\t" << static_cast<char>(buf.GetU32(offset + 1)) << std::endl;
    return offset + GetSize();
}

uint32_t GotoOpCode::EmitOpCode(DynChunk *buf, uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    buf->EmitU32(para);
    return GetDynChunkfSize(*buf);
}

void GotoOpCode::UpdateOpPara(DynChunk *buf, uint32_t offset, uint32_t para) const
{
    buf->PutU32(offset + 1, para);
}

uint32_t GotoOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "goto\t" << buf.GetU32(offset + 1) + offset + GetSize() << std::endl;
    return offset + GetSize();
}

uint32_t SplitNextOpCode::InsertOpCode(DynChunk *buf, uint32_t offset, uint32_t para) const
{
    buf->Insert(offset, GetSize());
    buf->PutU8(offset, GetOpCode());
    buf->PutU32(offset + 1, para);
    return GetDynChunkfSize(*buf);
}

uint32_t SplitNextOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "split_next\t" << buf.GetU32(offset + 1) + offset + GetSize() << std::endl;
    return offset + GetSize();
}

uint32_t SplitFirstOpCode::InsertOpCode(DynChunk *buf, uint32_t offset, uint32_t para) const
{
    buf->Insert(offset, GetSize());
    buf->PutU8(offset, GetOpCode());
    buf->PutU32(offset + 1, para);
    return GetDynChunkfSize(*buf);
}

uint32_t SplitFirstOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "split_first\t" << buf.GetU32(offset + 1) + offset + GetSize() << std::endl;
    return offset + GetSize();
}

uint32_t LoopOpCode::EmitOpCode(DynChunk *buf, uint32_t start, uint32_t min, uint32_t max) const
{
    buf->EmitChar(GetOpCode());
    buf->EmitU32(start);
    buf->EmitU32(min);
    buf->EmitU32(max);
    return GetDynChunkfSize(*buf);
}

uint32_t LoopOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "loop\t" << buf.GetU32(offset + 1) + offset + GetSize() << "\t"
        << buf.GetU32(offset + RegExpOpCode::OP_SIZE_FIVE) << "\t" << buf.GetU32(offset + RegExpOpCode::OP_SIZE_NINE)
        << std::endl;
    return offset + GetSize();
}

uint32_t LoopGreedyOpCode::EmitOpCode(DynChunk *buf, uint32_t start, uint32_t min, uint32_t max) const
{
    buf->EmitChar(GetOpCode());
    buf->EmitU32(start);
    buf->EmitU32(min);
    buf->EmitU32(max);
    return GetDynChunkfSize(*buf);
}

uint32_t LoopGreedyOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "greedy_loop\t" << buf.GetU32(offset + 1) + offset + GetSize() << "\t"
        << buf.GetU32(offset + RegExpOpCode::OP_SIZE_FIVE) << "\t" << buf.GetU32(offset + RegExpOpCode::OP_SIZE_NINE)
        << std::endl;
    return offset + GetSize();
}

uint32_t PushCharOpCode::InsertOpCode(DynChunk *buf, uint32_t offset) const
{
    buf->Insert(offset, GetSize());
    buf->PutU8(offset, GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t PushCharOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "push_char" << std::endl;
    return offset + GetSize();
}

uint32_t PushOpCode::InsertOpCode(DynChunk *buf, uint32_t offset) const
{
    buf->Insert(offset, GetSize());
    buf->PutU8(offset, GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t PushOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "push" << std::endl;
    return offset + GetSize();
}

uint32_t PopOpCode::EmitOpCode(DynChunk *buf) const
{
    buf->EmitChar(GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t PopOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "pop" << std::endl;
    return offset + GetSize();
}

uint32_t CheckCharOpCode::EmitOpCode(DynChunk *buf, uint32_t offset) const
{
    buf->EmitChar(GetOpCode());
    buf->EmitU32(offset);
    return GetDynChunkfSize(*buf);
}

uint32_t CheckCharOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "check_char\t" << buf.GetU32(offset + 1) + offset + GetSize() << std::endl;
    return offset + GetSize();
}

uint32_t SaveResetOpCode::InsertOpCode(DynChunk *buf, uint32_t offset, uint32_t start, uint32_t end) const
{
    auto capture_start = static_cast<uint8_t>(start & 0xffU);  // NOLINT(readability-magic-numbers)
    auto capture_end = static_cast<uint8_t>(end & 0xffU);      // NOLINT(readability-magic-numbers)
    buf->Insert(offset, GetSize());
    buf->PutU8(offset, GetOpCode());
    buf->PutU8(offset + RegExpOpCode::OP_SIZE_ONE, capture_start);
    buf->PutU8(offset + RegExpOpCode::OP_SIZE_TWO, capture_end);
    return GetDynChunkfSize(*buf);
}

uint32_t SaveResetOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "save_reset\t" << buf.GetU8(offset + RegExpOpCode::OP_SIZE_ONE) << "\t"
        << buf.GetU8(offset + RegExpOpCode::OP_SIZE_TWO) << std::endl;
    return offset + GetSize();
}

uint32_t MatchOpCode::EmitOpCode(DynChunk *buf, [[maybe_unused]] uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t MatchOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "match" << std::endl;
    return offset + GetSize();
}

uint32_t MatchEndOpCode::EmitOpCode(DynChunk *buf, [[maybe_unused]] uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t MatchEndOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "match_end" << std::endl;
    return offset + GetSize();
}

uint32_t LineStartOpCode::EmitOpCode(DynChunk *buf, [[maybe_unused]] uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t LineStartOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "line_start" << std::endl;
    return offset + GetSize();
}

uint32_t LineEndOpCode::EmitOpCode(DynChunk *buf, [[maybe_unused]] uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t LineEndOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "line_end" << std::endl;
    return offset + GetSize();
}

uint32_t WordBoundaryOpCode::EmitOpCode(DynChunk *buf, [[maybe_unused]] uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t WordBoundaryOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "word_boundary" << std::endl;
    return offset + GetSize();
}

uint32_t NotWordBoundaryOpCode::EmitOpCode(DynChunk *buf, [[maybe_unused]] uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t NotWordBoundaryOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf,
                                           uint32_t offset) const
{
    out << offset << ":\t"
        << "not_word_boundary" << std::endl;
    return offset + GetSize();
}

uint32_t AllOpCode::EmitOpCode(DynChunk *buf, [[maybe_unused]] uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t AllOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "all" << std::endl;
    return offset + GetSize();
}

uint32_t DotsOpCode::EmitOpCode(DynChunk *buf, [[maybe_unused]] uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t DotsOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "dots" << std::endl;
    return offset + GetSize();
}

uint32_t MatchAheadOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "match_ahead\t" << buf.GetU32(offset + 1) + offset + GetSize() << std::endl;
    return offset + GetSize();
}

uint32_t RangeOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "range\t";
    size_t size = buf.GetU16(offset + 1);
    for (size_t i = 0; i < size; i++) {
        out << buf.GetU16(offset + RegExpOpCode::OP_SIZE_THREE + (i * RegExpOpCode::OP_SIZE_FOUR)) << "\t"
            << buf.GetU16(offset + RegExpOpCode::OP_SIZE_THREE +
                          (i * RegExpOpCode::OP_SIZE_FOUR + RegExpOpCode::OP_SIZE_TWO))
            << "\t";
    }
    out << std::endl;
    return offset + size * RegExpOpCode::OP_SIZE_FOUR + RegExpOpCode::OP_SIZE_THREE;
}

uint32_t RangeOpCode::InsertOpCode(DynChunk *buf, const RangeSet &range_set) const
{
    buf->EmitChar(GetOpCode());
    size_t size = range_set.range_set_.size();
    buf->EmitU16(size);
    for (auto range : range_set.range_set_) {
        buf->EmitU16(range.first);
        buf->EmitU16(range.second);
    }
    return GetDynChunkfSize(*buf);
}

uint32_t Range32OpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "range32\t";
    size_t size = buf.GetU16(offset + 1);
    for (size_t i = 0; i < size; i++) {
        out << buf.GetU32(offset + RegExpOpCode::OP_SIZE_THREE + (i * RegExpOpCode::OP_SIZE_EIGHT)) << "\t"
            << buf.GetU32(offset + RegExpOpCode::OP_SIZE_THREE +
                          (i * RegExpOpCode::OP_SIZE_EIGHT + RegExpOpCode::OP_SIZE_FOUR))
            << "\t";
    }
    out << std::endl;
    return offset + size * +RegExpOpCode::OP_SIZE_EIGHT + RegExpOpCode::OP_SIZE_THREE;
}

uint32_t Range32OpCode::InsertOpCode(DynChunk *buf, const RangeSet &range_set) const
{
    buf->EmitChar(GetOpCode());
    size_t size = range_set.range_set_.size();
    buf->EmitU16(size);
    for (auto range : range_set.range_set_) {
        buf->EmitU32(range.first);
        buf->EmitU32(range.second);
    }
    return GetDynChunkfSize(*buf);
}

uint32_t MatchAheadOpCode::InsertOpCode(DynChunk *buf, uint32_t offset, uint32_t para) const
{
    buf->Insert(offset, GetSize());
    buf->PutU8(offset, GetOpCode());
    buf->PutU32(offset + 1, para);
    return GetDynChunkfSize(*buf);
}

uint32_t NegativeMatchAheadOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "negative_match_ahead\t" << buf.GetU32(offset + 1) + offset + GetSize() << std::endl;
    return offset + GetSize();
}

uint32_t NegativeMatchAheadOpCode::InsertOpCode(DynChunk *buf, uint32_t offset, uint32_t para) const
{
    buf->Insert(offset, GetSize());
    buf->PutU8(offset, GetOpCode());
    buf->PutU32(offset + 1, para);
    return GetDynChunkfSize(*buf);
}

uint32_t PrevOpCode::EmitOpCode(DynChunk *buf, [[maybe_unused]] uint32_t para) const
{
    buf->EmitChar(GetOpCode());
    return GetDynChunkfSize(*buf);
}

uint32_t PrevOpCode::DumpOpCode(std::ostream &out, [[maybe_unused]] const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "prev" << std::endl;
    return offset + GetSize();
}

uint32_t BackReferenceOpCode::EmitOpCode(DynChunk *buf, uint32_t para) const
{
    auto capture = static_cast<uint8_t>(para & 0xffU);  // NOLINT(readability-magic-numbers)
    buf->EmitChar(GetOpCode());
    buf->EmitChar(capture);
    return GetDynChunkfSize(*buf);
}

uint32_t BackReferenceOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "backreference\t" << buf.GetU8(offset + 1) << std::endl;
    return offset + GetSize();
}

uint32_t BackwardBackReferenceOpCode::EmitOpCode(DynChunk *buf, uint32_t para) const
{
    auto capture = static_cast<uint8_t>(para & 0xffU);  // NOLINT(readability-magic-numbers)
    buf->EmitChar(GetOpCode());
    buf->EmitChar(capture);
    return GetDynChunkfSize(*buf);
}

uint32_t BackwardBackReferenceOpCode::DumpOpCode(std::ostream &out, const DynChunk &buf, uint32_t offset) const
{
    out << offset << ":\t"
        << "backward_backreference\t" << buf.GetU8(offset + 1) << std::endl;
    return offset + GetSize();
}

void RangeSet::Insert(uint32_t start, uint32_t end)
{
    if (start > end) {
        return;
    }
    std::pair<uint32_t, uint32_t> pair_element = std::make_pair(start, end);
    if (range_set_.empty()) {
        range_set_.emplace_back(pair_element);
    } else {
        for (auto iter = range_set_.begin(); iter != range_set_.end(); iter++) {
            if (IsIntersect(start, end, iter->first, iter->second) ||
                IsAdjacent(start, end, iter->first, iter->second)) {
                iter->first = std::min(iter->first, start);
                iter->second = std::max(iter->second, end);
                return;
            }
            if (iter->first > end) {
                range_set_.insert(iter, pair_element);
                return;
            }
        }
        range_set_.emplace_back(pair_element);
    }
}

void RangeSet::Insert(const RangeSet &s1)
{
    if (s1.range_set_.empty()) {
        return;
    }
    if (range_set_.empty()) {
        range_set_ = s1.range_set_;
    } else {
        for (auto range : s1.range_set_) {
            Insert(range.first, range.second);
        }
        Compress();
    }
}

void RangeSet::Invert(bool is_utf16)
{
    uint32_t max_value = is_utf16 ? UINT32_MAX : UINT16_MAX;
    if (range_set_.empty()) {
        range_set_.emplace_back(std::make_pair(0, max_value));
        return;
    }

    auto iter = range_set_.begin();
    auto iter2 = range_set_.begin();
    if (iter->first == 0 && iter->second == max_value) {
        range_set_.clear();
        return;
    }
    iter2++;

    uint32_t first = iter->first;

    for (iter = range_set_.begin(); iter != range_set_.end(); iter++) {
        if (iter->second == max_value) {
            range_set_.erase(iter);
            break;
        }
        iter->first = iter->second + 1;
        if (iter2 != range_set_.end()) {
            iter->second = iter2->first - 1;
            iter2++;
        } else {
            iter->second = max_value;
        }
    }
    if (first > 0) {
        std::pair<uint32_t, uint32_t> pair1 = std::make_pair(0, first - 1);
        range_set_.push_front(pair1);
    }
    Compress();
}

void RangeSet::Compress()
{
    auto iter = range_set_.begin();
    auto iter2 = range_set_.begin();
    iter2++;
    while (iter2 != range_set_.end()) {
        if (IsIntersect(iter->first, iter->second, iter2->first, iter2->second) ||
            IsAdjacent(iter->first, iter->second, iter2->first, iter2->second)) {
            iter->first = std::min(iter->first, iter2->first);
            iter->second = std::max(iter->second, iter2->second);
            iter2 = range_set_.erase(iter2);
        } else {
            iter++;
            iter2++;
        }
    }
}
}  // namespace panda
