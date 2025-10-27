/**
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

#include <gtest/gtest.h>

#include "abckit_wrapper/modifiers.h"

#include "configuration/configuration_parser.h"

using namespace testing::ext;

namespace {
const std::string CONFIG_FILE_PATH = ARK_GUARD_UNIT_TEST_DIR "ut/configuration";
}  // namespace

/**
 * @tc.name: configuration_parser_test_001
 * @tc.desc: verify whether the regular config json file without keep options is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_001, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_001.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    ASSERT_EQ(configuration.GetAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetObfAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetDefaultNameCachePath(), "xxx");
    ASSERT_TRUE(configuration.HasKeepConfiguration());

    ASSERT_EQ(configuration.DisableObfuscation(), true);
    ASSERT_EQ(configuration.IsRemoveLogObfEnabled(), true);
    ASSERT_EQ(configuration.GetPrintNameCache(), "xxx");
    ASSERT_EQ(configuration.GetApplyNameCache(), "xxx");

    ASSERT_EQ(configuration.GetPrintSeedsOption().enable, true);
    ASSERT_EQ(configuration.GetPrintSeedsOption().filePath, "xxx");

    ASSERT_EQ(configuration.IsFileNameObfEnabled(), true);
    ASSERT_EQ(configuration.IsKeepFileName("xxx1"), true);
    ASSERT_EQ(configuration.IsKeepFileName("xxx2"), true);
    ASSERT_EQ(configuration.IsKeepFileName("xax"), true);
    ASSERT_EQ(configuration.IsKeepFileName("xAx"), false);
    ASSERT_EQ(configuration.IsKeepFileName("1xx"), true);
    ASSERT_EQ(configuration.IsKeepFileName("zzz"), false);
}

/**
 * @tc.name: configuration_parser_test_002
 * @tc.desc: verify whether the regular config json file with keep options is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_002, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_002.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    processor.Parse(configuration);

    ASSERT_EQ(configuration.GetAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetObfAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetDefaultNameCachePath(), "xxx");
    ASSERT_TRUE(configuration.HasKeepConfiguration());

    const auto classSpecifications = configuration.GetClassSpecifications();
    ASSERT_EQ(classSpecifications.size(), 3);
    for (const auto &specification : classSpecifications) {
        ASSERT_EQ(specification.annotationName_, "AnnotationA");
        ASSERT_EQ(specification.setAccessFlags_, 0);
        ASSERT_EQ(specification.unSetAccessFlags_, abckit_wrapper::AccessFlags::FINAL);
        ASSERT_EQ(specification.setTypeDeclarations_, abckit_wrapper::TypeDeclarations::CLASS);
        ASSERT_EQ(specification.unSetTypeDeclarations_, 0);
        ASSERT_EQ(specification.className_, "classA");
        ASSERT_EQ(specification.extensionType_, abckit_wrapper::ExtensionType::EXTENDS);
        ASSERT_EQ(specification.extensionAnnotationName_, "AnnotationB");
        ASSERT_EQ(specification.extensionClassName_, "classB");

        ASSERT_EQ(specification.fieldSpecifications_.size(), 1);
        ASSERT_EQ(specification.fieldSpecifications_[0].keepMember_, true);

        ASSERT_EQ(specification.methodSpecifications_.size(), 1);
        ASSERT_EQ(specification.methodSpecifications_[0].keepMember_, true);
    }
}

/**
 * @tc.name: configuration_parser_test_003
 * @tc.desc: verify whether the abnormal config json file is parse failed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_003, TestSize.Level0)
{
    std::vector<std::string> configFilePathList = {
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_no_abcPath.json",
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_no_obfAbcPath.json",
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_no_defaultNameCache.json",
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_abcPath_empty.json",
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_obfAbcPath_empty.json",
        CONFIG_FILE_PATH + "/" + "configuration_parser_test_003_defaultNameCache_empty.json",
    };
    for (const auto &configFilePath : configFilePathList) {
        ark::guard::ConfigurationParser processor(configFilePath);
        ark::guard::Configuration configuration;
        EXPECT_DEATH(processor.Parse(configuration), "");
    }
}

/**
 * @tc.name: configuration_parser_test_004
 * @tc.desc: verify whether the config json file with required field is successfully parsed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_004, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_004.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    ASSERT_EQ(configuration.GetAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetObfAbcPath(), "xxx");
    ASSERT_EQ(configuration.GetDefaultNameCachePath(), "xxx");
    ASSERT_FALSE(configuration.HasKeepConfiguration());

    ASSERT_EQ(configuration.DisableObfuscation(), false);
    ASSERT_EQ(configuration.IsRemoveLogObfEnabled(), false);
    ASSERT_EQ(configuration.GetPrintNameCache(), "");
    ASSERT_EQ(configuration.GetApplyNameCache(), "");

    ASSERT_EQ(configuration.GetPrintSeedsOption().enable, false);
    ASSERT_EQ(configuration.GetPrintSeedsOption().filePath, "");

    ASSERT_EQ(configuration.IsFileNameObfEnabled(), false);
    ASSERT_EQ(configuration.IsKeepFileName("xxx"), false);

    ASSERT_EQ(configuration.IsKeepPath("xxx"), false);
}

/**
 * @tc.name: configuration_parser_test_005
 * @tc.desc: verify the config json file with keep path
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_005, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_005.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    ASSERT_TRUE(configuration.HasKeepConfiguration());

    ASSERT_TRUE(configuration.IsKeepPath("entry.src.main.Main1"));
    ASSERT_TRUE(configuration.IsKeepPath("entry.src.main.Main2"));
    ASSERT_TRUE(configuration.IsKeepPath("entry.src.main.Main3"));
    ASSERT_TRUE(configuration.IsKeepPath("entry.src.main.Main4"));
    ASSERT_TRUE(configuration.IsKeepPath("entry.src.A.B.C.main.Main4"));
    ASSERT_FALSE(configuration.IsKeepPath("entry.src.A.B.C.main.Main6"));
}

/**
 * @tc.name: configuration_parser_test_006
 * @tc.desc: verify the config json file with keep union type field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_006, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_006.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    std::unordered_map<std::string, std::string> expectFieldTypeMap {{"v1", "{Ustd.core.Int,std.core.String}"},
                                                                     {"v2", "{Ustd.core.Byte,std.core.String}"},
                                                                     {"v3", "{Ustd.core.Short,std.core.String}"},
                                                                     {"v4", "{Ustd.core.Double,std.core.String}"},
                                                                     {"v5", "{Ustd.core.Long,std.core.String}"},
                                                                     {"v6", "{Ustd.core.Float,std.core.String}"},
                                                                     {"v7", "{Ustd.core.Double,std.core.String}"},
                                                                     {"v8", "{Ustd.core.Char,std.core.String}"},
                                                                     {"v9", "{Ustd.core.Boolean,std.core.String}"},
                                                                     {"v10", "{Ustd.core.BigInt,std.core.String}"},
                                                                     {"v11", "std.core.String"},
                                                                     {"v12", "{Ustd.core.Null,std.core.String}"},
                                                                     {"v13", "std.core.String"},
                                                                     {"v14", "{Uescompat.Array,std.core.String}"},
                                                                     {"v15", "std.core.Object"},
                                                                     {"v16", "std.core.Int"}};

    const auto classSpecifications = configuration.GetClassSpecifications();
    ASSERT_TRUE(!classSpecifications.empty());

    for (const auto &classSpec : classSpecifications) {
        ASSERT_TRUE(!classSpec.fieldSpecifications_.empty());
        for (const auto &fieldSpec : classSpec.fieldSpecifications_) {
            auto it = expectFieldTypeMap.find(fieldSpec.memberName_);
            ASSERT_TRUE(it != expectFieldTypeMap.end()) << "not find config:" << fieldSpec.memberName_;
            ASSERT_EQ(fieldSpec.memberType_, it->second);
        }
    }
}

/**
 * @tc.name: configuration_parser_test_007
 * @tc.desc: verify the config json file with keep union type method arguments type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_007, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_007.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    std::unordered_map<std::string, std::string> expectMethodArgumentTypeMap {
        {"foo1", "{Ustd.core.Int,std.core.String}"},
        {"foo2", "{Ustd.core.Byte,std.core.String}"},
        {"foo3", "{Ustd.core.Short,std.core.String}"},
        {"foo4", "{Ustd.core.Double,std.core.String}"},
        {"foo5", "{Ustd.core.Long,std.core.String}"},
        {"foo6", "{Ustd.core.Float,std.core.String}"},
        {"foo7", "{Ustd.core.Double,std.core.String}"},
        {"foo8", "{Ustd.core.Char,std.core.String}"},
        {"foo9", "{Ustd.core.Boolean,std.core.String}"},
        {"foo10", "{Ustd.core.BigInt,std.core.String}"},
        {"foo11", "std.core.String"},
        {"foo12", "{Ustd.core.Null,std.core.String}"},
        {"foo13", "std.core.String"},
        {"foo14", "{Uescompat.Array,std.core.String}"},
        {"foo15", "std.core.Object"},
        {"foo16", "std.core.Int"}};

    const auto classSpecifications = configuration.GetClassSpecifications();
    ASSERT_TRUE(!classSpecifications.empty());

    for (const auto &classSpec : classSpecifications) {
        ASSERT_TRUE(!classSpec.methodSpecifications_.empty());
        for (const auto &methodSpec : classSpec.methodSpecifications_) {
            auto it = expectMethodArgumentTypeMap.find(methodSpec.memberName_);
            ASSERT_TRUE(it != expectMethodArgumentTypeMap.end()) << "not find config:" << methodSpec.memberName_;
            ASSERT_EQ(methodSpec.memberType_, it->second);
        }
    }
}

/**
 * @tc.name: configuration_parser_test_008
 * @tc.desc: verify the config json file with keep union type method return type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_008, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_008.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    std::unordered_map<std::string, std::string> expectMethodReturnTypeMap {
        {"fooEx1", "{Ustd.core.Int,std.core.String}"},
        {"fooEx2", "{Ustd.core.Byte,std.core.String}"},
        {"fooEx3", "{Ustd.core.Short,std.core.String}"},
        {"fooEx4", "{Ustd.core.Double,std.core.String}"},
        {"fooEx5", "{Ustd.core.Long,std.core.String}"},
        {"fooEx6", "{Ustd.core.Float,std.core.String}"},
        {"fooEx7", "{Ustd.core.Double,std.core.String}"},
        {"fooEx8", "{Ustd.core.Char,std.core.String}"},
        {"fooEx9", "{Ustd.core.Boolean,std.core.String}"},
        {"fooEx10", "{Ustd.core.BigInt,std.core.String}"},
        {"fooEx11", "std.core.String"},
        {"fooEx12", "{Ustd.core.Null,std.core.String}"},
        {"fooEx13", "std.core.String"},
        {"fooEx14", "{Uescompat.Array,std.core.String}"},
        {"fooEx15", "std.core.Object"},
        {"fooEx16", "std.core.Int"}};

    const auto classSpecifications = configuration.GetClassSpecifications();
    ASSERT_TRUE(!classSpecifications.empty());

    for (const auto &classSpec : classSpecifications) {
        ASSERT_TRUE(!classSpec.methodSpecifications_.empty());
        for (const auto &methodSpec : classSpec.methodSpecifications_) {
            auto it = expectMethodReturnTypeMap.find(methodSpec.memberName_);
            ASSERT_TRUE(it != expectMethodReturnTypeMap.end()) << "not find config:" << methodSpec.memberName_;
            ASSERT_EQ(methodSpec.memberValue_, it->second);
        }
    }
}

/**
 * @tc.name: configuration_parser_test_009
 * @tc.desc: verify the config json file with keep union type is sorted
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigParserTest, configuration_parser_test_009, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "configuration_parser_test_009.json";
    ark::guard::ConfigurationParser parser(configFilePath);
    ark::guard::Configuration configuration;
    parser.Parse(configuration);

    std::unordered_map<std::string, std::string> expectMap {
            {"foo1", "{Ustd.core.Int,std.core.String}"},
            {"foo2", "{Uescompat.Array,std.core.String}"}};

    const auto classSpecifications = configuration.GetClassSpecifications();
    ASSERT_TRUE(!classSpecifications.empty());

    for (const auto &classSpec : classSpecifications) {
        ASSERT_TRUE(!classSpec.methodSpecifications_.empty());
        for (const auto &methodSpec : classSpec.methodSpecifications_) {
            auto it = expectMap.find(methodSpec.memberName_);
            ASSERT_TRUE(it != expectMap.end()) << "not find config:" << methodSpec.memberName_;
            ASSERT_EQ(methodSpec.memberValue_, it->second);
        }
    }
}