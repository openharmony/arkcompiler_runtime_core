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

#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "disassembler.h"

using namespace testing::ext;

namespace panda::disasm {

static const std::string MANY_PARAMS_ABC = GRAPH_TEST_ABC_DIR "many-params.abc";
static const std::string NESTED_LITERAL_ABC = GRAPH_TEST_ABC_DIR "declaration-3d-array-number.abc";
static const std::string NONEXISTENT_ABC = GRAPH_TEST_ABC_DIR "nonexistent-file.abc";
static const std::string ANNO3_RECORD = "declaration-3d-array-number.Anno3";

class DisassemblerSecurityTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    static bool Contains(const std::string &text, const std::string &needle)
    {
        return text.find(needle) != std::string::npos;
    }

    static bool Contains(const std::stringstream &ss, const std::string &needle)
    {
        return Contains(ss.str(), needle);
    }
};

/**
 * @tc.name: disassembler_security_test_001
 * @tc.desc: Disassemble returns false when panda file does not exist.
 * @tc.type: FUNC
 */
HWTEST_F(DisassemblerSecurityTest, disassembler_security_test_001, TestSize.Level1)
{
    panda::disasm::Disassembler disasm {};
    EXPECT_FALSE(disasm.Disassemble(NONEXISTENT_ABC, false, false));
}

/**
 * @tc.name: disassembler_security_test_002
 * @tc.desc: CollectInfo and Serialize are safe after failed Disassemble.
 * @tc.type: FUNC
 */
HWTEST_F(DisassemblerSecurityTest, disassembler_security_test_002, TestSize.Level1)
{
    panda::disasm::Disassembler disasm {};
    EXPECT_FALSE(disasm.Disassemble(NONEXISTENT_ABC, false, false));
    disasm.CollectInfo();
    std::stringstream ss;
    disasm.Serialize(ss);
    EXPECT_FALSE(Contains(ss, "manyParams"));
}

/**
 * @tc.name: disassembler_security_test_003
 * @tc.desc: Disassemble and Serialize handle methods with at least 256 parameters.
 * @tc.type: FUNC
 */
HWTEST_F(DisassemblerSecurityTest, disassembler_security_test_003, TestSize.Level1)
{
    panda::disasm::Disassembler disasm {};
    EXPECT_TRUE(disasm.Disassemble(MANY_PARAMS_ABC, false, false));
    std::stringstream ss;
    disasm.Serialize(ss);
    EXPECT_TRUE(Contains(ss, "manyParams"));
    EXPECT_TRUE(Contains(ss, " a255"));
}

/**
 * @tc.name: disassembler_security_test_004
 * @tc.desc: Serialize nested literal arrays without stack overflow.
 * @tc.type: FUNC
 */
HWTEST_F(DisassemblerSecurityTest, disassembler_security_test_004, TestSize.Level1)
{
    panda::disasm::Disassembler disasm {};
    EXPECT_TRUE(disasm.Disassemble(NESTED_LITERAL_ABC, false, false));
    auto record = disasm.GetSerializedRecord(ANNO3_RECORD);
    ASSERT_NE(record, std::nullopt);
    EXPECT_TRUE(Contains(record.value(), "[[[1, -2, 3"));
}

}  // namespace panda::disasm
