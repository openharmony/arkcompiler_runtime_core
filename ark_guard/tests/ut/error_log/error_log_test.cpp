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
#include <sstream>
#include <csetjmp>

#include "abckit_wrapper/modifiers.h"
#include "ark_guard/library/error.h"
#include "ark_guard/library/util/assert_util.h"
#include "ark_guard/library/name_cache/name_cache_parser.h"
#include "configuration/configuration_parser.h"
#include "configuration/configuration_constants.h"
#include "configuration/keep_option_parser.h"
#include "ark_guard/ark_guard.h"
#include "util/json_util.h"
#include "util/file_util.h"
#include "util/string_util.h"
#include "ark_guard/library/logger.h"

using namespace testing::ext;

namespace {
const std::string CONFIG_FILE_PATH = ARK_GUARD_UNIT_TEST_DIR "ut/error_log";
}  // namespace

/**
 * @tc.name: error_log_test_001
 * @tc.desc: test invalid keep option error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_001, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_001.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311003\n\\[Description\\]:KeepOption parsing failed: keepOption can only be one of '-keep', "
        "'-keepclassmembers', '-keepclasswithmembers', unknown keep option: -keeps\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_002
 * @tc.desc: test unknown keep option error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_002, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_002.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311003\n\\[Description\\]:KeepOption parsing failed: keepOption can only be one of '-keep', "
        "'-keepclassmembers', '-keepclasswithmembers', unknown keep option: xxx\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_003
 * @tc.desc: test invalid field access flags error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_003, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_003.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n\\[Description\\]:ClassSpecification parsing failed: "
        "invalid access flags for field: '\\[\\^/\\\\\\.\\]\\*'. \nCurrent line:\n    abstract \\*;\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_004
 * @tc.desc: test invalid field name access flags error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_004, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_004.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n\\[Description\\]:ClassSpecification parsing failed: invalid access flags for "
        "field: 'f1'. \nCurrent line:\n    abstract f1:f32;\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_005
 * @tc.desc: test invalid fields wildcard access flags error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_005, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_005.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n\\[Description\\]:ClassSpecification parsing failed: invalid access flags for "
        "field: '<fields>'. \nCurrent line:\n    abstract <fields>;\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_006
 * @tc.desc: test conflicting access flags error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_006, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_006.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n\\[Description\\]:ClassSpecification parsing failed: conflicting access flags for: "
        "'public'. \nCurrent line:\n    public !public <methods>;\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_007
 * @tc.desc: test unexpected character after methods error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_007, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_007.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n\\[Description\\]:ClassSpecification parsing failed: the expect character is ';', "
        "but actual character is '\\*'. \nCurrent line:\n    <methods> \\*\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_008
 * @tc.desc: test missing class brace error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_008, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_008.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n\\[Description\\]:ClassSpecification parsing failed: the expect character is '\\{', "
        "but actual character is 'B'.\nCurrent line:\n-keep @AnnotationA !final class classA B \\{\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_009
 * @tc.desc: test unknown access flag error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_009, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_009.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n\\[Description\\]:ClassSpecification parsing failed: unknown access flag: "
        "'public'.\nCurrent line:\n-keep @AnnotationA public class classA \\{\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_010
 * @tc.desc: test invalid member format error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_010, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_010.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n\\[Description\\]:ClassSpecification parsing failed: the format of member part of "
        "class_specification is incorrect. The member name is 'f1'.\nCurrent line:\n    f1 f2;\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_011
 * @tc.desc: test missing colon error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_011, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_011.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n"
        "\\[Description\\]:ClassSpecification parsing failed: the expect character is ':', but actual character is "
        "';'.\nCurrent line:\n    foo\\(\\);\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_012
 * @tc.desc: test missing closing single quote error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_012, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_012.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n"
        "\\[Description\\]:ClassSpecification parsing failed: missing closing quote.\nCurrent line:\n    "
        "'foo\\(\\):void;\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_013
 * @tc.desc: test missing closing double quote error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_013, TestSize.Level0)
{
    std::string configFilePath = CONFIG_FILE_PATH + "/" + "error_log_test_013.json";
    ark::guard::ConfigurationParser processor(configFilePath);
    ark::guard::Configuration configuration;
    std::string expected =
        "\\[ErrorCode\\]:11311004\n"
        "\\[Description\\]:ClassSpecification parsing failed: missing closing quote.\nCurrent line:\n    "
        "\"foo\\(\\):void;\n";
    EXPECT_DEATH(processor.Parse(configuration), expected);
}

/**
 * @tc.name: error_log_test_014
 * @tc.desc: test unknown name cache module format error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_014, TestSize.Level0)
{
    std::string applyNameCache = CONFIG_FILE_PATH + "/" + "error_log_test_014.json";
    ark::guard::NameCacheParser parser(applyNameCache);
    std::string expected =
        "\\[ErrorCode\\]:11311006\n"
        "\\[Description\\]:NameCache parsing failed: unknown name cache module format, object: ns1, key: #obfName\n";
    EXPECT_DEATH(parser.Parse(), expected);
}

/**
 * @tc.name: error_log_test_015
 * @tc.desc: test unknown name cache key format error message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_015, TestSize.Level0)
{
    std::string applyNameCache = CONFIG_FILE_PATH + "/" + "error_log_test_015.json";
    ark::guard::NameCacheParser parser(applyNameCache);
    std::string expected =
        "\\[ErrorCode\\]:11311006\n"
        "\\[Description\\]:NameCache parsing failed: unknown name cache module format, object: "
        "name_cache_keeper_test_001, key: field1\n";
    EXPECT_DEATH(parser.Parse(), expected);
}

/**
 * @tc.name: error_log_test_016
 * @tc.desc: verify ArkGuard::Run fails with invalid arguments (parameter parsing failed)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_016, TestSize.Level0)
{
    const char *argv[] = {"ark_guard", "--unknown"};
    int argc = 2;

    std::string regex =
        "Usage:\nark_guard \\[options\\] config-file-path\nSupported "
        "options:(.|\n)*\\[ErrorCode\\]:11311001\n\\[Description\\]:Parameter parsing failed: invalid arguments";
    ark::guard::ArkGuard guard;
    EXPECT_DEATH(guard.Run(argc, argv), regex);
}

/**
 * @tc.name: error_log_test_017
 * @tc.desc: verify GetStringValue asserts on missing required field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_017, TestSize.Level0)
{
    std::string jsonStr = R"({"foo": "bar"})";
    ark::JsonObject object(jsonStr);

    EXPECT_EQ(ark::guard::JsonUtil::GetStringValue(&object, "missing", true), "");

    std::string regex =
        "\\[ErrorCode\\]:11311002\n\\[Description\\]:Configuration parsing failed: fail to obtain required "
        "field:missing";
    EXPECT_DEATH(ark::guard::JsonUtil::GetStringValue(&object, "missing", false), regex);
}

/**
 * @tc.name: error_log_test_018
 * @tc.desc: verify GetBoolValue asserts on missing required field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_018, TestSize.Level0)
{
    std::string jsonStr = R"({"foo": true})";
    ark::JsonObject object(jsonStr);

    EXPECT_EQ(ark::guard::JsonUtil::GetBoolValue(&object, "missing", false, true), false);

    std::string regex =
        "\\[ErrorCode\\]:11311002\n\\[Description\\]:Configuration parsing failed: fail to obtain required "
        "field:missing";
    EXPECT_DEATH(ark::guard::JsonUtil::GetBoolValue(&object, "missing", false, false), regex);
}

/**
 * @tc.name: error_log_test_019
 * @tc.desc: verify GetArrayStringValue asserts on missing required field
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_019, TestSize.Level0)
{
    std::string jsonStr = R"({"foo": ["bar"]})";
    ark::JsonObject object(jsonStr);

    auto res = ark::guard::JsonUtil::GetArrayStringValue(&object, "missing", true);
    EXPECT_TRUE(res.empty());

    std::string regex =
        "\\[ErrorCode\\]:11311002\n\\[Description\\]:Configuration parsing failed: fail to obtain required "
        "field:missing";
    EXPECT_DEATH(ark::guard::JsonUtil::GetArrayStringValue(&object, "missing", false), regex);
}

/**
 * @tc.name: error_log_test_020
 * @tc.desc: verify WriteFile asserts when file cannot be opened
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_020, TestSize.Level0)
{
    ark::Logger::ComponentMask component_mask;
    component_mask.set(ark::Logger::Component::ARK_GUARD);
    ark::Logger::InitializeStdLogging(ark::Logger::Level::ERROR, component_mask);
    // Try to write to a directory that doesn't exist
    std::string invalidPath = "/non/existent/dir/file.txt";
    std::string regex =
        "can not open file, invalid "
        "filePath:.*/non/existent/dir/file.txt(.|\\n)*\\[ErrorCode\\]:11311900\n\\[Description\\]:obfuscate failed";
    EXPECT_DEATH(ark::guard::FileUtil::WriteFile(invalidPath, "content"), regex);
}

