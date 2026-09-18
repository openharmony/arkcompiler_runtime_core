/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <fstream>
#include <sstream>
#include <string>

#include "disassembler.h"
#include <gtest/gtest.h>

using namespace testing::ext;
namespace panda::disasm {

static const std::string DEDUP_ABC_PATH = GRAPH_TEST_ABC_DIR "dedup_literal_array.abc";
static const std::string DEDUP_OBJECT_ABC_PATH = GRAPH_TEST_ABC_DIR "dedup_object_buffer.abc";
static const std::string DEDUP_CLASS_ABC_PATH = GRAPH_TEST_ABC_DIR "dedup_class_buffer.abc";

class DisassemblerDedupLiteralArrayTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: disassembler_dedup_literal_array_test_001
 * @tc.desc: Verify that after es2abc --merge-abc merges two source files with identical array
 *           literals ([1, 2, 3]), the disassembled .pa output's LITERALS section contains only
 *           one literal array with that content — no duplicates. DeduplicateLiteralArrays
 *           (runtime_core) shrinks the merged abc.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DisassemblerDedupLiteralArrayTest, disassembler_dedup_literal_array_test_001, TestSize.Level1)
{
    Disassembler disasm;
    bool ok = disasm.Disassemble(DEDUP_ABC_PATH, false, false);
    ASSERT_TRUE(ok);

    std::ostringstream os;
    disasm.Serialize(os, false, false);
    std::string pa = os.str();
    ASSERT_FALSE(pa.empty());

    // Extract the LITERALS section (everything before the first ".language" directive).
    auto langPos = pa.find(".language");
    std::string literalsSection = (langPos != std::string::npos) ? pa.substr(0, langPos) : pa;
    ASSERT_FALSE(literalsSection.empty());

    // Count literal array entries that contain "i32:1" — the first element of [1, 2, 3].
    // After dedup, exactly one entry should contain this content.
    // Without dedup, two identical arrays would produce two entries.
    size_t dupCount = 0;
    std::istringstream litStream(literalsSection);
    std::string line;
    while (std::getline(litStream, line)) {
        if (line.find("i32:1") != std::string::npos && line.find("i32:2") != std::string::npos &&
            line.find("i32:3") != std::string::npos) {
            dupCount++;
        }
    }
    EXPECT_EQ(dupCount, 1U);
}

