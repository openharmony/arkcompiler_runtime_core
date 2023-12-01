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

static const std::string MODULE_REQUEST_FILE_NAME = GRAPH_TEST_ABC_DIR "module-requests-annotation-import.abc";
static const std::string SLOT_NUMBER_FILE_NAME = GRAPH_TEST_ABC_DIR "slot-number-annotation.abc";

class DisassemblerAnnotationTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: disassembler_annotation_test_001
* @tc.desc: get module request annotation of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerAnnotationTest, disassembler_annotation_test_001, TestSize.Level1)
{
    static const std::string METHOD_NAME = "module-requests-annotation-import.funcD";
    static const std::string ANNOTATION_NAME = "L_ESConcurrentModuleRequestsAnnotation";
    static const uint32_t EXPECT_REQUEST_INDEX = 0x0;
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(MODULE_REQUEST_FILE_NAME, false, false);
    std::optional<uint32_t> request_index = disasm.GetMethodAnnotationByName(METHOD_NAME, ANNOTATION_NAME);
    ASSERT_NE(request_index, std::nullopt);
    EXPECT_EQ(EXPECT_REQUEST_INDEX, request_index.value());
}

/**
* @tc.name: disassembler_annotation_test_002
* @tc.desc: get solt number annotation of abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(DisassemblerAnnotationTest, disassembler_annotation_test_002, TestSize.Level1)
{
    static const std::string METHOD_NAME = "funcA";
    static const std::string ANNOTATION_NAME = "L_ESSlotNumberAnnotation";
    static const uint32_t EXPECT_SLOT_NUMBER = 0x3;
    panda::disasm::Disassembler disasm {};
    disasm.Disassemble(SLOT_NUMBER_FILE_NAME, false, false);
    std::optional<uint32_t> slot_number = disasm.GetMethodAnnotationByName(METHOD_NAME, ANNOTATION_NAME);
    ASSERT_NE(slot_number, std::nullopt);
    EXPECT_EQ(EXPECT_SLOT_NUMBER, slot_number.value());
}
};
