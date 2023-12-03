/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "disassembler.h"
#include <gtest/gtest.h>
#include <string>

using namespace testing::ext;
namespace panda::disasm {

static const std::string ANNOTATION_TEST_FILE_NAME_001 = GRAPH_TEST_ABC_DIR "script-string1.abc";
static const std::string ANNOTATION_TEST_FILE_NAME_002 = GRAPH_TEST_ABC_DIR "script-string2.abc";

class DisassemblerStringTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: disassembler_string_test_001
* @tc.desc: get existed string.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerStringTest, disassembler_string_test_001, TestSize.Level1)
{
    static const uint32_t STRING_OFFSET = 0xd2;
    panda::panda_file::File::EntityId STRING_ID(STRING_OFFSET);
    static const std::string EXPECT_STRING = "test";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(ANNOTATION_TEST_FILE_NAME_001, false, false);
    bool is_exists_string = disasm.ValidateStringOffset(STRING_ID, EXPECT_STRING);
    EXPECT_TRUE(is_exists_string);
}

/**
* @tc.name: disassembler_string_test_002
* @tc.desc: get not existed string.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerStringTest, disassembler_string_test_002, TestSize.Level1)
{
    static const uint32_t STRING_OFFSET = 0xba;
    panda::panda_file::File::EntityId STRING_ID(STRING_OFFSET);
    static const std::string EXPECT_STRING = "ClassA";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(ANNOTATION_TEST_FILE_NAME_001, false, false);
    bool is_exists_string = disasm.ValidateStringOffset(STRING_ID, EXPECT_STRING);
    EXPECT_TRUE(is_exists_string);
}

/**
* @tc.name: disassembler_string_test_003
* @tc.desc: get not existed string.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerStringTest, disassembler_string_test_003, TestSize.Level1)
{
    static const uint32_t STRING_OFFSET = 0xbf;
    panda::panda_file::File::EntityId STRING_ID(STRING_OFFSET);
    static const std::string EXPECT_STRING = "Student";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(ANNOTATION_TEST_FILE_NAME_002, false, false);
    bool is_exists_string = disasm.ValidateStringOffset(STRING_ID, EXPECT_STRING);
    EXPECT_TRUE(is_exists_string);
}

/**
* @tc.name: disassembler_string_test_004
* @tc.desc: get not existed string.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerStringTest, disassembler_string_test_004, TestSize.Level1)
{
    static const uint32_t STRING_OFFSET = 0xd9;
    panda::panda_file::File::EntityId STRING_ID(STRING_OFFSET);
    static const std::string EXPECT_STRING = "student_name";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(ANNOTATION_TEST_FILE_NAME_002, false, false);
    bool is_exists_string = disasm.ValidateStringOffset(STRING_ID, EXPECT_STRING);
    EXPECT_TRUE(is_exists_string);
}

};
