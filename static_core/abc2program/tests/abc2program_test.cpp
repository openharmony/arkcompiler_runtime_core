/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "abc2program_driver.h"

namespace ark::abc2program {

constexpr std::string_view HELLO_WORLD_ABC_TEST_FILE_NAME = "sts/HelloWorld.abc";
constexpr std::string_view FUNCTIONS_ABC_TEST_FILE_NAME = "sts/Functions.abc";

class Abc2ProgramTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void TearDown() override {}

    template <typename T>
    bool ValidateString(const T &strings, const T &expectedStrings)
    {
        if (strings.size() != expectedStrings.size()) {
            return false;
        }
        for (const std::string &expectedString : expectedStrings) {
            const auto stringIter = std::find(strings.begin(), strings.end(), expectedString);
            if (stringIter == strings.end()) {
                return false;
            }
        }
        return true;
    }

    Abc2ProgramDriver driver;                // NOLINT(misc-non-private-member-variables-in-classes)
    const pandasm::Program *prog = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

class Abc2ProgramHelloWorldTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(HELLO_WORLD_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramFunctionsTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(FUNCTIONS_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

TEST_F(Abc2ProgramHelloWorldTest, Lang)
{
    panda_file::SourceLang expectedLang = panda_file::SourceLang::ETS;
    bool langMatched = (prog->lang == expectedLang);
    EXPECT_TRUE(langMatched);
}

TEST_F(Abc2ProgramHelloWorldTest, RecordTable)
{
    std::vector<std::string> expectedRecordNames = {"HelloWorld", "ETSGLOBAL"};
    panda_file::SourceLang expectedLang = panda_file::SourceLang::ETS;
    std::vector<std::string> recordNames;
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.second.language == expectedLang);
        EXPECT_TRUE(it.first == it.second.name);
        recordNames.emplace_back(it.first);
    }
    ValidateString(recordNames, expectedRecordNames);
}

TEST_F(Abc2ProgramHelloWorldTest, Functions)
{
    std::set<std::string> expectedFunctions = {"ETSGLOBAL._$init$_:void;",
                                               "ETSGLOBAL._$trigger_cctor$_:void;",
                                               "ETSGLOBAL._cctor_:void;",
                                               "ETSGLOBAL.main:void;",
                                               "HelloWorld._ctor_:HelloWorld;void;",
                                               "HelloWorld.bar:HelloWorld;std.core.Object;std.core.Object;",
                                               "HelloWorld.foo:HelloWorld;i32;i32;",
                                               "std.core.Console.log:std.core.Console;i32;void;",
                                               "std.core.Object._ctor_:std.core.Object;void;"};
    std::set<std::string> existingFunctions {};
    for (auto &it : prog->functionTable) {
        existingFunctions.insert(it.first);
    }
    bool result = ValidateString(existingFunctions, expectedFunctions);
    EXPECT_TRUE(result);
}

TEST_F(Abc2ProgramHelloWorldTest, Fields)
{
    for (const auto &it : prog->recordTable) {
        if (it.first == "HelloWorld") {
            EXPECT_TRUE(it.second.fieldList.size() == 2U);
        }
    }
}

TEST_F(Abc2ProgramHelloWorldTest, StringTable)
{
    std::set<std::string> expectedStrings = {R"(HelloWorld)", R"(NewLine\n)"};
    bool stringMatched = ValidateString(prog->strings, expectedStrings);
    EXPECT_TRUE(stringMatched);
}

TEST_F(Abc2ProgramFunctionsTest, Lang)
{
    panda_file::SourceLang expectedLang = panda_file::SourceLang::ETS;
    bool langMatched = (prog->lang == expectedLang);
    EXPECT_TRUE(langMatched);
}

TEST_F(Abc2ProgramFunctionsTest, RecordTable)
{
    std::vector<std::string> expectedRecordNames = {"ETSGLOBAL"};
    panda_file::SourceLang expectedLang = panda_file::SourceLang::ETS;
    std::vector<std::string> recordNames;
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.second.language == expectedLang);
        EXPECT_TRUE(it.first == it.second.name);
        recordNames.emplace_back(it.first);
    }
    ValidateString(recordNames, expectedRecordNames);
}

TEST_F(Abc2ProgramFunctionsTest, Functions)
{
    std::set<std::string> expectedFunctions = {
        "ETSGLOBAL._$init$_:void;",
        "ETSGLOBAL._$trigger_cctor$_:void;",
        "ETSGLOBAL._cctor_:void;",
        "ETSGLOBAL.bar:f64;std.core.String;",
        "ETSGLOBAL.foo:std.core.String;std.core.String;",
        "std.core.StringBuilder._ctor_:std.core.StringBuilder;void;",
        "std.core.StringBuilder.append:std.core.StringBuilder;f64;std.core.StringBuilder;",
        "std.core.StringBuilder.append:std.core.StringBuilder;std.core.String;std.core.StringBuilder;",
        "std.core.StringBuilder.toString:std.core.StringBuilder;std.core.String;"};
    std::set<std::string> existingFunctions {};
    for (auto &it : prog->functionTable) {
        existingFunctions.insert(it.first);
    }
    bool result = ValidateString(existingFunctions, expectedFunctions);
    EXPECT_TRUE(result);
}

TEST_F(Abc2ProgramFunctionsTest, StringTable)
{
    std::set<std::string> expectedStrings = {"Function foo was called", "Function bar was called"};
    bool result = ValidateString(prog->strings, expectedStrings);
    EXPECT_TRUE(result);
}

}  // namespace ark::abc2program
