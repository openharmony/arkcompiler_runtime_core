/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

static inline std::string ExtractRecordBody(const std::string &text, const std::string &header)
{
    auto beg = text.find(header);
    auto end = text.find('}', beg);

    ASSERT(beg != std::string::npos);
    ASSERT(end != std::string::npos);

    return text.substr(beg + header.length(), end - (beg + header.length()));
}

namespace ark::disasm::test {

TEST(RecordTest, EmptyRecord)
{
    auto program = ark::pandasm::Parser().Parse(R"(
.record A {}
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string line;
    std::getline(ss, line);
    EXPECT_EQ(".language PandaAssembly", line);
    std::getline(ss, line);
    std::getline(ss, line);
    EXPECT_EQ(".record A {", line);
    std::getline(ss, line);
    EXPECT_EQ("}", line);
}

static std::string GetRecordWithFieldsSource()
{
    return R"(
.record A {
    u1 a
    i8 b
    u8 c
    i16 d
    u16 e
    i32 f
    u32 g
    f32 h
    f64 i
    i64 j
    u64 k
}
)";
}

TEST(RecordTest, RecordWithFields)
{
    auto program = ark::pandasm::Parser().Parse(GetRecordWithFieldsSource());
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string line;
    std::getline(ss, line);
    EXPECT_EQ(".language PandaAssembly", line);
    std::getline(ss, line);
    std::getline(ss, line);
    EXPECT_EQ(".record A {", line);
    std::getline(ss, line);
    EXPECT_EQ("\tu1 a", line);
    std::getline(ss, line);
    EXPECT_EQ("\ti8 b", line);
    std::getline(ss, line);
    EXPECT_EQ("\tu8 c", line);
    std::getline(ss, line);
    EXPECT_EQ("\ti16 d", line);
    std::getline(ss, line);
    EXPECT_EQ("\tu16 e", line);
    std::getline(ss, line);
    EXPECT_EQ("\ti32 f", line);
    std::getline(ss, line);
    EXPECT_EQ("\tu32 g", line);
    std::getline(ss, line);
    EXPECT_EQ("\tf32 h", line);
    std::getline(ss, line);
    EXPECT_EQ("\tf64 i", line);
    std::getline(ss, line);
    EXPECT_EQ("\ti64 j", line);
    std::getline(ss, line);
    EXPECT_EQ("\tu64 k", line);
}

TEST(RecordTest, RecordWithRecord)
{
    auto program = ark::pandasm::Parser().Parse(R"(
.record A {
    i64 aw
}

.record B {
    A a
    i32 c
}
    )");
    ASSERT(program);
    auto pf = ark::pandasm::AsmEmitter::Emit(program.Value());
    ASSERT(pf);

    ark::disasm::Disassembler d {};
    std::stringstream ss {};

    d.Disassemble(pf);
    d.Serialize(ss);

    std::string bodyA = ExtractRecordBody(ss.str(), ".record A {\n");
    std::stringstream a {bodyA};
    std::string bodyB = ExtractRecordBody(ss.str(), ".record B {\n");
    std::stringstream b {bodyB};

    std::string line;
    std::getline(b, line);
    EXPECT_EQ("\tA a", line);
    std::getline(b, line);
    EXPECT_EQ("\ti32 c", line);
    std::getline(a, line);
    EXPECT_EQ("\ti64 aw", line);
}

#undef DISASM_BIN_DIR

}  // namespace ark::disasm::test