/**
 * @tc.name: error_log_test_021
 * @tc.desc: verify ConfigurationParser fails with empty file
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_021, TestSize.Level0)
{
    std::string emptyConfig = CONFIG_FILE_PATH + "/" + "empty_config.json";
    FILE *fp = fopen(emptyConfig.c_str(), "w");
    if (fp) {
        fclose(fp);
    }

    ark::guard::ConfigurationParser parser(emptyConfig);
    ark::guard::Configuration configuration;
    std::string regex =
        "\\[ErrorCode\\]:11311002\n\\[Description\\]:Configuration parsing failed: configuration file is empty:.*";
    EXPECT_DEATH(parser.Parse(configuration), regex);

    remove(emptyConfig.c_str());
}

/**
 * @tc.name: error_log_test_022
 * @tc.desc: verify ConfigurationParser fails with invalid JSON
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_022, TestSize.Level0)
{
    std::string invalidJsonConfig = CONFIG_FILE_PATH + "/" + "invalid_json.json";
    std::string content = "{ \"abcPath\": \"path\", ";
    FILE *fp = fopen(invalidJsonConfig.c_str(), "w");
    if (fp) {
        fwrite(content.c_str(), 1, content.length(), fp);
        fclose(fp);
    }

    ark::guard::ConfigurationParser parser(invalidJsonConfig);
    ark::guard::Configuration configuration;
    std::string regex =
        "\\[ErrorCode\\]:11311002\n\\[Description\\]:Configuration parsing failed: the configuration file is not a "
        "valid json";
    EXPECT_DEATH(parser.Parse(configuration), regex);

    remove(invalidJsonConfig.c_str());
}

/**
 * @tc.name: error_log_test_023
 * @tc.desc: test RemovePrefixIfMatches assertions
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ErrorLogTest, error_log_test_023, TestSize.Level0)
{
    ark::Logger::ComponentMask component_mask;
    component_mask.set(ark::Logger::Component::ARK_GUARD);
    ark::Logger::InitializeStdLogging(ark::Logger::Level::ERROR, component_mask);
    std::string regex =
        "remove invalid prefix to: someString(.|\\n)*\\[ErrorCode\\]:11311900\n\\[Description\\]:obfuscate failed";
    EXPECT_DEATH(ark::guard::StringUtil::RemovePrefixIfMatches("someString", ""), regex);
}
