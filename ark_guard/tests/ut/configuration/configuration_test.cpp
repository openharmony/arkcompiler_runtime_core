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
#include <string>
#include <vector>

#include "abckit_wrapper/modifiers.h"

#include "configuration/configuration.h"
#include "configuration/configuration_parser.h"

using namespace testing::ext;
using namespace ark::guard;

/**
 * @tc.name: WarmupRegexCache_test_001
 * @tc.desc: Test WarmupRegexCache with empty configuration
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, WarmupRegexCache_test_001, TestSize.Level0)
{
    Configuration configuration;
    EXPECT_NO_FATAL_FAILURE(configuration.WarmupRegexCache());
}

/**
 * @tc.name: WarmupRegexCache_test_002
 * @tc.desc: Test WarmupRegexCache with fileNameOption.universalReservedFileNames
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, WarmupRegexCache_test_002, TestSize.Level0)
{
    Configuration Configuration;
    Configuration.obfuscationRules.fileNameOption.enable = true;
    Configuration.obfuscationRules.fileNameOption.universalReservedFileNames = {"test\\..*", ".*\\.tmp", ".*\\.log"};
    EXPECT_NO_FATAL_FAILURE(Configuration.WarmupRegexCache());
}

/**
 * @tc.name: WarmupRegexCache_test_003
 * @tc.desc: Test WarmupRegexCache with pathOption.universalReservedPaths
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, WarmupRegexCache_test_003, TestSize.Level0)
{
    Configuration Configuration;
    Configuration.obfuscationRules.keepOptions.pathOption.universalReservedPaths = {".*/test/.*", ".*/src/.*",
                                                                                    ".*/lib/.*"};
    EXPECT_NO_FATAL_FAILURE(Configuration.WarmupRegexCache());
}

/**
 * @tc.name: WarmupRegexCache_test_004
 * @tc.desc: Test WarmupRegexCache with classSpecifications containing className and extensionClassName
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, WarmupRegexCache_test_004, TestSize.Level0)
{
    Configuration Configuration;
    ClassSpecification spec1;
    spec1.className_ = "TestClass";
    spec1.extensionClassName_ = "BaseClass";
    spec1.annotationName_ = "TestAnnotation";
    spec1.extensionAnnotationName_ = "BaseAnnotation";

    ClassSpecification spec2;
    spec2.className_ = "AnotherClass";
    spec2.extensionClassName_ = "";  // Empty should be skipped

    Configuration.obfuscationRules.keepOptions.classSpecifications.push_back(spec1);
    Configuration.obfuscationRules.keepOptions.classSpecifications.push_back(spec2);
    EXPECT_NO_FATAL_FAILURE(Configuration.WarmupRegexCache());
}

/**
 * @tc.name: WarmupRegexCache_test_005
 * @tc.desc: Test WarmupRegexCache with classSpecifications containing fieldSpecifications
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, WarmupRegexCache_test_005, TestSize.Level0)
{
    Configuration Configuration;
    ClassSpecification spec;
    spec.className_ = "TestClass";

    MemberSpecification field1;
    field1.memberName_ = "field1";
    field1.memberType_ = "int";
    field1.memberValue_ = "0";

    MemberSpecification field2;
    field2.memberName_ = "field2";
    field2.memberType_ = "string";
    field2.memberValue_ = "";  // Empty should be skipped

    MemberSpecification field3;
    field3.memberName_ = "";  // Empty should be skipped
    field3.memberType_ = "float";
    field3.memberValue_ = "1.0";

    spec.fieldSpecifications_.push_back(field1);
    spec.fieldSpecifications_.push_back(field2);
    spec.fieldSpecifications_.push_back(field3);

    Configuration.obfuscationRules.keepOptions.classSpecifications.push_back(spec);
    EXPECT_NO_FATAL_FAILURE(Configuration.WarmupRegexCache());
}

/**
 * @tc.name: WarmupRegexCache_test_006
 * @tc.desc: Test WarmupRegexCache with classSpecifications containing methodSpecifications
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, WarmupRegexCache_test_006, TestSize.Level0)
{
    Configuration Configuration;
    ClassSpecification spec;
    spec.className_ = "TestClass";

    MemberSpecification method1;
    method1.memberName_ = "method1";
    method1.memberType_ = "int,string";
    method1.memberValue_ = "void";

    MemberSpecification method2;
    method2.memberName_ = "method2";
    method2.memberType_ = "";
    method2.memberValue_ = "int";

    MemberSpecification method3;
    method3.memberName_ = "";
    method3.memberType_ = "string";
    method3.memberValue_ = "";

    spec.methodSpecifications_.push_back(method1);
    spec.methodSpecifications_.push_back(method2);
    spec.methodSpecifications_.push_back(method3);

    Configuration.obfuscationRules.keepOptions.classSpecifications.push_back(spec);
    EXPECT_NO_FATAL_FAILURE(Configuration.WarmupRegexCache());
}

/**
 * @tc.name: WarmupRegexCache_test_007
 * @tc.desc: Test WarmupRegexCache with all options combined
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, WarmupRegexCache_test_007, TestSize.Level0)
{
    Configuration Configuration;
    // Set fileNameOption
    Configuration.obfuscationRules.fileNameOption.enable = true;
    Configuration.obfuscationRules.fileNameOption.universalReservedFileNames = {"test\\..*", ".*\\.tmp"};

    // Set path pathOption
    Configuration.obfuscationRules.keepOptions.pathOption.universalReservedPaths = {".*/test/.*"};

    // Set classSpecifications with all fields
    ClassSpecification spec;
    spec.className_ = "TestClass";
    spec.extensionClassName_ = "BaseClass";
    spec.annotationName_ = "TestAnnotation";
    spec.extensionAnnotationName_ = "BaseAnnotation";

    MemberSpecification field;
    field.memberName_ = "testField";
    field.memberType_ = "int";
    field.memberValue_ = "0";
    spec.fieldSpecifications_.push_back(field);

    MemberSpecification method;
    method.memberName_ = "testMethod";
    method.memberType_ = "int";
    method.memberValue_ = "void";
    spec.methodSpecifications_.push_back(method);

    Configuration.obfuscationRules.keepOptions.classSpecifications.push_back(spec);
    EXPECT_NO_FATAL_FAILURE(Configuration.WarmupRegexCache());
}

