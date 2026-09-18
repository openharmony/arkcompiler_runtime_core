/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef VERIFIER_VERIFIER_INTERNAL_H
#define VERIFIER_VERIFIER_INTERNAL_H

#include "helpers.h"
#include "verifier.h"

#include <array>
#include <climits>
#include <optional>

namespace panda::verifier {

// Register usage and control flow of one instruction.
struct Verifier::InsnRegInfo {
    std::vector<uint16_t> read_regs {};
    std::optional<uint16_t> write_reg {};
    bool define_all_regs {false};
    bool has_fallthrough {true};
    std::optional<uint32_t> jump_target_index {};
    uint32_t offset {0};
};

// May-defined register dataflow; states merge with word-sized OR.
class Verifier::RegisterDefinitionDataflow {
public:
    class RegBitset {
    public:
        explicit RegBitset(uint64_t num_regs, bool set_all = false)
            : words_((num_regs + WORD_BITS - 1U) / WORD_BITS, set_all ? ~0ULL : 0ULL)
        {
        }

        void Set(uint64_t reg)
        {
            words_[reg / WORD_BITS] |= 1ULL << (reg % WORD_BITS);
        }

        bool Test(uint64_t reg) const
        {
            return (words_[reg / WORD_BITS] & (1ULL << (reg % WORD_BITS))) != 0;
        }

        void SetRange(uint64_t begin, uint64_t end)
        {
            uint64_t reg = begin;
            while (reg < end) {
                if (reg % WORD_BITS == 0U && reg + WORD_BITS <= end) {
                    words_[reg / WORD_BITS] = ~0ULL;
                    reg += WORD_BITS;
                } else {
                    Set(reg);
                    reg++;
                }
            }
        }

        bool OrWith(const RegBitset &state)
        {
            bool changed = false;
            for (size_t i = 0; i < words_.size(); i++) {
                uint64_t merged = words_[i] | state.words_[i];
                if (merged != words_[i]) {
                    words_[i] = merged;
                    changed = true;
                }
            }
            return changed;
        }

    private:
        static constexpr size_t WORD_BITS = sizeof(uint64_t) * CHAR_BIT;
        std::vector<uint64_t> words_;
    };

    RegisterDefinitionDataflow(size_t insn_num, uint64_t num_regs)
        : num_regs_(num_regs),
          defined_states_(insn_num, RegBitset(num_regs)),
          reachable_(insn_num, false),
          in_worklist_(insn_num, false)
    {
    }

    // Seed args and never-written vregs as defined at method entry.
    void SeedEntry(uint64_t num_vregs, uint64_t num_regs, const std::vector<bool> &written_anywhere)
    {
        RegBitset entry_defined(num_regs);
        entry_defined.SetRange(num_vregs, num_regs);
        for (uint64_t reg = 0; reg < num_vregs; reg++) {
            if (!written_anywhere[reg]) {
                entry_defined.Set(reg);
            }
        }
        Seed(0, entry_defined);
    }

    // Catch handlers conservatively see all registers defined.
    void SeedAllDefined(uint32_t index)
    {
        Seed(index, RegBitset(num_regs_, true));
    }

    void Run(const std::vector<InsnRegInfo> &insn_infos, uint64_t num_vregs)
    {
        while (!worklist_.empty()) {
            uint32_t index = worklist_.back();
            worklist_.pop_back();
            in_worklist_[index] = false;
            RegBitset out_state = defined_states_[index];
            const auto &info = insn_infos[index];
            if (info.define_all_regs) {
                out_state.SetRange(0, num_vregs);
            }
            if (info.write_reg.has_value()) {
                out_state.Set(info.write_reg.value());
            }
            if (info.has_fallthrough && index + 1U < defined_states_.size()) {
                Seed(index + 1U, out_state);
            }
            if (info.jump_target_index.has_value()) {
                Seed(info.jump_target_index.value(), out_state);
            }
        }
    }

    bool IsReachable(size_t index) const
    {
        return reachable_[index];
    }

