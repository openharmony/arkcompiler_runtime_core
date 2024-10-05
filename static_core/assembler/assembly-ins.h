/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_INS_H
#define PANDA_ASSEMBLER_ASSEMBLY_INS_H

#include <array>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "assembly-debug.h"
#include "bytecode_emitter.h"
#include "file_items.h"
#include "isa.h"
#include "lexer.h"

namespace ark::pandasm {

enum class Opcode {
// CC-OFFNXT(G.PRE.02) opcode is class member
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) opcode,
    PANDA_INSTRUCTION_LIST(OPLIST)
#undef OPLIST
        INVALID,
    NUM_OPCODES = INVALID
};

enum InstFlags {
    NONE = 0,
    JUMP = (1U << 0U),
    COND = (1U << 1U),
    CALL = (1U << 2U),
    RETURN = (1U << 3U),
    ACC_READ = (1U << 4U),
    ACC_WRITE = (1U << 5U),
    PSEUDO = (1U << 6U),
    THROWING = (1U << 7U),
    METHOD_ID = (1U << 8U),
    FIELD_ID = (1U << 9U),
    TYPE_ID = (1U << 10U),
    STRING_ID = (1U << 11U),
    LITERALARRAY_ID = (1U << 12U),
    CALL_RANGE = (1U << 13U)
};

constexpr int INVALID_REG_IDX = -1;

constexpr InstFlags operator|(InstFlags a, InstFlags b)
{
    using Utype = std::underlying_type_t<InstFlags>;
    return static_cast<InstFlags>(static_cast<Utype>(a) | static_cast<Utype>(b));
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) (flags),
constexpr std::array<unsigned, static_cast<size_t>(Opcode::NUM_OPCODES)> INST_FLAGS_TABLE = {
    PANDA_INSTRUCTION_LIST(OPLIST)};
#undef OPLIST

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) (width),
constexpr std::array<size_t, static_cast<size_t>(Opcode::NUM_OPCODES)> INST_WIDTH_TABLE = {
    PANDA_INSTRUCTION_LIST(OPLIST)};
#undef OPLIST

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) (def_idx),
constexpr std::array<int, static_cast<size_t>(Opcode::NUM_OPCODES)> DEF_IDX_TABLE = {PANDA_INSTRUCTION_LIST(OPLIST)};
#undef OPLIST

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) (use_idxs),
constexpr std::array<std::array<int, MAX_NUMBER_OF_SRC_REGS>, static_cast<size_t>(Opcode::NUM_OPCODES)> USE_IDXS_TABLE =
    {PANDA_INSTRUCTION_LIST(OPLIST)};  // CC-OFF(G.FMT.03) any style changes will worsen readability
#undef OPLIST

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(opcode, name, optype, width, flags, def_idx, use_idxs, prof_size) (prof_size),
// clang-format off
constexpr std::array<unsigned, static_cast<size_t>(Opcode::NUM_OPCODES) + 1> INST_PROFILE_SIZES =
    {PANDA_INSTRUCTION_LIST(OPLIST) 0};  // CC-OFF(G.FMT.03) any style changes will worsen readability
