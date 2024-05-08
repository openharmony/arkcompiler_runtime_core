/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <vector>
#include "abc2program_driver.h"
#include "abc2program_test_utils.h"
#include "common/abc_file_utils.h"

using namespace testing::ext;

namespace panda::abc2program {
const std::string HELLO_WORLD_ABC_TEST_FILE_NAME = GRAPH_TEST_ABC_DIR "HelloWorld.abc";
constexpr uint32_t NUM_OF_CODE_TEST_UT_FOO_METHOD_INS = 77;
constexpr std::string_view FUNC_NAME_HELLO_WORLD = ".HelloWorld";
constexpr std::string_view FUNC_NAME_FOO = ".foo";
constexpr std::string_view FUNC_NAME_GOO = ".goo";
constexpr std::string_view FUNC_NAME_HOO = ".hoo";
constexpr std::string_view FUNC_NAME_MAIN = ".func_main_0";
constexpr uint8_t INS_SIZE_OF_FUNCTION_HOO = 7;
constexpr uint8_t IMMS_SIZE_OF_OPCODE_FLDAI = 1;
constexpr uint8_t SIZE_OF_LITERAL_ARRAY_TABLE = 4;
constexpr uint8_t TOTAL_NUM_OF_MODULE_LITERALS = 21;
constexpr uint8_t NUM_OF_MODULE_REQUESTS = 4;
constexpr uint8_t NUM_OF_REGULAR_IMPORTS = 1;
constexpr uint8_t NUM_OF_NAMESPACE_IMPORTS = 1;
constexpr uint8_t NUM_OF_LOCAL_EXPORTS = 1;
constexpr uint8_t NUM_OF_INDIRECT_EXPORTS = 1;
constexpr uint8_t NUM_OF_STAR_EXPORTS = 1;

class Abc2ProgramHelloWorldTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        (void)driver_.Compile(HELLO_WORLD_ABC_TEST_FILE_NAME);
        prog_ = &(driver_.GetProgram());
        for (const auto &it : prog_->function_table) {
            if (it.first == FUNC_NAME_HELLO_WORLD) {
                hello_world_function_ = &(it.second);
            }
            if (it.first == FUNC_NAME_FOO) {
                foo_function_ = &(it.second);
            }
            if (it.first == FUNC_NAME_GOO) {
                goo_function_ = &(it.second);
            }
            if (it.first == FUNC_NAME_HOO) {
                hoo_function_ = &(it.second);
            }
            if (it.first == FUNC_NAME_MAIN) {
                main_function_ = &(it.second);
            }
        }
    }

    void TearDown() {}

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
    const pandasm::Function *hello_world_function_ = nullptr;
    const pandasm::Function *foo_function_ = nullptr;
    const pandasm::Function *goo_function_ = nullptr;
    const pandasm::Function *hoo_function_ = nullptr;
    const pandasm::Function *main_function_ = nullptr;
};

