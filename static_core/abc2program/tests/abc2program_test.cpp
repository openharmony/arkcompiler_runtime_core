/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <sstream>
#include <vector>
#include "abc2program_driver.h"

namespace ark::abc2program {

constexpr std::string_view FIELD_ABC_TEST_FILE_NAME = "ets/Field.abc";
constexpr std::string_view FUNCTIONS_ABC_TEST_FILE_NAME = "ets/Functions.abc";
constexpr std::string_view HELLO_WORLD_ABC_TEST_FILE_NAME = "ets/HelloWorld.abc";
constexpr std::string_view INS_ABC_TEST_FILE_NAME = "ets/Ins.abc";
constexpr std::string_view LITERAL_ARRAY_ABC_TEST_FILE_NAME = "ets/LiteralArray.abc";
constexpr std::string_view METADATA_ABC_TEST_FILE_NAME = "ets/Metadata.abc";
constexpr std::string_view RECORD_ABC_TEST_FILE_NAME = "ets/Record.abc";
constexpr std::string_view TYPE_ABC_TEST_FILE_NAME = "ets/Type.abc";
constexpr size_t ANNOTATION_ID_SIZE = 1;
constexpr size_t ANNOTATION_ID_PREFIX_SIZE = 3;

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

    template <typename T>
    bool ContainString(const T &strings, const T &expectedStrings)
    {
        for (const std::string &expectedString : expectedStrings) {
            const auto stringIter = std::find(strings.begin(), strings.end(), expectedString);
            if (stringIter == strings.end()) {
                return false;
            }
        }
        return true;
    }

    template <typename T>
    bool ValidateVector(const T &vec, const T &expectedVec)
    {
        if (vec.size() != expectedVec.size()) {
            return false;
        }
        for (size_t i = 0; i < vec.size(); i++) {
            if (vec[i] != expectedVec[i]) {
                return false;
            }
        }
        return true;
    }

    bool ValidateAnnotationId(const std::vector<std::string> &id)
    {
        if (id.size() != ANNOTATION_ID_SIZE) {
            return false;
        }
        return id[0].size() > ANNOTATION_ID_PREFIX_SIZE && id[0].substr(0, ANNOTATION_ID_PREFIX_SIZE) == "id_";
    }

    bool ValidateBoolAttributes(const std::unordered_set<std::string> &boolAttributes,
                                const std::unordered_set<std::string> &expectedBoolAttributes)
    {
        if (boolAttributes.size() != expectedBoolAttributes.size()) {
            return false;
        }
        for (const auto &attr : boolAttributes) {
            if (expectedBoolAttributes.find(attr) == expectedBoolAttributes.end()) {
                return false;
            }
        }
        return true;
    }

