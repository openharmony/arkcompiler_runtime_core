/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "assembly-ins.h"
#include "assembly-parser.h"
#include "ide_helpers.h"
#include "ins_to_string.cpp"
#include "operand_types_print.h"
#include "extensions/ecmascript_meta.h"
#include "meta.h"

using namespace testing::ext;

namespace panda::pandasm {
class AssemblerInsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AssemblerInsTest::SetUpTestCase(void)
{
}

void AssemblerInsTest::TearDownTestCase(void)
{
}

void AssemblerInsTest::SetUp(void)
{
}

void AssemblerInsTest::TearDown(void)
{
}

/**
 * @tc.name: assembler_ins_test_001
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblerInsTest, assembler_ins_test_001, TestSize.Level1)
{
    Parser p;
    const auto source = R"(
        .function any foo(){}
        .function any func(any a0, any a1, any a2) <static> {
            mov v0, a0
            mov v1, a1
            mov v2, a2
            ldglobalvar 0x0, "foo"
            sta v4
            lda v4
            callarg0 0x1
            return
        }
    )";
    auto item = p.Parse(source);
    const std::string func_name = "func:(any,any,any)";
    auto it = item.Value().function_table.find(func_name);
    EXPECT_NE(it, item.Value().function_table.end());
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[3].OperandListLength(), 2ULL);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[3].HasFlag(InstFlags::TYPE_ID), false);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[3].CanThrow(), true);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[3].IsJump(), false);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[3].IsConditionalJump(), false);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[3].IsCall(), false);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[3].IsCallRange(), false);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[3].IsPseudoCall(), false);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[7].IsReturn(), true);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[7].MaxRegEncodingWidth(), 0);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[7].HasDebugInfo(), true);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[7].Uses().size(), 6);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[7].Def(), std::nullopt);
    EXPECT_EQ(item.Value().function_table.at(func_name).ins[7].IsValidToEmit(), true);
    EXPECT_EQ(item.Value().JsonDump().size(), 280);
    EXPECT_EQ(p.ShowError().err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: assembler_ins_test_002
 * @tc.desc: Verify the ToString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblerInsTest, assembler_ins_test_002, TestSize.Level1)
{
    Parser p;
    const auto source = R"(
        .function any func() {
            sta v4
            lda.str "xxx"
            ldglobalvar 0x7, "oDiv"
            callarg1 0x1, v0
            return
        }
    )";
    auto item = p.Parse(source);
    const std::string func_name = "func:()";
    auto it = item.Value().function_table.find(func_name);
    EXPECT_NE(it, item.Value().function_table.end());
    std::string ret = item.Value().function_table.at(func_name).ins[0].ToString("test", true, 0);
    EXPECT_EQ(ret, "sta a4test");
    ret = item.Value().function_table.at(func_name).ins[0].ToString("test", false, 0);
    EXPECT_EQ(ret, "sta v4test");

    ret = item.Value().function_table.at(func_name).ins[1].ToString("test", true, 0);
    EXPECT_EQ(ret, "lda.str xxxtest");
    ret = item.Value().function_table.at(func_name).ins[1].ToString("test", false, 0);
    EXPECT_EQ(ret, "lda.str xxxtest");

    ret = item.Value().function_table.at(func_name).ins[2].ToString("test", true, 0);
    EXPECT_EQ(ret, "ldglobalvar 0x7, oDivtest");
    ret = item.Value().function_table.at(func_name).ins[2].ToString("test", false, 0);
    EXPECT_EQ(ret, "ldglobalvar 0x7, oDivtest");

    panda::pandasm::Ins ins;
    ins.opcode = Opcode::DEPRECATED_LDMODULEVAR;
    ins.regs.push_back(2U);
    ins.regs.push_back(3U);
    ins.imms.push_back(Ins::IType(int64_t(0x1)));
    ins.ids.push_back("a1");
    ins.set_label = false;
    ins.label = "label";

    ret = ins.ToString("test", true, 0);
    EXPECT_EQ(ret, "deprecated.ldmodulevar a1, 0x1test");
    ret = ins.ToString("test", false, 0);
    EXPECT_EQ(ret, "deprecated.ldmodulevar a1, 0x1test");

    ins.opcode = Opcode::MOVX;
    ret = ins.ToString("test", true, 0);
    EXPECT_EQ(ret, "MOVX a2, a3, 0x1, a1test");
    ret = ins.ToString("test", false, 0);
    EXPECT_EQ(ret, "MOVX v2, v3, 0x1, a1test");

    ins.opcode = Opcode::DEFINEFUNC;
    ret = ins.ToString("test", true, 0);
    EXPECT_EQ(ret, "definefunc 0x1, a1test");
    ret = ins.ToString("test", false, 0);
    EXPECT_EQ(ret, "definefunc 0x1, a1test");
    EXPECT_TRUE(ins.CanThrow());

    ins.opcode = Opcode::JEQZ;
    EXPECT_TRUE(ins.IsConditionalJump());

    ins.opcode = Opcode::SUPERCALLARROWRANGE;
    ret = ins.ToString("test", true, 0);
    EXPECT_EQ(ret, "supercallarrowrange 0x1, a2test");
    ret = ins.ToString("test", false, 0);
    EXPECT_EQ(ret, "supercallarrowrange 0x1, v2test");

    ins.opcode = Opcode::NEWOBJRANGE;
    ret = ins.ToString("test", true, 0);
    EXPECT_EQ(ret, "newobjrange 0x1, a2test");
    ret = ins.ToString("test", false, 0);
    EXPECT_EQ(ret, "newobjrange 0x1, v2test");

    ins.imms.clear();
    ins.opcode = Opcode::DEFINECLASSWITHBUFFER;
    ret = ins.ToString("test", true, 0);
    EXPECT_EQ(ret, "defineclasswithbuffer, a1, a2test");
    ret = ins.ToString("test", false, 0);
    EXPECT_EQ(ret, "defineclasswithbuffer, a1, v2test");

    ins.opcode = Opcode::CALLX;
    auto unit = ins.Uses();
    EXPECT_GT(unit.size(), 0);

    ins.regs.clear();
    ins.opcode = Opcode::STOBJBYVALUE;
    ret = ins.ToString("test", true, 0);
    EXPECT_EQ(ret, "stobjbyvaluetest");
    ret = ins.ToString("test", false, 0);
    EXPECT_EQ(ret, "stobjbyvaluetest");

    EXPECT_EQ(ins.Def(), std::nullopt);

    ins.opcode = Opcode::INVALID;
    ret = ins.MaxRegEncodingWidth();
    auto unit1 = ins.Uses();
    EXPECT_EQ(unit1.size(), 0);

    EXPECT_EQ(ins.Def(), std::nullopt);

    ins.regs.push_back(100);
    EXPECT_FALSE(ins.IsValidToEmit());

    panda::pandasm::Program pro;
    ret = pro.JsonDump();
    EXPECT_EQ(ret, "{ \"functions\": [  ], \"records\": [  ] }");

    std::string_view component_name = "u8";
    size_t rank = 0;
    panda::pandasm::Type type(component_name, rank);
    bool bo = type.IsArrayContainsPrimTypes();
    EXPECT_TRUE(bo);

    ret = type.GetDescriptor(false);
    EXPECT_EQ(ret, "H");

    EXPECT_EQ(panda::pandasm::Type::GetId(component_name, true), panda_file::Type::TypeId::REFERENCE);
    component_name = "[]";
    EXPECT_EQ(panda::pandasm::Type::FromDescriptor(component_name).GetName(), component_name);
    EXPECT_EQ(panda::pandasm::Type::FromName(component_name, false).GetName(), component_name);
    panda::panda_file::SourceLang language {panda::panda_file::SourceLang::PANDA_ASSEMBLY};

    std::string name = "test";
    EXPECT_FALSE(panda::pandasm::Type::IsStringType(name, language));
    ins.opcode = Opcode::CALLRANGE;
    EXPECT_EQ(ins.Def(), 65535);
    auto unit2 = ins.Uses();
    EXPECT_GT(unit2.size(), 0);

    component_name = "test";
    rank = 0;
    panda::pandasm::Type type1(component_name, rank);
    bo = type1.IsArrayContainsPrimTypes();
    EXPECT_FALSE(bo);

    panda::panda_file::SourceLang language1 {panda::panda_file::SourceLang::PANDA_ASSEMBLY};
    panda::pandasm::Function function("fun", language1);
    function.file_location->is_defined = false;
    panda::pandasm::Function function1("fun", language1, 0, 10, "func", false, 10);

    ret = JsonSerializeItemBody(function);
    EXPECT_EQ(ret, "{ \"name\": \"fun\" }");

    std::map<std::string, panda::pandasm::Function> function_table;
    ret = JsonSerializeProgramItems(function_table);
    EXPECT_EQ(ret, "[  ]");

    std::string test = "test";
    function_table.emplace(test, std::move(function1));

    ret = JsonSerializeProgramItems(function_table);
    EXPECT_EQ(ret, "[ { \"name\": \"fun\" } ]");

    ret = item.Value().function_table.at(func_name).ins[3].ToString("test", true, 0);
    EXPECT_EQ(ret, "callarg1 0x1, a0test");
    ret = item.Value().function_table.at(func_name).ins[3].ToString("test", false, 0);
    EXPECT_EQ(ret, "callarg1 0x1, v0test");
}

/**
 * @tc.name: assembler_ins_test_003
 * @tc.desc: Verify the ToString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(AssemblerInsTest, assembler_ins_test_003, TestSize.Level1)
{
    Context con;
    con.token = "test";

    EXPECT_GT(con.Len(), 0);
    EXPECT_FALSE(con.ValidateParameterName(65536));
    EXPECT_FALSE(con.ValidateParameterName(4));

    panda::pandasm::Token token1;
    con.tokens.push_back(token1);
    panda::pandasm::Token token2;
    con.tokens.push_back(token2);
    EXPECT_EQ(con.Next(), Token::Type::ID_BAD);
    con.number = 3;
    con.end = false;
    EXPECT_TRUE(con.NextMask());
    con.end = true;
    EXPECT_TRUE(con.NextMask());
}
}