// clang-format on
#undef OPLIST

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Ins {
    using IType = std::variant<int64_t, double>;

    constexpr static uint16_t ACCUMULATOR = -1;
    constexpr static size_t MAX_CALL_SHORT_ARGS = 2;
    constexpr static size_t MAX_CALL_ARGS = 4;
    constexpr static uint16_t MAX_NON_RANGE_CALL_REG = 15;
    constexpr static uint16_t MAX_RANGE_CALL_START_REG = 255;

    Opcode opcode = Opcode::INVALID; /* operation type */
    std::vector<uint16_t> regs;      /* list of arguments - registers */
    std::vector<std::string> ids;    /* list of arguments - identifiers */
    std::vector<IType> imms;         /* list of arguments - immediates */
    std::string label;               /* label at the beginning of a line */
    bool setLabel = false;           /* whether this label is defined */
    debuginfo::Ins insDebug;
    uint16_t profileId {0}; /* Index in the profile vector */

    PANDA_PUBLIC_API std::string ToString(const std::string &endline = "", bool printArgs = false,
                                          size_t firstArgIdx = 0) const;

    // CC-OFFNXT(G.FUN.01-CPP) solid logic
    bool Emit(BytecodeEmitter &emitter, panda_file::MethodItem *method,
              const std::unordered_map<std::string, panda_file::BaseMethodItem *> &methods,
              const std::unordered_map<std::string, panda_file::BaseFieldItem *> &fields,
              const std::unordered_map<std::string, panda_file::BaseClassItem *> &classes,
              const std::unordered_map<std::string_view, panda_file::StringItem *> &strings,
              const std::unordered_map<std::string, panda_file::LiteralArrayItem *> &literalarrays,
              const std::unordered_map<std::string_view, ark::Label> &labels) const;

    size_t OperandListLength() const
    {
        return regs.size() + ids.size() + imms.size();
    }

    bool HasFlag(InstFlags flag) const
    {
        if (opcode == Opcode::INVALID) {  // NOTE(mbolshov): introduce 'label' opcode for labels
            return false;
        }
        return (INST_FLAGS_TABLE[static_cast<size_t>(opcode)] & flag) != 0;
    }

    bool CanThrow() const
    {
        return HasFlag(InstFlags::THROWING) || HasFlag(InstFlags::METHOD_ID) || HasFlag(InstFlags::FIELD_ID) ||
               HasFlag(InstFlags::TYPE_ID) || HasFlag(InstFlags::STRING_ID);
    }

    bool IsJump() const
    {
        return HasFlag(InstFlags::JUMP);
    }

    bool IsConditionalJump() const
    {
        return IsJump() && HasFlag(InstFlags::COND);
    }

    bool IsCall() const
    {  // Non-range call
        return HasFlag(InstFlags::CALL);
    }

    bool IsCallRange() const
    {  // Range call
        return HasFlag(InstFlags::CALL_RANGE);
    }

    bool IsPseudoCall() const
    {
        return HasFlag(InstFlags::PSEUDO) && HasFlag(InstFlags::CALL);
    }

    bool IsReturn() const
    {
        return HasFlag(InstFlags::RETURN);
    }

    size_t MaxRegEncodingWidth() const
    {
        if (opcode == Opcode::INVALID) {
            return 0;
        }
        return INST_WIDTH_TABLE[static_cast<size_t>(opcode)];
    }

    std::vector<uint16_t> Uses() const
    {
        if (IsPseudoCall()) {
            return regs;
        }

        if (opcode == Opcode::INVALID) {
            return {};
        }

        auto useIdxs = USE_IDXS_TABLE[static_cast<size_t>(opcode)];
        std::vector<uint16_t> res(MAX_NUMBER_OF_SRC_REGS + 1);
        if (HasFlag(InstFlags::ACC_READ)) {
            res.push_back(Ins::ACCUMULATOR);
        }
        for (auto idx : useIdxs) {
            if (idx == INVALID_REG_IDX) {
                break;
            }
            ASSERT(static_cast<size_t>(idx) < regs.size());
            res.emplace_back(regs[idx]);
        }
        return res;
    }

    std::optional<size_t> Def() const
    {
        if (opcode == Opcode::INVALID) {
            return {};
        }
        auto defIdx = DEF_IDX_TABLE[static_cast<size_t>(opcode)];
        if (defIdx != INVALID_REG_IDX) {
            return regs[defIdx];
        }
        if (HasFlag(InstFlags::ACC_WRITE)) {
            return Ins::ACCUMULATOR;
        }
        return {};
    }

    bool IsValidToEmit() const
    {
        const auto invalidRegNum = 1U << MaxRegEncodingWidth();
        for (auto reg : regs) {
            if (reg >= invalidRegNum) {
                return false;
            }
        }
        return true;
    }

    bool HasDebugInfo() const
    {
        return insDebug.lineNumber != 0;
    }

private:
    std::string OperandsToString(bool printArgs = false, size_t firstArgIdx = 0) const;
    std::string RegsToString(bool &first, bool printArgs = false, size_t firstArgIdx = 0) const;
    std::string ImmsToString(bool &first) const;
    std::string IdsToString(bool &first) const;

    std::string IdToString(size_t idx, bool isFirst) const;
    std::string ImmToString(size_t idx, bool isFirst) const;
    std::string RegToString(size_t idx, bool isFirst, bool printArgs = false, size_t firstArgIdx = 0) const;
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_INS_H
