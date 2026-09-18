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

#include "verifier.h"
#include "utils.h"

#include <gtest/gtest.h>
#include <algorithm>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include "bytecode_instruction-inl.h"
#include "class_data_accessor-inl.h"
#include "code_data_accessor-inl.h"
#include "file.h"
#include "utils/logger.h"

using namespace testing::ext;

namespace panda::verifier {
class VerifierNewChecksTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    // Reads the whole file into the buffer.
    static bool ReadAbcFile(const std::string &file_name, std::vector<unsigned char> &buffer)
    {
        std::ifstream base_file(file_name, std::ios::binary);
        if (!base_file.is_open()) {
            LOG(ERROR, VERIFIER) << "Failed to open file " << file_name;
            return false;
        }
        buffer.assign(std::istreambuf_iterator<char>(base_file), {});
        base_file.close();
        return true;
    }

    // Tamper the first occurrence of the pattern in the buffer.
    static bool TamperFirst(std::vector<unsigned char> &buffer, const std::vector<unsigned char> &pattern,
                            const std::vector<unsigned char> &replacement)
    {
        for (size_t i = 0; i + pattern.size() <= buffer.size(); i++) {
            bool matched = true;
            for (size_t j = 0; j < pattern.size(); j++) {
                if (buffer[i + j] != pattern[j]) {
                    matched = false;
                    break;
                }
            }
            if (matched) {
                for (size_t j = 0; j < replacement.size(); j++) {
                    buffer[i + j] = replacement[j];
                }
                return true;
            }
        }
        return false;
    }

    // Tamper the first occurrence of "asyncfunctionresolve vN, return" or
    // "asyncfunctionreject vN, return" in the buffer into a direct undefined return.
    static bool TamperTlaMainReturn(std::vector<unsigned char> &buffer)
    {
        constexpr unsigned char ASYNCFUNCTIONRESOLVE_OP = 205;  // 0xcd
        constexpr unsigned char ASYNCFUNCTIONREJECT_OP = 206;   // 0xce
        constexpr unsigned char RETURN_OP = 100;                // 0x64
        for (size_t i = 0; i + 2U < buffer.size(); i++) {
            if ((buffer[i] == ASYNCFUNCTIONRESOLVE_OP || buffer[i] == ASYNCFUNCTIONREJECT_OP) &&
                buffer[i + 2U] == RETURN_OP) {
                buffer[i + 2U] = static_cast<unsigned char>(RETURN_OP + 1);  // returnundefined
                return true;
            }
        }
        return false;
    }

    static uint32_t ReadU32Le(const std::vector<unsigned char> &buffer, size_t offset)
    {
        uint32_t value = 0;
        for (size_t i = 0; i < sizeof(uint32_t); i++) {
            value |= static_cast<uint32_t>(buffer[offset + i]) << (i * 8U);
        }
        return value;
    }

    static void WriteU32Le(std::vector<unsigned char> &buffer, size_t offset, uint32_t value)
    {
        for (size_t i = 0; i < sizeof(uint32_t); i++) {
            buffer[offset + i] = static_cast<unsigned char>((value >> (i * 8U)) & 0xffU);
        }
    }