/**
 * @tc.name: abc2program_hello_world_test_func_annotation
 * @tc.desc: get program function annotation.
 * @tc.type: FUNC
 * @tc.require: #I9AQ3K
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_func_annotation, TestSize.Level1)
{
    constexpr uint32_t NUM_OF_HELLO_WORLD_TEST_UT_HELLO_WORLD_SLOTS_NUM = 2;
    constexpr uint32_t NUM_OF_HELLO_WORLD_TEST_UT_FOO_SLOTS_NUM = 24;
    constexpr uint32_t NUM_OF_HELLO_WORLD_TEST_UT_GOO_SLOTS_NUM = 0;
    EXPECT_TRUE(hello_world_function_->GetSlotsNum() == NUM_OF_HELLO_WORLD_TEST_UT_HELLO_WORLD_SLOTS_NUM);
    EXPECT_TRUE(foo_function_->GetSlotsNum() == NUM_OF_HELLO_WORLD_TEST_UT_FOO_SLOTS_NUM);
    EXPECT_TRUE(goo_function_->GetSlotsNum() == NUM_OF_HELLO_WORLD_TEST_UT_GOO_SLOTS_NUM);
    EXPECT_TRUE(hello_world_function_->concurrent_module_requests.empty());
    EXPECT_TRUE(foo_function_->concurrent_module_requests.empty());
    EXPECT_TRUE(goo_function_->concurrent_module_requests.empty());
}

/**
 * @tc.name: abc2program_hello_world_test_field_metadata
 * @tc.desc: get program field metadata.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_field_metadata, TestSize.Level1)
{
    for (const auto &it : prog_->record_table) {
        if (it.first == "_ESModuleRecord") {
            const pandasm::Record &record = it.second;
            const std::vector<pandasm::Field> &field_list = record.field_list;
            const pandasm::Field &field = field_list[0];
            EXPECT_TRUE(field.type.GetPandasmName() == "u32");
            EXPECT_TRUE(field.name.find("HelloWorld.js") != std::string::npos);
        }
    }
}

/**
 * @tc.name: abc2program_hello_world_test_lang
 * @tc.desc: get program language.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_lang, TestSize.Level1)
{
    panda_file::SourceLang expected_lang = panda_file::SourceLang::ECMASCRIPT;
    bool lang_matched = (prog_->lang == expected_lang);
    EXPECT_TRUE(lang_matched);
}

/**
 * @tc.name: abc2program_hello_world_test_literalarray_table
 * @tc.desc: get program literalarray_table.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_literalarray_table, TestSize.Level1)
{
    std::set<size_t> literals_sizes;
    std::vector<std::string> literal_array_keys;
    for (const auto &it : prog_->literalarray_table) {
        const pandasm::LiteralArray &literal_array = it.second;
        size_t literals_size = literal_array.literals_.size();
        literals_sizes.insert(literals_size);
        literal_array_keys.emplace_back(it.first);
    }
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateLiteralsSizes(literals_sizes));
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateLiteralArrayKeys(literal_array_keys));
}

/**
 * @tc.name: abc2program_hello_world_test_record_table
 * @tc.desc: get program record_table.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_record_table, TestSize.Level1)
{
    panda_file::SourceLang expected_lang = panda_file::SourceLang::ECMASCRIPT;
    std::vector<std::string> record_names;
    for (const auto &it : prog_->record_table) {
        EXPECT_TRUE(it.second.language == expected_lang);
        EXPECT_TRUE(it.second.source_file == "");
        EXPECT_TRUE(it.first == it.second.name);
        record_names.emplace_back(it.first);
    }
    EXPECT_TRUE(Abc2ProgramTestUtils::ValidateRecordNames(record_names));
}

/**
 * @tc.name: abc2program_hello_world_test_fields
 * @tc.desc: get program record_table.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_fields, TestSize.Level1)
{
    for (const auto &it : prog_->record_table) {
        if (it.first == "_ESModuleRecord") {
            const pandasm::Record &record = it.second;
            const std::vector<pandasm::Field> &field_list = record.field_list;
            EXPECT_TRUE(field_list.size() == 1);
            const pandasm::Field &field = field_list[0];
            EXPECT_TRUE(field.type.GetPandasmName() == "u32");
            EXPECT_TRUE(field.name.find("HelloWorld.js") != std::string::npos);
        } else {
            EXPECT_TRUE(it.second.field_list.size() == 0);
        }
    }
}

/**
 * @tc.name: abc2program_hello_world_test_strings
 * @tc.desc: get existed string.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_strings, TestSize.Level1)
{
    bool string_matched = Abc2ProgramTestUtils::ValidateProgramStrings(prog_->strings);
    EXPECT_TRUE(string_matched);
}

/**
 * @tc.name: abc2program_hello_world_test_function_kind_access_flags
 * @tc.desc: get existed function_kind.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_function_kind_access_flags, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    panda_file::FunctionKind function_kind = function.function_kind;
    EXPECT_TRUE(function_kind == panda_file::FunctionKind::FUNCTION);
    uint32_t access_flags = function.metadata->GetAccessFlags();
    EXPECT_TRUE(access_flags == 0x08);
}

/**
 * @tc.name: abc2program_code_test_function_foo_part1
 * @tc.desc: get program fuction foo.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_test_function_foo_part1, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    size_t regs_num = function.regs_num;
    constexpr size_t NUM_OF_ARGS_FOR_FOO_METHOD = 3;
    EXPECT_TRUE(function.params.size() == NUM_OF_ARGS_FOR_FOO_METHOD);
    for (size_t i = 0; i < function.params.size(); ++i) {
        EXPECT_TRUE(function.params[i].type.GetPandasmName() == ANY_TYPE_NAME);
    }
    EXPECT_TRUE(function.name == FUNC_NAME_FOO);
    EXPECT_TRUE(function.ins.size() == NUM_OF_CODE_TEST_UT_FOO_METHOD_INS);
    // check ins[0]
    const pandasm::Ins &ins0 = function.ins[0];
    std::string pa_ins_str0 = ins0.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str0 == "mov v0, a0");
    EXPECT_TRUE(ins0.label == "");
    EXPECT_FALSE(ins0.set_label);
    // check ins[1]
    const pandasm::Ins &ins1 = function.ins[1];
    std::string pa_ins_str1 = ins1.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str1 == "mov v1, a1");
    EXPECT_TRUE(ins1.label == "");
    EXPECT_FALSE(ins1.set_label);
    // check ins[2]
    const pandasm::Ins &ins2 = function.ins[2];
    std::string pa_ins_str2 = ins2.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str2 == "mov v2, a2");
    EXPECT_TRUE(ins2.label == "");
    EXPECT_FALSE(ins2.set_label);
    // check ins[5]
    const pandasm::Ins &ins5 = function.ins[5];
    std::string pa_ins_str5 = ins5.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str5 == "label@5: ldai 0xb");
    EXPECT_TRUE(ins5.label == "label@5");
    EXPECT_TRUE(ins5.set_label);
    // check ins[9]
    const pandasm::Ins &ins9 = function.ins[9];
    std::string pa_ins_str9 = ins9.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str9 == "label@9: ldai 0x1");
    EXPECT_TRUE(ins9.label == "label@9");
    EXPECT_TRUE(ins9.set_label);
    // check ins[11]
    const pandasm::Ins &ins11 = function.ins[11];
    std::string pa_ins_str11 = ins11.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str11 == "label@11: jmp label@21");
    EXPECT_TRUE(ins11.label == "label@11");
    EXPECT_TRUE(ins11.set_label);
}

/**
 * @tc.name: abc2program_code_test_function_foo_part2
 * @tc.desc: get program fuction foo.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_test_function_foo_part2, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    size_t regs_num = function.regs_num;
    
    // check ins[12]
    const pandasm::Ins &ins12 = function.ins[12];
    std::string pa_ins_str12 = ins12.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str12 == "label@12: sta v5");
    EXPECT_TRUE(ins12.label == "label@12");
    EXPECT_TRUE(ins12.set_label);
    // check ins[21]
    const pandasm::Ins &ins21 = function.ins[21];
    std::string pa_ins_str21 = ins21.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str21 == "label@21: tryldglobalbyname 0x8, varA");
    EXPECT_TRUE(ins21.label == "label@21");
    EXPECT_TRUE(ins21.set_label);
    // check ins[25]
    const pandasm::Ins &ins25 = function.ins[25];
    std::string pa_ins_str25 = ins25.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str25 == "jeqz label@28");
    EXPECT_TRUE(ins25.label == "");
    EXPECT_FALSE(ins25.set_label);
    // check ins[28]
    const pandasm::Ins &ins28 = function.ins[28];
    std::string pa_ins_str28 = ins28.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str28 == "label@28: tryldglobalbyname 0xa, x");
    EXPECT_TRUE(ins28.label == "label@28");
    EXPECT_TRUE(ins28.set_label);
    // check ins[32]
    const pandasm::Ins &ins32 = function.ins[32];
    std::string pa_ins_str32 = ins32.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str32 == "jeqz label@36");
    EXPECT_TRUE(ins32.label == "");
    EXPECT_FALSE(ins32.set_label);
    // check ins[35]
    const pandasm::Ins &ins35 = function.ins[35];
    std::string pa_ins_str35 = ins35.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str35 == "jmp label@38");
    EXPECT_TRUE(ins35.label == "");
    EXPECT_FALSE(ins35.set_label);
    // check ins[36]
    const pandasm::Ins &ins36 = function.ins[36];
    std::string pa_ins_str36 = ins36.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str36 == "label@36: lda.str min");
    EXPECT_TRUE(ins36.label == "label@36");
    EXPECT_TRUE(ins36.set_label);
}

/**
 * @tc.name: abc2program_code_test_function_foo_part3
 * @tc.desc: get program fuction foo.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_test_function_foo_part3, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    size_t regs_num = function.regs_num;
    // check ins[38]
    const pandasm::Ins &ins38 = function.ins[38];
    std::string pa_ins_str38 = ins38.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str38 == "label@38: jmp label@48");
    EXPECT_TRUE(ins38.label == "label@38");
    EXPECT_TRUE(ins38.set_label);
    // check ins[39]
    const pandasm::Ins &ins39 = function.ins[39];
    std::string pa_ins_str39 = ins39.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str39 == "label@39: sta v5");
    EXPECT_TRUE(ins39.label == "label@39");
    EXPECT_TRUE(ins39.set_label);
    // check ins[48]
    const pandasm::Ins &ins48 = function.ins[48];
    std::string pa_ins_str48 = ins48.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str48 == "label@48: ldhole");
    EXPECT_TRUE(ins48.label == "label@48");
    EXPECT_TRUE(ins48.set_label);
    // check ins[50]
    const pandasm::Ins &ins50 = function.ins[50];
    std::string pa_ins_str50 = ins50.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str50 == "jmp label@52");
    EXPECT_TRUE(ins50.label == "");
    EXPECT_FALSE(ins50.set_label);
    // check ins[51]
    const pandasm::Ins &ins51 = function.ins[51];
    std::string pa_ins_str51 = ins51.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str51 == "label@51: sta v5");
    EXPECT_TRUE(ins51.label == "label@51");
    EXPECT_TRUE(ins51.set_label);
    // check ins[52]
    const pandasm::Ins &ins52 = function.ins[52];
    std::string pa_ins_str52 = ins52.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str52 == "label@52: lda v4");
    EXPECT_TRUE(ins52.label == "label@52");
    EXPECT_TRUE(ins52.set_label);
    // check ins[56]
    const pandasm::Ins &ins56 = function.ins[56];
    std::string pa_ins_str56 = ins56.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str56 == "jeqz label@66");
    EXPECT_TRUE(ins56.label == "");
    EXPECT_FALSE(ins56.set_label);
    // check ins[66]
    const pandasm::Ins &ins66 = function.ins[66];
    std::string pa_ins_str66 = ins66.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str66 == "label@66: lda v5");
    EXPECT_TRUE(ins66.label == "label@66");
    EXPECT_TRUE(ins66.set_label);
}

/**
 * @tc.name: abc2program_code_test_function_foo_part4
 * @tc.desc: get program fuction foo.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_test_function_foo_part4, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    size_t regs_num = function.regs_num;
    // check ins[72]
    const pandasm::Ins &ins72 = function.ins[72];
    std::string pa_ins_str72 = ins72.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str72 == "jeqz label@75");
    EXPECT_TRUE(ins72.label == "");
    EXPECT_FALSE(ins72.set_label);
    // check ins[75]
    const pandasm::Ins &ins75 = function.ins[75];
    std::string pa_ins_str75 = ins75.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str75 == "label@75: ldundefined");
    EXPECT_TRUE(ins75.label == "label@75");
    EXPECT_TRUE(ins75.set_label);
    // check ins[76]
    const pandasm::Ins &ins76 = function.ins[76];
    std::string pa_ins_str76 = ins76.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str76 == "returnundefined");
    EXPECT_TRUE(ins76.label == "");
    EXPECT_FALSE(ins76.set_label);
    // check catch blocks
    constexpr uint32_t NUM_OF_CODE_TEST_UT_FOO_METHOD_CATCH_BLOCKS = 3;
    EXPECT_TRUE(function.catch_blocks.size() == NUM_OF_CODE_TEST_UT_FOO_METHOD_CATCH_BLOCKS);
    // catch_blocks[0]
    const pandasm::Function::CatchBlock &pa_catch_block0 = function.catch_blocks[0];
    EXPECT_TRUE(pa_catch_block0.try_begin_label == "label@9");
    EXPECT_TRUE(pa_catch_block0.try_end_label == "label@11");
    EXPECT_TRUE(pa_catch_block0.catch_begin_label == "label@12");
    EXPECT_TRUE(pa_catch_block0.catch_end_label == "label@21");
    // catch_blocks[1]
    const pandasm::Function::CatchBlock &pa_catch_block1 = function.catch_blocks[1];
    EXPECT_TRUE(pa_catch_block1.try_begin_label == "label@5");
    EXPECT_TRUE(pa_catch_block1.try_end_label == "label@38");
    EXPECT_TRUE(pa_catch_block1.catch_begin_label == "label@39");
    EXPECT_TRUE(pa_catch_block1.catch_end_label == "label@48");
    // catch_blocks[2]
    const pandasm::Function::CatchBlock &pa_catch_block2 = function.catch_blocks[2];
    EXPECT_TRUE(pa_catch_block2.try_begin_label == "label@5");
    EXPECT_TRUE(pa_catch_block2.try_end_label == "label@48");
    EXPECT_TRUE(pa_catch_block2.catch_begin_label == "label@51");
    EXPECT_TRUE(pa_catch_block2.catch_end_label == "label@52");
}

/**
 * @tc.name: abc2program_code_test_function_goo
 * @tc.desc: get program fuction goo.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_test_function_goo, TestSize.Level1)
{
    const pandasm::Function &function = *goo_function_;
    size_t regs_num = function.regs_num;
    constexpr uint32_t NUM_OF_CODE_TEST_UT_GOO_METHOD_INS = 5;
    EXPECT_TRUE(function.name == FUNC_NAME_GOO);
    EXPECT_TRUE(function.ins.size() == NUM_OF_CODE_TEST_UT_GOO_METHOD_INS);
    // check ins[0]
    constexpr uint32_t INDEX_OF_FUNC_LDUNDEFINED = 3;
    const pandasm::Ins &ins0 = function.ins[INDEX_OF_FUNC_LDUNDEFINED];
    std::string pa_ins_str0 = ins0.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str0 == "ldundefined");
    EXPECT_TRUE(ins0.label == "");
    EXPECT_FALSE(ins0.set_label);
    // check ins[1]
    constexpr uint32_t INDEX_OF_FUNC_RETURNUNDEFINED = 4;
    const pandasm::Ins &ins1 = function.ins[INDEX_OF_FUNC_RETURNUNDEFINED];
    std::string pa_ins_str1 = ins1.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str1 == "returnundefined");
    EXPECT_TRUE(ins1.label == "");
    EXPECT_FALSE(ins1.set_label);
    // check catch blocks
    EXPECT_TRUE(function.catch_blocks.size() == 0);
}

/**
 * @tc.name: abc2program_hello_world_test_source_file
 * @tc.desc: Verify the source file.
 * @tc.type: FUNC
 * @tc.require: issueI9CL5Z
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_source_file, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    std::string source_file = function.source_file;
    EXPECT_TRUE(source_file.find("HelloWorld.js") != std::string::npos);
}

/**
 * @tc.name: abc2program_hello_world_test_source_code
 * @tc.desc: Verify the source code
 * @tc.type: FUNC
 * @tc.require: issueI9DT0V
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_source_code, TestSize.Level1)
{
    const pandasm::Function &function = *main_function_;
    std::string source_code = function.source_code;
    EXPECT_TRUE(source_code.find("import {a} from './a'") != std::string::npos);
}

/**
 * @tc.name: abc2program_code_imm_of_FLDAI
 * @tc.desc: get and check immediate number of opcode FLDAI.
 * @tc.type: FUNC
 * @tc.require: issueI9E5ZM
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_code_imm_of_FLDAI, TestSize.Level1)
{
    const pandasm::Function &hoo = *hoo_function_;
    EXPECT_EQ(hoo.ins.size(), INS_SIZE_OF_FUNCTION_HOO);
    constexpr uint32_t INDEX_OF_FUNC_FLDAI = 3;
    auto &ins_fldai = hoo.ins[INDEX_OF_FUNC_FLDAI];
    EXPECT_TRUE(ins_fldai.opcode == pandasm::Opcode::FLDAI);
    // check imm of FLDAI
    EXPECT_EQ(ins_fldai.imms.size(), IMMS_SIZE_OF_OPCODE_FLDAI);
    auto p = std::get_if<double>(&ins_fldai.imms[0]);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(*p, 1.23);
}

/**
 * @tc.name: abc2program_module_literal_entry_test
 * @tc.desc: get and check number of modules.
 * @tc.type: FUNC
 * @tc.require: issueI9E5ZM
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_module_literal_entry_test, TestSize.Level1)
{
    auto &mod_table = prog_->literalarray_table;
    EXPECT_EQ(mod_table.size(), SIZE_OF_LITERAL_ARRAY_TABLE);
    auto &module_literals = mod_table.begin()->second.literals_;
    EXPECT_EQ(module_literals.size(), TOTAL_NUM_OF_MODULE_LITERALS);
    auto check_entry = [&module_literals](size_t idx, panda::panda_file::LiteralTag expect_tag, uint32_t expect_value) {
        auto *literal = &(module_literals[idx]);
        EXPECT_EQ(literal->tag_, expect_tag);
        auto p = std::get_if<uint32_t>(&literal->value_);
        EXPECT_TRUE(p != nullptr && *p == expect_value);
    };
    size_t idx = 0;
    // check ModuleRequests
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_MODULE_REQUESTS);
    // Each constant value '1' below stands for each entry. Each entry is also in the literal vector and shall be
    // considered while calculating the position (index) of next entry.
    // check RegularImportEntry
    idx += NUM_OF_MODULE_REQUESTS + 1;
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_REGULAR_IMPORTS);
    // check NamespaceImportEntry
    idx += NUM_OF_REGULAR_IMPORTS * LITERAL_NUM_OF_REGULAR_IMPORT + 1;
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_NAMESPACE_IMPORTS);
    // check LocalExportEntry
    idx += NUM_OF_NAMESPACE_IMPORTS * LITERAL_NUM_OF_NAMESPACE_IMPORT + 1;
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_LOCAL_EXPORTS);
    // check IndirectExportEntry
    idx += NUM_OF_LOCAL_EXPORTS * LITERAL_NUM_OF_LOCAL_EXPORT + 1;
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_INDIRECT_EXPORTS);
    // check StarExportEntry
    idx += NUM_OF_INDIRECT_EXPORTS * LITERAL_NUM_OF_INDIRECT_EXPORT + 1;
    check_entry(idx, panda::panda_file::LiteralTag::INTEGER, NUM_OF_STAR_EXPORTS);
    // check idx
    idx += NUM_OF_STAR_EXPORTS * LITERAL_NUM_OF_STAR_EXPORT + 1;
    EXPECT_EQ(idx, TOTAL_NUM_OF_MODULE_LITERALS);
}

/**
 * @tc.name: abc2program_hello_world_test_local_variable
 * @tc.desc: get local variables
 * @tc.type: FUNC
 * @tc.require: issueI9DT0V
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_local_variable, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    EXPECT_FALSE(function.local_variable_debug.empty());
    EXPECT_TRUE(function.local_variable_debug[0].name.find("4funcObj") != std::string::npos);
    EXPECT_TRUE(function.local_variable_debug[0].signature.find("any") != std::string::npos);
    EXPECT_TRUE(function.local_variable_debug[0].signature_type.find("any") != std::string::npos);
    EXPECT_TRUE(function.local_variable_debug[0].reg == 0);
    EXPECT_TRUE(function.local_variable_debug[0].start == 3);
    EXPECT_TRUE(function.local_variable_debug[0].length == 74);
}

/**
 * @tc.name: abc2program_hello_world_test_ins_debug
 * @tc.desc: get ins_debug line number and column number
 * @tc.type: FUNC
 * @tc.require: issueI9DT0V
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_ins_debug, TestSize.Level1)
{
    const pandasm::Function &function = *foo_function_;
    size_t regs_num = function.regs_num;
    EXPECT_FALSE(function.local_variable_debug.empty());

    const pandasm::Ins &ins0 = function.ins[0];
    std::string pa_ins_str0 = ins0.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str0 == "mov v0, a0");
    EXPECT_TRUE(ins0.ins_debug.line_number == -1);
    EXPECT_TRUE(ins0.ins_debug.column_number == -1);

    const pandasm::Ins &ins3 = function.ins[3];
    std::string pa_ins_str3 = ins3.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str3 == "ldundefined");
    EXPECT_TRUE(ins3.ins_debug.line_number == 32);
    EXPECT_TRUE(ins3.ins_debug.column_number == 2);

    const pandasm::Ins &ins5 = function.ins[5];
    std::string pa_ins_str5 = ins5.ToString("", true, regs_num);
    EXPECT_TRUE(pa_ins_str5 == "label@5: ldai 0xb");
    EXPECT_TRUE(ins5.ins_debug.line_number == 33);
    EXPECT_TRUE(ins5.ins_debug.column_number == 11);
}

};  // panda::abc2program