    bool IsDefined(size_t index, uint64_t reg) const
    {
        return defined_states_[index].Test(reg);
    }

private:
    // Merge state; queue on first visit or when bits change.
    void Seed(uint32_t index, const RegBitset &state)
    {
        if (index >= defined_states_.size()) {
            return;
        }
        bool bits_changed = defined_states_[index].OrWith(state);
        bool first_visit = !reachable_[index];
        if (!bits_changed && !first_visit) {
            return;
        }
        reachable_[index] = true;
        if (!in_worklist_[index]) {
            in_worklist_[index] = true;
            worklist_.push_back(index);
        }
    }

    uint64_t num_regs_;
    std::vector<RegBitset> defined_states_;
    std::vector<bool> reachable_;
    std::vector<bool> in_worklist_;
    std::vector<uint32_t> worklist_;
};

// Sequential ic slot assignment matching the es2panda frontend.
class Verifier::IcSlotAssignment {
public:
    explicit IcSlotAssignment(const std::vector<IcSlotInsnInfo> &ic_insns) : ic_insns_(ic_insns) {}

    void RunInitial(std::vector<uint32_t> &expected)
    {
        expected.assign(ic_insns_.size(), 0);
        for (size_t i = 0; i < ic_insns_.size(); i++) {
            Assign(i, expected);
        }
    }

    // Two-pass overflow handling: 8-bit-only first, then 16-bit capable.
    void RunRearranged(std::vector<uint32_t> &expected)
    {
        expected.assign(ic_insns_.size(), 0);
        counter_ = 0;
        for (size_t i = 0; i < ic_insns_.size(); i++) {
            if (ic_insns_[i].is_8bit_only) {
                Assign(i, expected);
            }
        }
        for (size_t i = 0; i < ic_insns_.size(); i++) {
            if (!ic_insns_[i].is_8bit_only) {
                Assign(i, expected);
            }
        }
    }

    uint32_t GetTotal() const
    {
        return counter_;
    }

private:
    // Assign one insn; write INVALID_IC_SLOT on overflow.
    void Assign(size_t i, std::vector<uint32_t> &expected)
    {
        const IcSlotInsnInfo &insn = ic_insns_[i];
        uint32_t ret = insn.is_two_slot ? 2U : 1U;
        if (counter_ <= INVALID_IC_SLOT && counter_ + ret > INVALID_IC_SLOT) {
            expected[i] = insn.is_8bit_only ? INVALID_IC_SLOT : IC_SLOT_16BIT_BASE;
            counter_ = insn.is_8bit_only ? IC_SLOT_16BIT_BASE : IC_SLOT_16BIT_BASE + ret;
            return;
        }
        if ((insn.is_8bit_only && counter_ > INVALID_IC_SLOT) || counter_ > IC_SLOT_MAX_16BIT) {
            expected[i] = INVALID_IC_SLOT;
            return;
        }
        expected[i] = counter_;
        counter_ += ret;
    }