    // Tamper the last entry of the method index region (the constant pool) into
    // an offset beyond the file size.
    static bool TamperLastConstantPoolEntry(std::vector<unsigned char> &buffer)
    {
        // Header layout: [magic 8][checksum 4][version 4][file size 4][foreign off 4][foreign size 4]
        // [num classes 4][class idx off 4][num lnps 4][lnp idx off 4][num literalarrays 4]
        // [literalarray idx off 4][num indexes 4][index section off 4]
        constexpr size_t NUM_INDEXES_OFF = 52U;
        constexpr size_t INDEX_SECTION_OFF = 56U;
        if (buffer.size() <= INDEX_SECTION_OFF + 4U) {
            return false;
        }
        uint32_t num_indexes = ReadU32Le(buffer, NUM_INDEXES_OFF);
        uint32_t index_section_off = ReadU32Le(buffer, INDEX_SECTION_OFF);
        if (num_indexes == 0 || index_section_off + 24U > buffer.size()) {
            return false;
        }
        // IndexHeader layout: [start][end][class idx size][class idx off][method idx size][method idx off]
        constexpr size_t METHOD_IDX_SIZE_OFF = 16U;
        constexpr size_t METHOD_IDX_OFF_OFF = 20U;
        uint32_t method_idx_size = ReadU32Le(buffer, index_section_off + METHOD_IDX_SIZE_OFF);
        uint32_t method_idx_off = ReadU32Le(buffer, index_section_off + METHOD_IDX_OFF_OFF);
        if (method_idx_size == 0 || method_idx_off + 4U * method_idx_size > buffer.size()) {
            return false;
        }
        uint32_t invalid_offset = 0xfffffff0U;
        size_t last_entry = method_idx_off + 4U * (method_idx_size - 1U);
        WriteU32Le(buffer, last_entry, invalid_offset);
        return true;
    }

    static bool ReadIndexSection(const std::vector<unsigned char> &buffer, uint32_t &num_indexes,
                                 uint32_t &index_section_off)
    {
        constexpr size_t NUM_INDEXES_OFF = 52U;
        constexpr size_t INDEX_SECTION_OFF = 56U;
        if (buffer.size() <= INDEX_SECTION_OFF + 4U) {
            return false;
        }
        num_indexes = ReadU32Le(buffer, NUM_INDEXES_OFF);
        index_section_off = ReadU32Le(buffer, INDEX_SECTION_OFF);
        return num_indexes != 0 && index_section_off + 24U <= buffer.size();
    }

    // Tamper the first catch-all catch block (type encoded 0x0) of the file
    // into a typed catch whose decoded type_idx is out of the bounds of the
    // class index of the method.
    static bool TamperCatchTypeIdxOutOfBounds(std::vector<unsigned char> &buffer)
    {
        uint32_t num_indexes = 0;
        uint32_t index_section_off = 0;
        if (!ReadIndexSection(buffer, num_indexes, index_section_off)) {
            return false;
        }
        // IndexHeader layout: [start][end][class idx size][class idx off][method idx size][method idx off]
        constexpr size_t CLASS_IDX_SIZE_OFF = 8U;
        uint32_t class_idx_size = ReadU32Le(buffer, index_section_off + CLASS_IDX_SIZE_OFF);
        // The tampered encoded type is class_idx_size + 1, so the decoded
        // type_idx class_idx_size is just out of the bounds of the class
        // index. It must stay a single byte uleb128 value.
        if (class_idx_size == 0 || class_idx_size + 1U >= 0x80U) {
            return false;
        }
        const std::vector<unsigned char> catch_block = {0x00, 0x12, 0x00};
        const std::vector<unsigned char> invalid_catch_block = {static_cast<unsigned char>(class_idx_size + 1U), 0x12,
                                                                0x00};
        return TamperFirst(buffer, catch_block, invalid_catch_block);
    }

    static bool VisitInternalClasses(const panda_file::File &file,
                                     const std::function<bool(panda_file::ClassDataAccessor &)> &cb)
    {
        for (uint32_t class_id : file.GetClasses()) {
            const panda_file::File::EntityId record_id {class_id};
            if (file.IsExternal(record_id)) {
                continue;
            }
            panda_file::ClassDataAccessor class_accessor {file, record_id};
            if (cb(class_accessor)) {
                return true;
            }
        }
        return false;
    }

    static bool FindSlotOffsetInCode(const panda_file::File &file, panda_file::CodeDataAccessor &code_accessor,
                                     uint8_t opcode, size_t &offset)
    {
        auto bc_ins = BytecodeInstruction(code_accessor.GetInstructions());
        const auto bc_ins_last = bc_ins.JumpTo(code_accessor.GetCodeSize());
        while (bc_ins.GetAddress() < bc_ins_last.GetAddress()) {
            if (bc_ins.GetPrimaryOpcode() == opcode) {
                offset = static_cast<size_t>(bc_ins.GetAddress() + 1U - file.GetBase());
                return true;
            }
            bc_ins = bc_ins.GetNext();
        }
        return false;
    }