    bool ValidateAttributes(const std::unordered_map<std::string, std::vector<std::string>> &attributes,
                            const std::unordered_map<std::string, std::vector<std::string>> &expectedAttributes)
    {
        for (const auto &[key, values] : attributes) {
            if (key == "ets.annotation.id") {
                if (!ValidateAnnotationId(values)) {
                    return false;
                }
                continue;
            }
            auto expectValuesIter = expectedAttributes.find(key);
            if (expectValuesIter == expectedAttributes.end()) {
                return false;
            }
            if (!ValidateVector(values, expectValuesIter->second)) {
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

class Abc2ProgramLiteralArrayTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(LITERAL_ARRAY_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramTypeTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(TYPE_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramRecordTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(RECORD_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramInsTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(INS_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramFieldTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(FIELD_ABC_TEST_FILE_NAME.data());
        prog = &(driver.GetProgram());
    }
};

class Abc2ProgramMetadataTest : public Abc2ProgramTest {
public:
    void SetUp() override
    {
        (void)driver.Compile(METADATA_ABC_TEST_FILE_NAME.data());
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
    std::set<std::string> expectedFunctions = {
        "HelloWorld.ETSGLOBAL._$init$_:void;",
        "HelloWorld.ETSGLOBAL._cctor_:void;",
        "HelloWorld.ETSGLOBAL.main:void;",
        "HelloWorld.HelloWorld._ctor_:HelloWorld.HelloWorld;void;",
        "HelloWorld.HelloWorld.bar:HelloWorld.HelloWorld;std.core.Object;std.core.Object;",
        "HelloWorld.HelloWorld.foo:HelloWorld.HelloWorld;i32;i32;",
        "std.core.Console.log:std.core.Console;i32;void;",
        "std.core.Object._ctor_:std.core.Object;void;"};
    std::set<std::string> existingFunctions {};
    for (auto &it : prog->functionStaticTable) {
        existingFunctions.insert(it.first);
    }
    for (auto &it : prog->functionInstanceTable) {
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
    std::vector<std::string> expectedRecordNames = {"Functions.ETSGLOBAL", "Functions.Cls",
                                                    "Functions.LambdaObject-ETSGLOBAL$lambda$invoke$0",
                                                    "Functions.LambdaObject-ETSGLOBAL$lambda$invoke$1"};
    panda_file::SourceLang expectedLang = panda_file::SourceLang::ETS;
    std::vector<std::string> recordNames;
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.second.language == expectedLang);
        EXPECT_TRUE(it.first == it.second.name);
        recordNames.emplace_back(it.first);
    }
    bool result = ContainString(recordNames, expectedRecordNames);
    EXPECT_TRUE(result);
}

TEST_F(Abc2ProgramFunctionsTest, Functions)
{
    const std::string prefix = "Functions.LambdaObject-ETSGLOBAL$lambda$invoke";
    std::set<std::string> expectedFunctions = {
        "Functions.Cls._ctor_:Functions.Cls;void;",
        "Functions.Cls.func_f:Functions.Cls;i32;i32;",
        "Functions.Cls.func_i:Functions.Cls;i32;i32;",
        "Functions.Cls.func_s:i32;i32;",
        "Functions.ETSGLOBAL._$init$_:void;",
        "Functions.ETSGLOBAL._cctor_:void;",
        "Functions.ETSGLOBAL.bar:f64;std.core.String;",
        "Functions.ETSGLOBAL.foo:std.core.String;std.core.String;",
        "Functions.ETSGLOBAL.lambda$invoke$0:std.core.String;std.core.String;",
        "Functions.ETSGLOBAL.lambda$invoke$1:std.core.String;std.core.String;",
        "Functions.ETSGLOBAL.main:void;",
        prefix + "$0.$_invoke:Functions.LambdaObject-ETSGLOBAL$lambda$invoke$0;std.core.String;std.core.String;",
        prefix + "$0._ctor_:Functions.LambdaObject-ETSGLOBAL$lambda$invoke$0;void;",
        prefix + "$0.invoke1:Functions.LambdaObject-ETSGLOBAL$lambda$invoke$0;std.core.Object;std.core.Object;",
        prefix + "$1.$_invoke:Functions.LambdaObject-ETSGLOBAL$lambda$invoke$1;std.core.String;std.core.String;",
        prefix + "$1._ctor_:Functions.LambdaObject-ETSGLOBAL$lambda$invoke$1;void;",
        prefix + "$1.invoke1:Functions.LambdaObject-ETSGLOBAL$lambda$invoke$1;std.core.Object;std.core.Object;",
        "std.core.Lambda1._ctor_:std.core.Lambda1;void;",
        "std.core.Object._ctor_:std.core.Object;void;",
        "std.core.Object.toString:std.core.Object;std.core.String;",
        "std.core.StringBuilder._ctor_:std.core.StringBuilder;void;",
        "std.core.StringBuilder.append:std.core.StringBuilder;f64;std.core.StringBuilder;",
        "std.core.StringBuilder.append:std.core.StringBuilder;std.core.String;std.core.StringBuilder;",
        "std.core.StringBuilder.toString:std.core.StringBuilder;std.core.String;"};
    std::set<std::string> existingFunctions {};
    for (auto &it : prog->functionStaticTable) {
        existingFunctions.insert(it.first);
    }
    for (auto &it : prog->functionInstanceTable) {
        existingFunctions.insert(it.first);
    }
    bool result = ValidateString(existingFunctions, expectedFunctions);
    EXPECT_TRUE(result);
}

TEST_F(Abc2ProgramFunctionsTest, StringTable)
{
    std::set<std::string> expectedStrings = {"Function foo was called", "Function bar was called", "Hi,"};
    bool result = ValidateString(prog->strings, expectedStrings);
    EXPECT_TRUE(result);
}

TEST_F(Abc2ProgramLiteralArrayTest, LiteralArray)
{
    std::string tagString = std::to_string(static_cast<int>(panda_file::LiteralTag::STRING));
    std::set<std::string> expectedLiterArray = {tagString + " test ", tagString + " name01 " + tagString + " name02 "};
    std::set<std::string> existingLiterArray {};
    for (auto &[unused, litArray] : prog->literalarrayTable) {
        std::stringstream content {};
        for (const auto &l : litArray.literals) {
            content << static_cast<int>(l.tag) << " " << std::get<std::string>(l.value) << " ";
        }
        existingLiterArray.insert(content.str());
    }
    bool result = ValidateString(existingLiterArray, expectedLiterArray);
    EXPECT_TRUE(result);
}

TEST_F(Abc2ProgramTypeTest, Type)
{
    std::set<std::string> expectedType = {"Type.Cls$partial._ctor_ Type.Cls$partial void",
                                          "Type.Cls._ctor_ Type.Cls void",
                                          "Type.Cls.add_inner Type.Cls i32 i64 f32 f64 std.core.String",
                                          "Type.ETSGLOBAL._$init$_ void",
                                          "Type.ETSGLOBAL._cctor_ void",
                                          "Type.ETSGLOBAL.add_outer i32 i64 f32 f64 std.core.String",
                                          "Type.ETSGLOBAL.main void",
                                          "std.core.Object._ctor_ std.core.Object void"};
    std::set<std::string> existingType {};
    for (auto &it : prog->functionInstanceTable) {
        std::stringstream str;
        str << it.second.name;
        for (auto &param : it.second.params) {
            str << " " << param.type.GetName();
        }
        str << " " << it.second.returnType.GetName();
        existingType.insert(str.str());
    }
    for (auto &it : prog->functionStaticTable) {
        std::stringstream str;
        str << it.second.name;
        for (auto &param : it.second.params) {
            str << " " << param.type.GetName();
        }
        str << " " << it.second.returnType.GetName();
        existingType.insert(str.str());
    }
    bool result = ValidateString(existingType, expectedType);
    EXPECT_TRUE(result);
}

TEST_F(Abc2ProgramRecordTest, Record)
{
    std::set<std::string> expectedRecord = {"Record.Anno",  "Record.Cls", "Record.ETSGLOBAL",
                                            "Record.Enums", "Record.Itf", "Record.Ns"};
    std::set<std::string> existingRecord {};
    panda_file::SourceLang expectedLang = panda_file::SourceLang::ETS;
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.second.language == expectedLang);
        EXPECT_TRUE(it.first == it.second.name);
        existingRecord.insert(it.first);
    }
    bool result = ContainString(existingRecord, expectedRecord);
    EXPECT_TRUE(result);
}

TEST_F(Abc2ProgramInsTest, Ins)
{
    std::set<std::string> expectedIns = {
        "ldai 0x1 sta v0 ldai 0x2 sta v1 ldai 0x3 sta v2 lda v0 sta v4 lda v1 sta v5 lda v4 mov v4, v5 add2 v4 sta v3 "
        "lda v2 sta v4 lda v3 mov v3, v4 add2 v3 return "};
    std::set<std::string> existingIns {};
    for (auto &it : prog->functionStaticTable) {
        if (it.first == "Ins.ETSGLOBAL.test:i32;") {
            std::stringstream str;
            for (auto &i : it.second.ins) {
                str << i.ToString(" ", true, it.second.regsNum);
            }
            existingIns.insert(str.str());
        }
    }

    bool result = ValidateString(expectedIns, existingIns);
    EXPECT_TRUE(result);
}

TEST_F(Abc2ProgramFieldTest, Field)
{
    std::set<std::string> expectedField = {"anno1", "cls1", "enums1", "ns1"};
    std::set<std::string> existingField {};
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.first == it.second.name);
        for (const auto &r : it.second.fieldList) {
            existingField.insert(r.name);
        }
    }
    bool result = ContainString(existingField, expectedField);
    EXPECT_TRUE(result);
}

TEST_F(Abc2ProgramMetadataTest, Record1)
{
    std::unordered_set<std::string> expectedBoolAttributes = {"ets.annotation", "ets.abstract"};
    std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {{"access.record", {"public"}}};
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.first == it.second.name);
        if (it.second.name != "Metadata.Anno") {
            continue;
        }
        auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_TRUE(ValidateBoolAttributes(boolAttributes, expectedBoolAttributes));
        auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Record2)
{
    std::unordered_set<std::string> expectedBoolAttributes = {"ets.abstract", "ets.interface"};
    std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"ets.extends", {"std.core.Object"}}, {"access.record", {"public"}}};
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.first == it.second.name);
        if (it.second.name != "Metadata.Iface") {
            continue;
        }
        auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_TRUE(ValidateBoolAttributes(boolAttributes, expectedBoolAttributes));
        auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Record3)
{
    std::unordered_set<std::string> expectedBoolAttributes = {};
    std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"ets.annotation.element.value", {"\"Cls\""}},
        {"ets.annotation.class", {"Metadata.Anno"}},
        {"ets.annotation.type", {"class"}},
        {"ets.implements", {"Metadata.Iface"}},
        {"ets.annotation.element.type", {"string"}},
        {"ets.annotation.element.name", {"name"}},
        {"ets.extends", {"std.core.Object"}},
        {"access.record", {"public"}}};
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.first == it.second.name);
        if (it.second.name != "Metadata.Cls") {
            continue;
        }
        auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_TRUE(ValidateBoolAttributes(boolAttributes, expectedBoolAttributes));
        auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Record4)
{
    std::unordered_set<std::string> expectedBoolAttributes = {"ets.abstract"};
    std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"ets.annotation.element.value", {""}},
        {"ets.annotation.class", {"ets.annotation.Module"}},
        {"ets.annotation.type", {"class"}},
        {"ets.annotation.element.type", {"array"}},
        {"ets.annotation.element.name", {"exported"}},
        {"ets.extends", {"std.core.Object"}},
        {"access.record", {"public"}}};
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.first == it.second.name);
        if (it.second.name != "Metadata.Ns") {
            continue;
        }
        auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_TRUE(ValidateBoolAttributes(boolAttributes, expectedBoolAttributes));
        auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Field)
{
    std::unordered_set<std::string> expectedBoolAttributes = {};
    std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"ets.annotation.element.type", {"string"}},
        {"ets.annotation.element.name", {"name"}},
        {"ets.annotation.class", {"Metadata.Anno"}},
        {"ets.annotation.element.value", {"\"foo\""}},
        {"access.field", {"public"}}};
    for (const auto &it : prog->recordTable) {
        EXPECT_TRUE(it.first == it.second.name);
        if (it.second.name != "Metadata.Cls") {
            continue;
        }
        for (const auto &r : it.second.fieldList) {
            if (r.name != "foo") {
                continue;
            }
            auto boolAttributes = it.second.metadata->GetBoolAttributes();
            EXPECT_TRUE(ValidateBoolAttributes(boolAttributes, expectedBoolAttributes));
            auto attributes = r.metadata->GetAttributes();
            EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
        }
    }
}

