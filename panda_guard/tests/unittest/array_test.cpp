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

#include <gtest/gtest.h>
#include <iterator>

#include "configs/guard_context.h"
#include "obfuscate/program.h"

using namespace testing::ext;

namespace panda::guard {
namespace {
constexpr auto LANG = panda_file::SourceLang::ECMASCRIPT;
const std::string CONFIG = PANDA_GUARD_UNIT_TEST_DIR "configs/array_test_config.json";

void AddRecord(pandasm::Program &program, const std::string &name)
{
    pandasm::Record record(name, LANG);
    pandasm::Field package(LANG);
    package.name = "pkgName@test";
    record.field_list.push_back(std::move(package));
    program.record_table.emplace(name, std::move(record));
}

void AddLiteral(pandasm::Program &program, const std::string &id, uint32_t value)
{
    pandasm::LiteralArray literalArray;
    pandasm::LiteralArray::Literal literal;
    literal.tag_ = panda_file::LiteralTag::INTEGER;
    literal.value_ = value;
    literalArray.literals_.push_back(literal);
    program.literalarray_table.emplace(id, std::move(literalArray));
}

pandasm::InsPtr MakeInstruction(pandasm::Opcode opcode, std::vector<pandasm::IType> imms = {},
                                std::vector<std::string> ids = {})
{
    return pandasm::InsPtr(pandasm::Ins::CreateIns(opcode, std::vector<uint16_t> {}, std::move(imms), std::move(ids)));
}

void AddFunction(pandasm::Program &program, const std::string &name, const std::vector<std::string> &literalIds)
{
    pandasm::Function function(name, LANG);
    for (const auto &id : literalIds) {
        function.ins.push_back(MakeInstruction(pandasm::Opcode::CREATEARRAYWITHBUFFER, {int64_t {0}}, {id}));
    }
    function.ins.push_back(MakeInstruction(pandasm::Opcode::RETURNUNDEFINED));
    program.function_table.emplace(name, std::move(function));
}

void ExpectLiteral(const pandasm::Program &program, const std::string &id, uint32_t value)
{
    const auto it = program.literalarray_table.find(id);
    ASSERT_NE(it, program.literalarray_table.end());
    ASSERT_EQ(it->second.literals_.size(), 1U);
    EXPECT_EQ(it->second.literals_[0].tag_, panda_file::LiteralTag::INTEGER);
    EXPECT_EQ(std::get<uint32_t>(it->second.literals_[0].value_), value);
}

class LiteralArrayRenamer : public Entity {
public:
    using Entity::Entity;
    using Entity::UpdateLiteralArrayTableIdx;
};
}  // namespace

class ArrayTest : public testing::Test {
protected:
    void SetUp() override
    {
        const char *argv[] = {"panda_guard", CONFIG.c_str()};
        GuardContext::GetInstance()->Init(static_cast<int>(std::size(argv)), argv);
    }
};

// Multiple instructions sharing a literal must all follow its single table rename.
HWTEST_F(ArrayTest, shared_literal_in_one_function, TestSize.Level1)
{
    pandasm::Program program;
    AddRecord(program, "arrays");
    AddLiteral(program, "arrays_100", 1);
    AddLiteral(program, "arrays_200", 2);
    AddFunction(program, "arrays.func_main_0", {"arrays_100", "arrays_100", "arrays_200", "arrays_100"});

    Program guard(&program);
    guard.Create();
    guard.Obfuscate();

    const auto &newName = guard.nodeTable_.at("arrays")->obfName_;
    ASSERT_NE(newName, "arrays");
    const auto &ins = program.function_table.at(newName + ".func_main_0").ins;
    EXPECT_EQ(ins[0]->GetId(0), newName + "_100");
    EXPECT_EQ(ins[1]->GetId(0), newName + "_100");
    EXPECT_EQ(ins[2]->GetId(0), newName + "_200");
    EXPECT_EQ(ins[3]->GetId(0), newName + "_100");
    EXPECT_EQ(program.literalarray_table.size(), 2U);
    EXPECT_EQ(program.literalarray_table.count("arrays_100"), 0U);
    EXPECT_EQ(program.literalarray_table.count("arrays_200"), 0U);
    ExpectLiteral(program, newName + "_100", 1);
    ExpectLiteral(program, newName + "_200", 2);
}

HWTEST_F(ArrayTest, shared_literal_across_functions, TestSize.Level1)
{
    pandasm::Program program;
    AddRecord(program, "arrays");
    AddLiteral(program, "arrays_100", 1);
    AddFunction(program, "arrays.func_main_0", {"arrays_100"});
    AddFunction(program, "arrays.#*#helper", {"arrays_100", "arrays_100"});
    auto &entry = program.function_table.at("arrays.func_main_0");
    entry.ins.insert(entry.ins.begin(),
                     MakeInstruction(pandasm::Opcode::DEFINEFUNC, {int64_t {0}, int64_t {0}}, {"arrays.#*#helper"}));

    Program guard(&program);
    guard.Create();
    guard.Obfuscate();

    const auto &newName = guard.nodeTable_.at("arrays")->obfName_;
    ASSERT_NE(newName, "arrays");
    const auto &entryIns = program.function_table.at(newName + ".func_main_0").ins;
    const auto &helperIns = program.function_table.at(newName + ".#*#helper").ins;
    EXPECT_EQ(entryIns[0]->GetId(0), newName + ".#*#helper");
    EXPECT_EQ(entryIns[1]->GetId(0), newName + "_100");
    EXPECT_EQ(helperIns[0]->GetId(0), newName + "_100");
    EXPECT_EQ(helperIns[1]->GetId(0), newName + "_100");
    EXPECT_EQ(program.literalarray_table.size(), 1U);
    ExpectLiteral(program, newName + "_100", 1);
}

// abc2program qualifies a shared bytecode offset with each referencing record's name.
HWTEST_F(ArrayTest, same_literal_offset_in_different_records, TestSize.Level1)
{
    pandasm::Program program;
    for (const std::string name : {"first", "second"}) {
        AddRecord(program, name);
        AddLiteral(program, name + "_100", 1);
        AddFunction(program, name + ".func_main_0", {name + "_100", name + "_100"});
    }

    Program guard(&program);
    guard.Create();
    guard.Obfuscate();

    EXPECT_EQ(program.literalarray_table.size(), 2U);
    EXPECT_NE(guard.nodeTable_.at("first")->obfName_, guard.nodeTable_.at("second")->obfName_);
    for (const std::string name : {"first", "second"}) {
        const auto &newName = guard.nodeTable_.at(name)->obfName_;
        ASSERT_NE(newName, name);
        const auto &ins = program.function_table.at(newName + ".func_main_0").ins;
        EXPECT_EQ(ins[0]->GetId(0), newName + "_100");
        EXPECT_EQ(ins[1]->GetId(0), newName + "_100");
        EXPECT_EQ(program.literalarray_table.count(name + "_100"), 0U);
        ExpectLiteral(program, newName + "_100", 1);
    }
}

HWTEST_F(ArrayTest, reserved_file_name_keeps_shared_literal, TestSize.Level1)
{
    pandasm::Program program;
    AddRecord(program, "kept");
    AddLiteral(program, "kept_100", 1);
    AddFunction(program, "kept.func_main_0", {"kept_100", "kept_100"});

    Program guard(&program);
    guard.Create();
    guard.Obfuscate();

    EXPECT_EQ(guard.nodeTable_.at("kept")->obfName_, "kept");
    const auto &ins = program.function_table.at("kept.func_main_0").ins;
    EXPECT_EQ(ins[0]->GetId(0), "kept_100");
    EXPECT_EQ(ins[1]->GetId(0), "kept_100");
    EXPECT_EQ(program.literalarray_table.size(), 1U);
    ExpectLiteral(program, "kept_100", 1);
}

HWTEST_F(ArrayTest, missing_literal_reports_error, TestSize.Level1)
{
    pandasm::Program program;
    Program guard(&program);
    LiteralArrayRenamer renamer(&guard);
    EXPECT_DEATH(renamer.UpdateLiteralArrayTableIdx("missing_100", "new_100"), "literal array not found: missing_100");
}

HWTEST_F(ArrayTest, literal_name_collision_reports_error, TestSize.Level1)
{
    pandasm::Program program;
    AddLiteral(program, "old_100", 1);
    AddLiteral(program, "new_100", 2);
    Program guard(&program);
    LiteralArrayRenamer renamer(&guard);
    EXPECT_DEATH(renamer.UpdateLiteralArrayTableIdx("old_100", "new_100"),
                 "literal array target already exists: new_100");
}
}  // namespace panda::guard
