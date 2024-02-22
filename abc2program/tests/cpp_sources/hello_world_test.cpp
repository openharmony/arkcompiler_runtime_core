/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include <iostream>
#include <vector>
#include "abc2program_driver.h"

using namespace testing::ext;

namespace panda::abc2program {

static const std::string HELLO_WORLD_ABC_TEST_FILE_NAME = GRAPH_TEST_ABC_DIR "HelloWorld.abc";

class Abc2ProgramHelloWorldTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        (void)driver_.Compile(HELLO_WORLD_ABC_TEST_FILE_NAME);
        prog_ = &(driver_.GetProgram());
    }

    void TearDown() {}

    template <typename T>
    bool ValidateString(const T &strings, const T &expected_strings)
    {
        if (strings.size() != expected_strings.size()) {
            return false;
        }
        for (const std::string &expected_string : expected_strings) {
            const auto string_iter = std::find(strings.begin(), strings.end(), expected_string);
            if (string_iter == strings.end()) {
                return false;
            }
        }
        return true;
    }

    Abc2ProgramDriver driver_;
    const pandasm::Program *prog_ = nullptr;
};

/**
 * @tc.name: abc2program_hello_world_test_lang
 * @tc.desc: get program language.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_lang, TestSize.Level1)
{
    panda_file::SourceLang expected_lang = panda_file::SourceLang::ECMASCRIPT;
    bool lang_matched = (prog_->lang == expected_lang);
    EXPECT_TRUE(lang_matched);
}

/**
 * @tc.name: abc2program_hello_world_test_record_table
 * @tc.desc: get program record_table.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_record_table, TestSize.Level1)
{
    std::vector<std::string> expected_record_names = {"_ESSlotNumberAnnotation", "_ESTypeAnnotation",
                                                      "_ESTypeInfoRecord", "_GLOBAL"};
    panda_file::SourceLang expected_lang = panda_file::SourceLang::ECMASCRIPT;
    std::vector<std::string> record_names;
    for (const auto &it : prog_->record_table) {
        EXPECT_TRUE(it.second.language == expected_lang);
        EXPECT_TRUE(it.second.source_file == "");
        EXPECT_TRUE(it.first == it.second.name);
        record_names.emplace_back(it.first);
    }
    ValidateString(record_names, expected_record_names);
}

/**
 * @tc.name: abc2program_hello_world_test_fields
 * @tc.desc: get program record_table.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_fields, TestSize.Level1)
{
    for (const auto &it : prog_->record_table) {
        EXPECT_TRUE(it.second.field_list.size() == 0);
    }
}

/**
 * @tc.name: abc2program_hello_world_test_strings
 * @tc.desc: get existed string.
 * @tc.type: FUNC
 * @tc.require: file path and name
 */
HWTEST_F(Abc2ProgramHelloWorldTest, abc2program_hello_world_test_strings, TestSize.Level1)
{
    std::set<std::string> expected_strings = {"HelloWorld", "prototype", "str"};
    bool string_matched = ValidateString(prog_->strings, expected_strings);
    EXPECT_TRUE(string_matched);
}

};  // panda::abc2program