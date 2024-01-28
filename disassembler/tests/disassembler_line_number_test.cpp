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

#include <gtest/gtest.h>
#include <string>
#include "disassembler.h"

using namespace testing::ext;

namespace panda::disasm {
class DisasmTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: disassembler_line_number_test_001
* @tc.desc: Check abc file line number function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisasmTest, disassembler_line_number_test_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "line-number1.abc";
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(file_name, false, false);
    disasm.CollectInfo();
    // The known line number in the abc file
    std::vector<size_t> expectedLineNumber = {-1, 15, -1, 15};
    std::vector<size_t> lineNumber = disasm.GetLineNumber();
    EXPECT_TRUE(expectedLineNumber.size() == lineNumber.size());
    bool res = true;
    for (size_t i = 0; i < lineNumber.size(); ++i) {
        if (expectedLineNumber[i] != lineNumber[i]) {
            res = false;
            break;
        }
    }
    EXPECT_TRUE(res);
}
}