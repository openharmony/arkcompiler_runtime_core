/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "generated/inst_props_helpers_dynamic.inc"

namespace libabckit::test::adapter_dynamic {
class InstPropsHelpersDynamicTest : public ::testing::Test {};
typedef AbckitIsaApiDynamicOpcode code;
constexpr size_t VALUE_2 = 2;
TEST_F(InstPropsHelpersDynamicTest, HasMethodIdOperandDynamicTest)
{
    EXPECT_EQ(HasMethodIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC), true);
    EXPECT_EQ(HasMethodIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEMETHOD), true);
    EXPECT_EQ(HasMethodIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER), true);
    EXPECT_EQ(HasMethodIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_DEFINESENDABLECLASS), true);
    EXPECT_EQ(HasMethodIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING), false);
}

TEST_F(InstPropsHelpersDynamicTest, GetMethodIdOperandIndexDynamicTest)
{
    EXPECT_EQ(GetMethodIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC), 1);
    EXPECT_EQ(GetMethodIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEMETHOD), 1);
    EXPECT_EQ(GetMethodIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER), 1);
    EXPECT_EQ(GetMethodIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_DEFINESENDABLECLASS), 1);
    EXPECT_EQ(GetMethodIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING), -1);
}

TEST_F(InstPropsHelpersDynamicTest, HasStringIdOperandDynamicTest)
{
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDA_STR), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYSTGLOBALBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDGLOBALVAR), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDSUPERBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTHISBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STTHISBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STGLOBALVAR), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYNAMEWITHNAMESET), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STSUPERBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDBIGINT), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFIELDBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEPROPERTYBYNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME), true);
    EXPECT_EQ(HasStringIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_NEWSENDABLEENV), false);
}

TEST_F(InstPropsHelpersDynamicTest, GetStringIdOperandIndexDynamicTest)
{
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING), 0);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDA_STR), 0);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYSTGLOBALBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDGLOBALVAR), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDSUPERBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STCONSTTOGLOBALRECORD), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STTOGLOBALRECORD), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTHISBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STTHISBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEREGEXPWITHLITERAL), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STGLOBALVAR), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYNAMEWITHNAMESET), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_STSUPERBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_LDBIGINT), 0);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFIELDBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEPROPERTYBYNAME), 1);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME), 0);
    EXPECT_EQ(GetStringIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_NEWSENDABLEENV), -1);
}

TEST_F(InstPropsHelpersDynamicTest, HasLiteralArrayIdOperandDynamicTest)
{
    EXPECT_EQ(HasLiteralArrayIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER), true);
    EXPECT_EQ(HasLiteralArrayIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER), true);
    EXPECT_EQ(HasLiteralArrayIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER), true);
    EXPECT_EQ(HasLiteralArrayIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWLEXENVWITHNAME), true);
    EXPECT_EQ(HasLiteralArrayIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_NEWLEXENVWITHNAME), true);
    EXPECT_EQ(HasLiteralArrayIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_CREATEPRIVATEPROPERTY),
              true);
    EXPECT_EQ(HasLiteralArrayIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_DEFINESENDABLECLASS),
              true);
    EXPECT_EQ(HasLiteralArrayIdOperandDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_NEWSENDABLEENV), false);
}

TEST_F(InstPropsHelpersDynamicTest, GetLiteralArrayIdOperandIndexDynamicTest)
{
    EXPECT_EQ(GetLiteralArrayIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER), 1);
    EXPECT_EQ(GetLiteralArrayIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER), 1);
    EXPECT_EQ(GetLiteralArrayIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER), VALUE_2);
    EXPECT_EQ(GetLiteralArrayIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWLEXENVWITHNAME), 1);
    EXPECT_EQ(GetLiteralArrayIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_NEWLEXENVWITHNAME), 1);
    EXPECT_EQ(
        GetLiteralArrayIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_CREATEPRIVATEPROPERTY), 1);
    EXPECT_EQ(GetLiteralArrayIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_DEFINESENDABLECLASS),
              VALUE_2);
    EXPECT_EQ(GetLiteralArrayIdOperandIndexDynamic(code::ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_NEWSENDABLEENV), -1);
}
}  // namespace libabckit::test::adapter_dynamic