TEST_F(Abc2ProgramMetadataTest, Method1)
{
    std::unordered_set<std::string> expectedBoolAttributes = {};
    std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {
        {"ets.annotation.element.type", {"string"}},
        {"ets.annotation.element.name", {"name"}},
        {"ets.annotation.class", {"Metadata.Anno"}},
        {"ets.annotation.element.value", {"\"bar\""}},
        {"access.function", {"public"}}};
    for (const auto &it : prog->functionInstanceTable) {
        if (it.second.name != "Metadata.Cls.bar") {
            continue;
        }
        auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_TRUE(ValidateBoolAttributes(boolAttributes, expectedBoolAttributes));
        auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Method2)
{
    std::unordered_set<std::string> expectedBoolAttributes = {"static"};
    std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {{"access.function", {"public"}}};
    for (const auto &it : prog->functionStaticTable) {
        if (it.second.name != "Metadata.Ns.add") {
            continue;
        }
        auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_TRUE(ValidateBoolAttributes(boolAttributes, expectedBoolAttributes));
        auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

TEST_F(Abc2ProgramMetadataTest, Method3)
{
    std::unordered_set<std::string> expectedBoolAttributes = {"ctor"};
    std::unordered_map<std::string, std::vector<std::string>> expectedAttributes = {{"access.function", {"public"}}};
    for (const auto &it : prog->functionInstanceTable) {
        if (it.second.name != "Metadata.Cls._ctor_") {
            continue;
        }
        auto boolAttributes = it.second.metadata->GetBoolAttributes();
        EXPECT_TRUE(ValidateBoolAttributes(boolAttributes, expectedBoolAttributes));
        auto attributes = it.second.metadata->GetAttributes();
        EXPECT_TRUE(ValidateAttributes(attributes, expectedAttributes));
    }
}

}  // namespace ark::abc2program