    static bool FindSlotOffsetByOpcode(const std::string &file_name, uint8_t opcode, size_t &offset)
    {
        auto file = panda_file::File::Open(file_name);
        if (file == nullptr) {
            return false;
        }
        return VisitInternalClasses(*file, [&](panda_file::ClassDataAccessor &class_accessor) {
            bool found = false;
            class_accessor.EnumerateMethods([&](panda_file::MethodDataAccessor &method_accessor) -> void {
                if (found || !method_accessor.GetCodeId().has_value()) {
                    return;
                }
                panda_file::CodeDataAccessor code_accessor {*file, method_accessor.GetCodeId().value()};
                found = FindSlotOffsetInCode(*file, code_accessor, opcode, offset);
            });
            return found;
        });
    }

    // Finds the file offset of the instruction array of the first method with
    // code, the caller tampers the first bytes of a real instruction of it.
    static bool FindFirstInstructionOffset(const std::string &file_name, size_t &offset)
    {
        auto file = panda_file::File::Open(file_name);
        if (file == nullptr) {
            return false;
        }
        return VisitInternalClasses(*file, [&](panda_file::ClassDataAccessor &class_accessor) {
            bool found = false;
            class_accessor.EnumerateMethods([&](panda_file::MethodDataAccessor &method_accessor) -> void {
                if (found || !method_accessor.GetCodeId().has_value()) {
                    return;
                }
                panda_file::CodeDataAccessor code_accessor {*file, method_accessor.GetCodeId().value()};
                if (code_accessor.GetCodeSize() < 2U) {
                    return;
                }
                offset = static_cast<size_t>(code_accessor.GetInstructions() - file->GetBase());
                found = true;
            });
            return found;
        });
    }

    static bool FindFirstImm16SlotOffset(const std::string &file_name, size_t &offset)
    {
        constexpr uint8_t LDOBJBYNAME_IMM16_OP = 0x90;
        return FindSlotOffsetByOpcode(file_name, LDOBJBYNAME_IMM16_OP, offset);
    }

    static bool FindFirstImm8SlotOffset(const std::string &file_name, size_t &offset)
    {
        constexpr uint8_t LDOBJBYNAME_IMM8_OP = 0x42;
        return FindSlotOffsetByOpcode(file_name, LDOBJBYNAME_IMM8_OP, offset);
    }

    static bool ScanLastInstruction(const panda_file::File &file, panda_file::CodeDataAccessor &code_accessor,
                                    size_t &offset, size_t &bytes_to_end)
    {
        auto bc_ins = BytecodeInstruction(code_accessor.GetInstructions());
        const auto bc_ins_last = bc_ins.JumpTo(code_accessor.GetCodeSize());
        auto current = bc_ins;
        bool found = false;
        while (current.GetAddress() < bc_ins_last.GetAddress()) {
            if (current.GetNext().GetAddress() <= bc_ins_last.GetAddress()) {
                offset = static_cast<size_t>(current.GetAddress() - file.GetBase());
                bytes_to_end = static_cast<size_t>(bc_ins_last.GetAddress() - current.GetAddress());
                found = true;
            }
            current = current.GetNext();
        }
        return found;
    }

    // Finds the file offset of the last instruction of the first method with
    // code and its distance to the code end, the caller tampers the
    // instruction into a wider one which reads operands beyond the code size.
    static bool FindLastInstructionOffset(const std::string &file_name, size_t &offset, size_t &bytes_to_end)
    {
        auto file = panda_file::File::Open(file_name);
        if (file == nullptr) {
            return false;
        }
        return VisitInternalClasses(*file, [&](panda_file::ClassDataAccessor &class_accessor) {
            bool found = false;
            class_accessor.EnumerateMethods([&](panda_file::MethodDataAccessor &method_accessor) -> void {
                if (found || !method_accessor.GetCodeId().has_value()) {
                    return;
                }
                panda_file::CodeDataAccessor code_accessor {*file, method_accessor.GetCodeId().value()};
                found = ScanLastInstruction(*file, code_accessor, offset, bytes_to_end);
            });
            return found;
        });
    }