/**
 * @tc.name: disassembler_dedup_literal_array_test_002
 * @tc.desc: Verify the disassembled .pa matches the expected golden file. The golden file
 *           contains exactly one literal array — if dedup failed, the output would have two.
 *           If the golden file is missing, the test is skipped (it must be generated manually
 *           by running the test locally with a writable source tree).
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DisassemblerDedupLiteralArrayTest, disassembler_dedup_literal_array_test_002, TestSize.Level1)
{
    Disassembler disasm;
    bool ok = disasm.Disassemble(DEDUP_ABC_PATH, false, false);
    ASSERT_TRUE(ok);

    std::ostringstream os;
    disasm.Serialize(os, false, false);
    std::string pa = os.str();
    ASSERT_FALSE(pa.empty());

    // Resolve golden file path relative to this source file.
    std::string srcFile = __FILE__;
    auto slash = srcFile.find_last_of('/');
    std::string goldenDir = (slash != std::string::npos) ? srcFile.substr(0, slash) + "/expected" : "expected";
    std::string goldenPath = goldenDir + "/dedup_literal_array.pa";

    std::ifstream ifs(goldenPath);
    if (!ifs.is_open()) {
        GTEST_SKIP() << "Golden file not found: " << goldenPath << ". Please generate it manually.";
        return;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string expected = ss.str();
    EXPECT_EQ(pa, expected);
}

static size_t CountLiteralEntries(const std::string &pa, const std::string &needle)
{
    auto langPos = pa.find(".language");
    std::string literalsSection = (langPos != std::string::npos) ? pa.substr(0, langPos) : pa;
    size_t count = 0;
    std::istringstream litStream(literalsSection);
    std::string line;
    while (std::getline(litStream, line)) {
        if (line.find(needle) != std::string::npos) {
            count++;
        }
    }
    return count;
}

/**
 * @tc.name: disassembler_dedup_literal_array_test_003
 * @tc.desc: Verify createobjectwithbuffer instruction's literal array is deduplicated across two
 *           merged modules with identical object literals {a:1, b:2, c:3}.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DisassemblerDedupLiteralArrayTest, disassembler_dedup_literal_array_test_003, TestSize.Level1)
{
    Disassembler disasm;
    bool ok = disasm.Disassemble(DEDUP_OBJECT_ABC_PATH, false, false);
    ASSERT_TRUE(ok);

    std::ostringstream os;
    disasm.Serialize(os, false, false);
    std::string pa = os.str();
    ASSERT_FALSE(pa.empty());

    // After dedup, the object buffer literal array should appear only once.
    size_t count = CountLiteralEntries(pa, "i32:1");
    EXPECT_EQ(count, 1U);
}

/**
 * @tc.name: disassembler_dedup_literal_array_test_004
 * @tc.desc: Verify defineclasswithbuffer instruction's class layout buffer is NOT deduplicated
 *           across two merged modules — class layout buffers are mutated at runtime and must stay
 *           independent even when content is identical.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DisassemblerDedupLiteralArrayTest, disassembler_dedup_literal_array_test_004, TestSize.Level1)
{
    Disassembler disasm;
    bool ok = disasm.Disassemble(DEDUP_CLASS_ABC_PATH, false, false);
    ASSERT_TRUE(ok);

    std::ostringstream os;
    disasm.Serialize(os, false, false);
    std::string pa = os.str();
    ASSERT_FALSE(pa.empty());

    // The merged abc should contain defineclasswithbuffer instructions from both modules.
    // Class layout buffers are excluded from dedup, so each module keeps its own copy.
    auto langPos = pa.find(".language");
    std::string literalsSection = (langPos != std::string::npos) ? pa.substr(0, langPos) : pa;
    ASSERT_FALSE(literalsSection.empty());

    // Count class layout buffer entries — they have the form "N 0xNNN { 1 [ i32:0, ]}" on a
    // single line (the property-count + slot layout). Since both modules define classes with
    // identical layouts but class layout buffers are excluded from dedup, there must be at
    // least two such entries (one per module).
    size_t clsBufCount = 0;
    std::istringstream litStream(literalsSection);
    std::string line;
    while (std::getline(litStream, line)) {
        if (line.find("i32:0") != std::string::npos && line.find("{") != std::string::npos) {
            clsBufCount++;
        }
    }
    EXPECT_GE(clsBufCount, 2U);
}

static void CompareWithGolden(const std::string &abcPath, const std::string &goldenName)
{
    Disassembler disasm;
    bool ok = disasm.Disassemble(abcPath, false, false);
    ASSERT_TRUE(ok);

    std::ostringstream os;
    disasm.Serialize(os, false, false);
    std::string pa = os.str();
    ASSERT_FALSE(pa.empty());

    std::string srcFile = __FILE__;
    auto slash = srcFile.find_last_of('/');
    std::string goldenDir = (slash != std::string::npos) ? srcFile.substr(0, slash) + "/expected" : "expected";
    std::string goldenPath = goldenDir + "/" + goldenName;

    std::ifstream ifs(goldenPath);
    if (!ifs.is_open()) {
        GTEST_SKIP() << "Golden file not found: " << goldenPath << ". Please generate it manually.";
        return;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string expected = ss.str();
    EXPECT_EQ(pa, expected);
}

/**
 * @tc.name: disassembler_dedup_literal_array_test_005
 * @tc.desc: Verify the disassembled .pa for createobjectwithbuffer scenario matches the golden
 *           file. The golden file contains the deduplicated literal arrays.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DisassemblerDedupLiteralArrayTest, disassembler_dedup_literal_array_test_005, TestSize.Level1)
{
    CompareWithGolden(DEDUP_OBJECT_ABC_PATH, "dedup_object_buffer.pa");
}

/**
 * @tc.name: disassembler_dedup_literal_array_test_006
 * @tc.desc: Verify the disassembled .pa for defineclasswithbuffer scenario matches the golden
 *           file. Class layout buffers are excluded from dedup, so the golden reflects independent
 *           (non-merged) class layout arrays.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DisassemblerDedupLiteralArrayTest, disassembler_dedup_literal_array_test_006, TestSize.Level1)
{
    CompareWithGolden(DEDUP_CLASS_ABC_PATH, "dedup_class_buffer.pa");
}
}  // namespace panda::disasm