    const std::vector<IcSlotInsnInfo> &ic_insns_;
    uint32_t counter_ {0};
};

// Tag kind used by the literal walk. Must match EnumerateLiteralVals.
enum class LiteralItemKind : uint8_t {
    REJECT = 0,
    SCALAR,
    STRING_ID,
    METHOD_ID,
    LITERAL_ID,
    ARRAY,
};

struct LiteralTagDesc {
    uint8_t value_width;
    uint8_t array_elem_size;
    LiteralItemKind kind;
};

constexpr uint8_t LITERAL_W8 = sizeof(uint8_t);
constexpr uint8_t LITERAL_W16 = sizeof(uint16_t);
constexpr uint8_t LITERAL_W32 = sizeof(uint32_t);
constexpr uint8_t LITERAL_W64 = sizeof(uint64_t);

constexpr LiteralTagDesc kRejectDesc {0, 0, LiteralItemKind::REJECT};
constexpr LiteralTagDesc kScalar8Desc {LITERAL_W8, 0, LiteralItemKind::SCALAR};
constexpr LiteralTagDesc kScalar16Desc {LITERAL_W16, 0, LiteralItemKind::SCALAR};
constexpr LiteralTagDesc kScalar32Desc {LITERAL_W32, 0, LiteralItemKind::SCALAR};
constexpr LiteralTagDesc kScalar64Desc {LITERAL_W64, 0, LiteralItemKind::SCALAR};
constexpr LiteralTagDesc kStringIdDesc {LITERAL_W32, 0, LiteralItemKind::STRING_ID};
constexpr LiteralTagDesc kMethodIdDesc {LITERAL_W32, 0, LiteralItemKind::METHOD_ID};
constexpr LiteralTagDesc kLiteralIdDesc {LITERAL_W32, 0, LiteralItemKind::LITERAL_ID};
constexpr LiteralTagDesc kArray1Desc {0, 1, LiteralItemKind::ARRAY};
constexpr LiteralTagDesc kArray2Desc {0, LITERAL_W16, LiteralItemKind::ARRAY};
constexpr LiteralTagDesc kArray4Desc {0, LITERAL_W32, LiteralItemKind::ARRAY};
constexpr LiteralTagDesc kArray8Desc {0, LITERAL_W64, LiteralItemKind::ARRAY};

// Indexed by LiteralTag. NULLVALUE (0xff) is handled separately.
constexpr std::array<LiteralTagDesc, 0x1d> kLiteralTagTable {{
    kRejectDesc,     // TAGVALUE
    kScalar8Desc,    // BOOL
    kScalar32Desc,   // INTEGER
    kScalar32Desc,   // FLOAT
    kScalar64Desc,   // DOUBLE
    kStringIdDesc,   // STRING
    kMethodIdDesc,   // METHOD
    kMethodIdDesc,   // GENERATORMETHOD
    kScalar8Desc,    // ACCESSOR
    kScalar16Desc,   // METHODAFFILIATE
    kArray1Desc,     // ARRAY_U1
    kArray1Desc,     // ARRAY_U8
    kArray1Desc,     // ARRAY_I8
    kArray2Desc,     // ARRAY_U16
    kArray2Desc,     // ARRAY_I16
    kArray4Desc,     // ARRAY_U32
    kArray4Desc,     // ARRAY_I32
    kArray8Desc,     // ARRAY_U64
    kArray8Desc,     // ARRAY_I64
    kArray4Desc,     // ARRAY_F32
    kArray8Desc,     // ARRAY_F64
    kArray4Desc,     // ARRAY_STRING
    kMethodIdDesc,   // ASYNCGENERATORMETHOD
    kScalar32Desc,   // LITERALBUFFERINDEX
    kLiteralIdDesc,  // LITERALARRAY
    kScalar8Desc,    // BUILTINTYPEINDEX
    kMethodIdDesc,   // GETTER
    kMethodIdDesc,   // SETTER
    kScalar32Desc,   // ETS_IMPLEMENTS
}};

constexpr LiteralTagDesc GetLiteralTagDesc(panda_file::LiteralTag tag)
{
    auto idx = static_cast<uint8_t>(tag);
    if (idx == static_cast<uint8_t>(panda_file::LiteralTag::NULLVALUE)) {
        return kScalar8Desc;
    }
    if (idx >= kLiteralTagTable.size()) {
        return kRejectDesc;
    }
    return kLiteralTagTable[idx];
}

constexpr LiteralTagDesc GetArrayLiteralTagDesc(panda_file::LiteralTag tag)
{
    auto desc = GetLiteralTagDesc(tag);
    if (desc.kind != LiteralItemKind::ARRAY) {
        return kRejectDesc;
    }
    return desc;
}

inline constexpr size_t LiteralValueWidth(panda_file::LiteralTag tag)
{
    return GetLiteralTagDesc(tag).value_width;
}

inline constexpr size_t ArrayElementSize(panda_file::LiteralTag tag)
{
    return GetLiteralTagDesc(tag).array_elem_size;
}

inline constexpr bool IsArrayLiteralTag(panda_file::LiteralTag tag)
{
    return GetLiteralTagDesc(tag).kind == LiteralItemKind::ARRAY;
}

// Prefix opcode and last valid secondary opcode; keep in sync with isa.yaml.
struct PrefixOpcodeBound {
    uint8_t prefix;
    uint8_t last_secondary;
};

inline constexpr std::array<PrefixOpcodeBound, 4> PREFIX_OPCODE_BOUNDS {{
    {0xfb, 0x1b},  // callruntime
    {0xfc, 0x2e},  // deprecated
    {0xfd, 0x14},  // wide
    {0xfe, 0x09},  // throw
}};

template <size_t N>
inline constexpr bool OpcodeIsAnyOf(Opcode op, const std::array<Opcode, N> &ops)
{
    for (const auto &candidate : ops) {
        if (op == candidate) {
            return true;
        }
    }
    return false;
}

inline constexpr std::array<Opcode, 19> kModuleOpcodes {{
    Opcode::GETMODULENAMESPACE_IMM8,
    Opcode::WIDE_GETMODULENAMESPACE_PREF_IMM16,
    Opcode::DEPRECATED_GETMODULENAMESPACE_PREF_ID32,
    Opcode::STMODULEVAR_IMM8,
    Opcode::WIDE_STMODULEVAR_PREF_IMM16,
    Opcode::DEPRECATED_STMODULEVAR_PREF_ID32,
    Opcode::LDLOCALMODULEVAR_IMM8,
    Opcode::WIDE_LDLOCALMODULEVAR_PREF_IMM16,
    Opcode::LDEXTERNALMODULEVAR_IMM8,
    Opcode::WIDE_LDEXTERNALMODULEVAR_PREF_IMM16,
    Opcode::DEPRECATED_LDMODULEVAR_PREF_ID32_IMM8,
    Opcode::CALLRUNTIME_LDSENDABLEEXTERNALMODULEVAR_PREF_IMM8,
    Opcode::CALLRUNTIME_WIDELDSENDABLEEXTERNALMODULEVAR_PREF_IMM16,
    Opcode::CALLRUNTIME_LDSENDABLELOCALMODULEVAR_PREF_IMM8,
    Opcode::CALLRUNTIME_WIDELDSENDABLELOCALMODULEVAR_PREF_IMM16,
    Opcode::CALLRUNTIME_LDLAZYMODULEVAR_PREF_IMM8,
    Opcode::CALLRUNTIME_WIDELDLAZYMODULEVAR_PREF_IMM16,
    Opcode::CALLRUNTIME_LDLAZYSENDABLEMODULEVAR_PREF_IMM8,
    Opcode::CALLRUNTIME_WIDELDLAZYSENDABLEMODULEVAR_PREF_IMM16,
}};

inline constexpr std::array<Opcode, 4> kRegisterWriteOpcodes {{
    Opcode::STA_V8,
    Opcode::MOV_V4_V4,
    Opcode::MOV_V8_V8,
    Opcode::MOV_V16_V16,
}};

struct ModuleEntrySection {
    uint32_t string_id_count;
    bool has_module_request_idx;
};

constexpr size_t MODULE_ENTRY_SECTION_NUM = 5;

// Max uleb128 length of uint32 (ceil(32 / 7)).
constexpr size_t MAX_ULEB128_U32_SIZE = 5;

// Little-endian read; nullopt if remaining span is too short.
template <size_t WIDTH>
uint64_t ReadFixedWidth(Span<const uint8_t> *sp)
{
    return static_cast<uint64_t>(panda_file::helpers::Read<WIDTH>(sp));
}

inline std::optional<uint64_t> ReadU8OrU16(Span<const uint8_t> *sp, size_t width)
{
    if (width == sizeof(uint8_t)) {
        return ReadFixedWidth<sizeof(uint8_t)>(sp);
    }
    if (width == sizeof(uint16_t)) {
        return ReadFixedWidth<sizeof(uint16_t)>(sp);
    }
    return std::nullopt;
}

inline std::optional<uint64_t> ReadU32OrU64(Span<const uint8_t> *sp, size_t width)
{
    if (width == sizeof(uint32_t)) {
        return ReadFixedWidth<sizeof(uint32_t)>(sp);
    }
    if (width == sizeof(uint64_t)) {
        return ReadFixedWidth<sizeof(uint64_t)>(sp);
    }
    return std::nullopt;
}

inline std::optional<uint64_t> ReadBoundedByWidth(Span<const uint8_t> *sp, size_t width)
{
    auto small = ReadU8OrU16(sp, width);
    if (small.has_value()) {
        return small;
    }
    return ReadU32OrU64(sp, width);
}

inline std::optional<uint64_t> ReadBounded(Span<const uint8_t> *sp, size_t width)
{
    if (sp == nullptr || sp->Size() < width) {
        return std::nullopt;
    }
    return ReadBoundedByWidth(sp, width);
}

}  // namespace panda::verifier

#endif  // VERIFIER_VERIFIER_INTERNAL_H