/**
 * @tc.name: WarmupRegexCache_test_008
 * @tc.desc: Test WarmupRegexCache with empty patterns (all empty strings)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, WarmupRegexCache_test_008, TestSize.Level0)
{
    Configuration Configuration;
    Configuration.obfuscationRules.fileNameOption.universalReservedFileNames = {"", ""};
    Configuration.obfuscationRules.keepOptions.pathOption.universalReservedPaths = {""};

    ClassSpecification spec;
    spec.className_ = "";
    spec.extensionClassName_ = "";
    spec.annotationName_ = "";
    spec.extensionAnnotationName_ = "";

    MemberSpecification field;
    field.memberName_ = "";
    field.memberType_ = "";
    field.memberValue_ = "";
    spec.fieldSpecifications_.push_back(field);

    Configuration.obfuscationRules.keepOptions.classSpecifications.push_back(spec);
    EXPECT_NO_FATAL_FAILURE(Configuration.WarmupRegexCache());
}

/**
 * @tc.name: IsReserved_test_001
 * @tc.desc: Test IsReserved function with exact match in reservedNames
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, IsReserved_test_001, TestSize.Level0)
{
    Configuration Configuration;
    Configuration.obfuscationRules.fileNameOption.enable = true;
    Configuration.obfuscationRules.fileNameOption.reservedFileNames = {"exact1", "exact2", "test.file"};
    Configuration.obfuscationRules.fileNameOption.universalReservedFileNames.clear();

    EXPECT_TRUE(Configuration.IsKeepFileName("exact1"));
    EXPECT_TRUE(Configuration.IsKeepFileName("exact2"));
    EXPECT_TRUE(Configuration.IsKeepFileName("test.file"));
    EXPECT_FALSE(Configuration.IsKeepFileName("notfound"));
    EXPECT_FALSE(Configuration.IsKeepFileName("exact"));
}

/**
 * @tc.name: IsReserved_test_002
 * @tc.desc: Test IsReserved function with regex match in universalReservedNames
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, IsReserved_test_002, TestSize.Level0)
{
    Configuration Configuration;
    Configuration.obfuscationRules.fileNameOption.enable = true;
    Configuration.obfuscationRules.fileNameOption.reservedFileNames.clear();
    Configuration.obfuscationRules.fileNameOption.universalReservedFileNames = {"test\\..*", ".*\\.tmp", ".*\\.log"};

    EXPECT_TRUE(Configuration.IsKeepFileName("test.abc"));
    EXPECT_TRUE(Configuration.IsKeepFileName("test.xyz"));
    EXPECT_TRUE(Configuration.IsKeepFileName("file.tmp"));
    EXPECT_TRUE(Configuration.IsKeepFileName("app.log"));
    EXPECT_FALSE(Configuration.IsKeepFileName("test"));
    EXPECT_FALSE(Configuration.IsKeepFileName("tmp"));
}

/**
 * @tc.name: IsReserved_test_003
 * @tc.desc: Test IsReserved function with both reservedNames and universalReservedNames
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, IsReserved_test_003, TestSize.Level0)
{
    Configuration Configuration;
    Configuration.obfuscationRules.fileNameOption.enable = true;
    Configuration.obfuscationRules.fileNameOption.reservedFileNames = {"exact1", "exact2"};
    Configuration.obfuscationRules.fileNameOption.universalReservedFileNames = {"test\\..*", ".*\\.tmp"};

    EXPECT_TRUE(Configuration.IsKeepFileName("exact1"));
    EXPECT_TRUE(Configuration.IsKeepFileName("exact2"));
    EXPECT_TRUE(Configuration.IsKeepFileName("test.abc"));
    EXPECT_TRUE(Configuration.IsKeepFileName("file.tmp"));
    EXPECT_FALSE(Configuration.IsKeepFileName("notfound"));
    EXPECT_FALSE(Configuration.IsKeepFileName("test"));
}

/**
 * @tc.name: IsReserved_test_004
 * @tc.desc: Test IsReserved function with empty lists
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, IsReserved_test_004, TestSize.Level0)
{
    Configuration Configuration;
    Configuration.obfuscationRules.fileNameOption.enable = true;
    Configuration.obfuscationRules.fileNameOption.reservedFileNames.clear();
    Configuration.obfuscationRules.fileNameOption.universalReservedFileNames.clear();

    EXPECT_FALSE(Configuration.IsKeepFileName("any"));
    EXPECT_FALSE(Configuration.IsKeepFileName("test.file"));
    EXPECT_FALSE(Configuration.IsKeepFileName(""));
}

/**
 * @tc.name: IsReserved_test_005
 * @tc.desc: Test IsReserved function with complex regex patterns
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(ConfigurationTest, IsReserved_test_005, TestSize.Level0)
{
    Configuration Configuration;
    Configuration.obfuscationRules.fileNameOption.enable = true;
    Configuration.obfuscationRules.fileNameOption.reservedFileNames.clear();
    Configuration.obfuscationRules.fileNameOption.universalReservedFileNames = {
        ".*test.*",      // Contains "test"
        "^start.*",      // Starts with "start"
        ".*end$",        // Ends with "end"
        "[0-9]+\\.txt",  // Digits followed by .txt
        ".*[a-z]{3}.*"   // Contains three lowercase letters
    };

    EXPECT_TRUE(Configuration.IsKeepFileName("mytestfile"));  // Matches ".*test.*"
    EXPECT_TRUE(Configuration.IsKeepFileName("start123"));    // Matches
    EXPECT_TRUE(Configuration.IsKeepFileName("fileend"));     // Matches ".*end$"
    EXPECT_TRUE(Configuration.IsKeepFileName("123.txt"));     // Matches "[0-9]+\\.txt"
    EXPECT_TRUE(Configuration.IsKeepFileName("abc123"));      // Matches ".*[a-z]{3}.*"
    EXPECT_TRUE(Configuration.IsKeepFileName("end"));         // Matches ".*end$"
    EXPECT_TRUE(Configuration.IsKeepFileName("start"));       // Matches
    EXPECT_FALSE(Configuration.IsKeepFileName("AB"));         // No match: uppercase, less than 3 letters
    EXPECT_FALSE(Configuration.IsKeepFileName("12"));         // No match: digits but not .txt format
    EXPECT_FALSE(Configuration.IsKeepFileName("ab"));
    EXPECT_FALSE(Configuration.IsKeepFileName("a"));  // No match: only 1 letter
}