    // Collects the literal array ids of the abc file sorted ascending, so
    // that the literal array a test tampers is deterministic.
    static bool CollectSortedLiteralIds(const std::string &file_name, std::vector<uint32_t> &literal_ids)
    {
        panda::verifier::Verifier ver {file_name};
        if (!ver.CollectIdInfos()) {
            return false;
        }
        literal_ids.assign(ver.literal_ids_.begin(), ver.literal_ids_.end());
        std::sort(literal_ids.begin(), literal_ids.end());
        return !literal_ids.empty();
    }
};

/**
 * @tc.name: verifier_new_checks_001
 * @tc.desc: Verify a module abc file, the module record content and the module
 * related instructions in the esmodule record are valid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_module_content.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyConstantPoolIndex());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.VerifyRegisterIndex());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_002
 * @tc.desc: Verify an abc file of a module with top level await, the return of
 * its func_main_0 is not undefined.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_002, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_tla_main.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_003
 * @tc.desc: Verify the modified abc file of a module with top level await whose
 * func_main_0 returns undefined directly.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_003, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_tla_main.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    ASSERT_TRUE(TamperTlaMainReturn(buffer));
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_003.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    // The direct undefined return of the func_main_0 of a TLA record is
    // rejected during the collection phase.
    EXPECT_FALSE(ver.CollectIdInfos());
}

/**
 * @tc.name: verifier_new_checks_004
 * @tc.desc: Verify the modified abc file whose literal array index in the
 * instruction is out of bounds of the constant pool and is resolved to offset 0x0.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_004, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    // The known literal array id operand of an instruction in the abc file
    std::vector<unsigned char> literal_id = {0x0f, 0x00};
    std::vector<unsigned char> invalid_id = {0xfe, 0xff};
    ASSERT_TRUE(TamperFirst(buffer, literal_id, invalid_id));
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_004.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    // The 16 bit constant pool index is out of bounds and rejected during the collection.
    EXPECT_FALSE(ver.CollectIdInfos());
}

/**
 * @tc.name: verifier_new_checks_005
 * @tc.desc: Verify the modified abc file whose constant pool id is out of the
 * bounds of the file.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_005, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    ASSERT_TRUE(TamperLastConstantPoolEntry(buffer));
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_005.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    // The tampered constant pool id may also be rejected during the collection
    // when it is referenced by an instruction, the pool bounds check always fails.
    ver.CollectIdInfos();
    EXPECT_FALSE(ver.VerifyConstantPoolIndex());
}

/**
 * @tc.name: verifier_new_checks_006
 * @tc.desc: Verify the modified abc file whose catch block type idx is out of
 * the bounds of the class index of the method.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_006, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    // The known catch block of the abc file: type idx(uleb encoded 0x0, catch all),
    // handler pc 0x12, code size 0x0. The type is tampered into a decoded
    // type_idx beyond the class index of the method, a catch-all (encoded 0x0,
    // decoded INVALID_INDEX) and a typed catch resolving to class index 0
    // (encoded 0x1) both stay valid.
    ASSERT_TRUE(TamperCatchTypeIdxOutOfBounds(buffer));
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_006.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_FALSE(ver.VerifyConstantPoolContent());
}

/**
 * @tc.name: verifier_new_checks_007
 * @tc.desc: Verify the modified abc file with an invalid secondary opcode of
 * a wide prefixed instruction.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_007, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_module_content.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    // Locate a real instruction of a method, a raw pattern search may hit a
    // byte outside the instruction region which depends on the abc layout.
    size_t insn_offset = 0;
    ASSERT_TRUE(FindFirstInstructionOffset(base_file_name, insn_offset));
    ASSERT_LT(insn_offset + 1U, buffer.size());
    // Tamper the first two bytes of the instruction into a wide prefixed
    // instruction whose secondary opcode 0x65 is invalid.
    buffer[insn_offset] = 0xfd;
    buffer[insn_offset + 1U] = 0x65;
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_007.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    EXPECT_FALSE(ver.CollectIdInfos());
}

/**
 * @tc.name: verifier_new_checks_008
 * @tc.desc: Verify an abc file whose methods use ic slot indexes beyond 0x80
 * in the 8 bit encoding: the slot operand is declared unsigned (u8 / u16) in
 * the isa and is read zero extended.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_008, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_ic_slots.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(file_name, buffer));
    // The 8 bit encoded slot operands with the high bit set (0x80..0xfe) must
    // appear in the compiled property accesses of the fixture.
    bool has_high_bit_slot = false;
    for (size_t i = 0; i + 1U < buffer.size(); i++) {
        if (buffer[i] == 0x42U && (buffer[i + 1U] & 0x80U) != 0) {  // ldobjbyname imm8
            has_high_bit_slot = true;
            break;
        }
    }
    ASSERT_TRUE(has_high_bit_slot);

    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_009
 * @tc.desc: Verify the modified abc file whose 16 bit encoded ic slot index of
 * a ldobjbyname instruction exceeds the slot number annotation.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_009, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_ic_slots.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    size_t slot_offset = 0;
    ASSERT_TRUE(FindFirstImm16SlotOffset(base_file_name, slot_offset));
    ASSERT_LT(slot_offset + 1U, buffer.size());
    // Tamper the slot operand to the maximum 16 bit value, the slot number
    // annotation of the fixture method is far below it.
    buffer[slot_offset] = 0xff;
    buffer[slot_offset + 1U] = 0xff;
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_009.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_FALSE(ver.VerifyConstantPoolContent());
}

/**
 * @tc.name: verifier_new_checks_010
 * @tc.desc: Verify the modified abc file whose literal array contains the
 * TAGVALUE tag: the tag is rejected by the literal enumeration of the runtime
 * (LiteralDataAccessor::EnumerateLiteralVals aborts on it), so the literal
 * array is invalid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_010, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    std::vector<uint32_t> literal_ids;
    ASSERT_TRUE(CollectSortedLiteralIds(base_file_name, literal_ids));
    // Tamper the first tag of every literal array into TAGVALUE (0x00), the
    // byte is also the most common one of garbage data.
    for (const auto &literal_id : literal_ids) {
        buffer[static_cast<size_t>(literal_id) + sizeof(uint32_t)] = 0x00;
    }
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_010.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    EXPECT_FALSE(ver.CollectIdInfos() && ver.VerifyConstantPoolContent());
}

/**
 * @tc.name: verifier_new_checks_011
 * @tc.desc: Verify the modified abc file whose literal array contains the
 * ETS_IMPLEMENTS tag: the tag is accepted by the literal enumeration of the
 * runtime, the file stays valid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_011, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    std::vector<uint32_t> literal_ids;
    ASSERT_TRUE(CollectSortedLiteralIds(base_file_name, literal_ids));
    // Tamper the first fixed width tag (an ARRAY_* tag consumes the rest of
    // the buffer and must stay untouched) into ETS_IMPLEMENTS (0x1c), the
    // value width of both tags is 4 bytes, so the file stays structurally
    // valid and must be accepted.
    constexpr uint8_t ETS_IMPLEMENTS_TAG = 0x1c;
    bool tampered = false;
    for (const auto &literal_id : literal_ids) {
        size_t tag_off = static_cast<size_t>(literal_id) + sizeof(uint32_t);
        if (buffer[tag_off] >= 0x0aU && buffer[tag_off] <= 0x15U) {
            continue;  // ARRAY_* tags consume the rest of the buffer
        }
        buffer[tag_off] = ETS_IMPLEMENTS_TAG;
        tampered = true;
        break;
    }
    ASSERT_TRUE(tampered);
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_011.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyConstantPoolIndex());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
}

/**
 * @tc.name: verifier_new_checks_012
 * @tc.desc: Verify the modified abc file whose ic slot operand of a
 * ldobjbyname instruction is tampered into the slot of another ic site: the
 * sequential slot allocation of the frontend is broken, the profile type
 * info array slot is aliased.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_012, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_ic_slots.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    size_t slot_offset = 0;
    ASSERT_TRUE(FindFirstImm8SlotOffset(base_file_name, slot_offset));
    ASSERT_LT(slot_offset, buffer.size());
    // Tamper the slot operand into another still in bounds slot index: every
    // slot participating instruction must carry the operand the frontend
    // assigned to it in the sequential allocation.
    buffer[slot_offset] = static_cast<unsigned char>(buffer[slot_offset] + 1U);
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_012.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_FALSE(ver.VerifyConstantPoolContent());
}

/**
 * @tc.name: verifier_new_checks_013
 * @tc.desc: Verify the modified abc file whose last instruction is tampered
 * into a wider one reading operands beyond the code size.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_013, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    size_t insn_offset = 0;
    size_t bytes_to_end = 0;
    ASSERT_TRUE(FindLastInstructionOffset(base_file_name, insn_offset, bytes_to_end));
    // Tamper the last instruction into ldai (op_imm_32, 5 bytes) whose end
    // goes past the code size.
    constexpr size_t LDAI_INSN_SIZE = 5U;
    ASSERT_LT(bytes_to_end, LDAI_INSN_SIZE);
    buffer[insn_offset] = 0x62;
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_013.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    EXPECT_FALSE(ver.CollectIdInfos());
}

/**
 * @tc.name: verifier_new_checks_014
 * @tc.desc: Verify an abc file whose methods overflow the 8 bit ic slot
 * space: the frontend writes INVALID_IC_SLOT sentinels and re-assigns the
 * slots with a two pass sort (the 8 bit only instructions first), the
 * allocation consistency check accepts the rearranged layout.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_014, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_ic_slots_overflow.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyConstantPoolIndex());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_015
 * @tc.desc: Verify a compiled class/literal abc through every public entry of
 * the verifier, the file is well formed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_015, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyChecksum());
    EXPECT_TRUE(ver.VerifyConstantPoolIndex());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.VerifyRegisterIndex());
    EXPECT_TRUE(ver.VerifyConstantPool());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_016
 * @tc.desc: Verify the modified abc file whose checksum is corrupted.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_016, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_module_content.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    ASSERT_GT(buffer.size(), 12U);
    buffer[8] = static_cast<unsigned char>(buffer[8] + 1U);
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_016.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    EXPECT_FALSE(ver.VerifyChecksum());
    EXPECT_FALSE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_017
 * @tc.desc: Verify the modified abc file whose first instruction is an unknown
 * primary opcode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_017, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_register_index.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    size_t insn_offset = 0;
    ASSERT_TRUE(FindFirstInstructionOffset(base_file_name, insn_offset));
    ASSERT_LT(insn_offset, buffer.size());
    buffer[insn_offset] = 0xff;
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_017.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    EXPECT_FALSE(ver.CollectIdInfos());
}

/**
 * @tc.name: verifier_new_checks_018
 * @tc.desc: Verify the modified abc file whose literal array count field is
 * truncated to an odd value.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_018, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool_content.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    std::vector<uint32_t> literal_ids;
    ASSERT_TRUE(CollectSortedLiteralIds(base_file_name, literal_ids));
    WriteU32Le(buffer, static_cast<size_t>(literal_ids.front()), 1U);
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_018.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    EXPECT_FALSE(ver.CollectIdInfos() && ver.VerifyConstantPoolContent());
}

/**
 * @tc.name: verifier_new_checks_019
 * @tc.desc: Verify a register-index fixture, every public entry stays valid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_019, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_register_index.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyRegisterIndex());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_020
 * @tc.desc: Verify the modified abc file whose 8 bit ic slot is overwritten
 * with 0xfe. ldobjbyname occupies two slots, so the sequential assignment
 * jumps from 0xfc to 0x100 and leaves 0xfe unused. The range fallback still
 * accepts that in-bounds non-overlapping hole, matching dead-code gaps.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_020, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_ic_slots.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    size_t slot_offset = 0;
    ASSERT_TRUE(FindFirstImm8SlotOffset(base_file_name, slot_offset));
    ASSERT_LT(slot_offset, buffer.size());
    buffer[slot_offset] = 0xfe;
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_020.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
}

/**
 * @tc.name: verifier_new_checks_021
 * @tc.desc: Verify a checksum fixture file as a well formed abc.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_021, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_checksum.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyChecksum());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_022
 * @tc.desc: Verify the modified abc file whose last constant pool entry is
 * replaced with offset 0.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_022, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_constant_pool.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    ASSERT_TRUE(TamperLastConstantPoolEntry(buffer));
    constexpr size_t INDEX_SECTION_OFF = 56U;
    uint32_t index_section_off = ReadU32Le(buffer, INDEX_SECTION_OFF);
    uint32_t method_idx_off = ReadU32Le(buffer, index_section_off + 20U);
    uint32_t method_idx_size = ReadU32Le(buffer, index_section_off + 16U);
    WriteU32Le(buffer, method_idx_off + 4U * (method_idx_size - 1U), 0);
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_022.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    ver.CollectIdInfos();
    EXPECT_FALSE(ver.VerifyConstantPoolIndex());
}

/**
 * @tc.name: verifier_new_checks_023
 * @tc.desc: Verify a module abc through register index and checksum together.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_023, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_module_content.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyChecksum());
    EXPECT_TRUE(ver.VerifyRegisterIndex());
    EXPECT_TRUE(ver.VerifyConstantPool());
}

/**
 * @tc.name: verifier_new_checks_024
 * @tc.desc: Verify the modified abc file whose 16 bit ic slot is overwritten
 * with 0x8000, which exceeds the annotation of the fixture.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_024, TestSize.Level1)
{
    const std::string base_file_name = GRAPH_TEST_ABC_DIR "test_ic_slots.abc";
    std::vector<unsigned char> buffer;
    ASSERT_TRUE(ReadAbcFile(base_file_name, buffer));
    size_t slot_offset = 0;
    ASSERT_TRUE(FindFirstImm16SlotOffset(base_file_name, slot_offset));
    ASSERT_LT(slot_offset + 1U, buffer.size());
    buffer[slot_offset] = 0x00;
    buffer[slot_offset + 1U] = 0x80;
    const std::string tar_file_name = GRAPH_TEST_ABC_DIR "verifier_new_checks_024.abc";
    GenerateModifiedAbc(buffer, tar_file_name);

    panda::verifier::Verifier ver {tar_file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_FALSE(ver.VerifyConstantPoolContent());
}

/**
 * @tc.name: verifier_new_checks_025
 * @tc.desc: Verify a class with getter, setter and static method.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_025, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_class_methods.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyConstantPoolIndex());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_026
 * @tc.desc: Verify nested try/catch/finally bytecode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_026, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_try_nested.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyRegisterIndex());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_027
 * @tc.desc: Verify generator and async generator methods.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_027, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_generators.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_028
 * @tc.desc: Verify object spread, rest and optional chain bytecode.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_028, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_object_literal.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyChecksum());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.Verify());
}

/**
 * @tc.name: verifier_new_checks_029
 * @tc.desc: Verify a module with named and default exports.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(VerifierNewChecksTest, verifier_new_checks_029, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_named_exports.abc";
    panda::verifier::Verifier ver {file_name};
    ASSERT_TRUE(ver.CollectIdInfos());
    EXPECT_TRUE(ver.VerifyConstantPoolIndex());
    EXPECT_TRUE(ver.VerifyConstantPoolContent());
    EXPECT_TRUE(ver.VerifyRegisterIndex());
    EXPECT_TRUE(ver.Verify());
}
}  // namespace panda::verifier
