/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <regex>
#include <string>

#include <gtest/gtest.h>
#include "disassembler.h"
#include "assembly-parser.h"

static inline std::string ExtractFuncBody(const std::string &text, const std::string &header)
{
    auto beg = text.find(header);
    auto end = text.find('}', beg);

    assert(beg != std::string::npos);
    assert(end != std::string::npos);

    return text.substr(beg + header.length(), end - (beg + header.length()));
}

TEST(TestDebugInfo, TestDebugInfo)
{
    auto program = panda::pandasm::Parser().Parse(R"(
.record A {
    u1 fld
}

.function A A.init() {
    newobj v0, A
    lda.obj v0

    return.obj
}

.function u1 g() {
    mov v0, v1
    mov.64 v2, v3
    mov.obj v4, v5

    movi v0, -1
    movi.64 v0, 2
    fmovi.64 v0, 3.01

    lda v1
    lda.64 v0
    lda.obj v1
}
    )");
    ASSERT(program);
    auto pf = panda::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    panda::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.CollectInfo();
    d.Serialize(ss, true, true);

    std::string body_g = ExtractFuncBody(ss.str(), "g() <static> {");

    ASSERT_NE(body_g.find("#   LINE_NUMBER_TABLE:"), std::string::npos);
    ASSERT_NE(body_g.find("#\tline 14: 0\n"), std::string::npos);

    size_t code_start = body_g.find("#   CODE:\n");
    ASSERT_NE(code_start, std::string::npos) << "Code section in function g not found";
    size_t code_end = body_g.find("\n\n");  // First gap in function body is code section end
    ASSERT_NE(code_end, std::string::npos) << "Gap after code section in function g not found";
    ASSERT_LT(code_start, code_end);
    std::string instructions =
        body_g.substr(code_start + strlen("#   CODE:\n"), code_end + 1 - (code_start + strlen("#   CODE:\n")));

    std::regex const inst_regex("# offset: ");
    std::ptrdiff_t const instruction_count(std::distance(
        std::sregex_iterator(instructions.begin(), instructions.end(), inst_regex), std::sregex_iterator()));

    const panda::disasm::ProgInfo &prog_info = d.GetProgInfo();
    auto g_it = prog_info.methods_info.find("g:()");
    ASSERT_NE(g_it, prog_info.methods_info.end());
    // In case of pandasm the table should contain entry on each instruction
    ASSERT_EQ(g_it->second.line_number_table.size(), instruction_count);

    // There should be no local variables for panda assembler
    ASSERT_EQ(body_g.find("#   LOCAL_VARIABLE_TABLE:"), std::string::npos);
}
