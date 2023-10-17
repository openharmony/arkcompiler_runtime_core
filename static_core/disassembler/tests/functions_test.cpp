/**
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <iostream>
#include <string>

#include <gtest/gtest.h>
#include "assembly-parser.h"
#include "disassembler.h"

static inline std::string ExtractFuncBody(const std::string &text, const std::string &header)
{
    auto beg = text.find(header);
    auto end = text.find('}', beg);

    assert(beg != std::string::npos);
    assert(end != std::string::npos);

    return text.substr(beg + header.length(), end - (beg + header.length()));
}

TEST(FunctionsTest, EmptyFunction)
{
    auto program = panda::pandasm::Parser().Parse(R"(
        .function void A(i32 a0) {}
    )");
    ASSERT(program);
    auto pf = panda::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    panda::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string line;
    std::getline(ss, line);
    ASSERT_EQ(line, ".language PandaAssembly");
    std::getline(ss, line);
    ASSERT_EQ(line, "");
    std::getline(ss, line);
    ASSERT_EQ(line, ".function void A(i32 a0) <static> {");
    std::getline(ss, line);
    ASSERT_EQ(line, "}");
}

TEST(FunctionsTest, OverloadingTest)
{
    auto program = panda::pandasm::Parser().Parse(R"(
        .function void f() {}

        .function void f(u1 a0) {}
        
        .function void f(u1 a0, i8 a1) {}
        
        .function void main() {
            call f:()
            call f:(u1), v1
            call f:(u1, i8), v1, v1
        }
    )");
    ASSERT(program);
    auto pf = panda::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    panda::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string body_main = ExtractFuncBody(ss.str(), "main() <static> {\n");

    std::string line;
    std::stringstream main {body_main};
    std::getline(main, line);
    EXPECT_EQ("\tcall.short f:()", line);
    std::getline(main, line);
    EXPECT_EQ("\tcall.short f:(u1), v1", line);
    std::getline(main, line);
    EXPECT_EQ("\tcall.short f:(u1,i8), v1, v1", line);

    EXPECT_TRUE(ss.str().find(".function void f() <static> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void f(u1 a0) <static> {") != std::string::npos);
    EXPECT_TRUE(ss.str().find(".function void f(u1 a0, i8 a1) <static> {") != std::string::npos);
}

static std::pair<std::unique_ptr<const panda::panda_file::File>, panda::panda_file::File::EntityId> BuildPandaMethod()
{
    auto program = panda::pandasm::Parser().Parse(R"(
        .function void func() {

            nop

        label:

            nop

            jmp label

            return

        }
    )");
    ASSERT(program);

    auto pf = panda::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    panda::panda_file::DebugInfoExtractor debug_info(pf.get());
    auto method_ids = debug_info.GetMethodIdList();
    ASSERT(method_ids.size() == 1);

    return std::make_pair(std::move(pf), method_ids[0]);
}

TEST(FunctionsTest, SerializeText)
{
    auto [pf, method_id] = BuildPandaMethod();

    panda::disasm::Disassembler d {};
    d.SetFile(*pf);

    panda::pandasm::Function method("", panda::SourceLanguage::PANDA_ASSEMBLY);
    d.GetMethod(&method, method_id);

    std::stringstream ss {};
    d.Serialize(method, ss);

    std::string line;
    std::getline(ss, line);
    ASSERT_EQ(line, ".function void func() <static> {");
    std::getline(ss, line);
    ASSERT_EQ(line, "\tnop");
    std::getline(ss, line);
    ASSERT_EQ(line, "jump_label_0:");
    std::getline(ss, line);
    ASSERT_EQ(line, "\tnop");
    std::getline(ss, line);
    ASSERT_EQ(line, "\tjmp jump_label_0");
    std::getline(ss, line);
    ASSERT_EQ(line, "\treturn");
    std::getline(ss, line);
    ASSERT_EQ(line, "}");
}

TEST(FunctionsTest, SerializeLineTable)
{
    auto [pf, method_id] = BuildPandaMethod();

    panda::disasm::Disassembler d {};
    d.SetFile(*pf);

    panda::pandasm::Function method("", panda::SourceLanguage::PANDA_ASSEMBLY);
    d.GetMethod(&method, method_id);

    std::stringstream ss {};
    panda::panda_file::LineNumberTable line_table;
    d.Serialize(method, ss, false, &line_table);

    ASSERT_EQ(line_table.size(), 4);

    ASSERT_EQ(line_table[0].offset, 0);
    ASSERT_EQ(line_table[0].line, 2);

    ASSERT_EQ(line_table[1].offset, 1);
    ASSERT_EQ(line_table[1].line, 4);

    ASSERT_EQ(line_table[2].offset, 2);
    ASSERT_EQ(line_table[2].line, 5);

    ASSERT_EQ(line_table[3].offset, 4);
    ASSERT_EQ(line_table[3].line, 6);
}

#undef DISASM_BIN_DIR
