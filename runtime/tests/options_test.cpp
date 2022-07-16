/**
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

#include <gtest/gtest.h>
#include <vector>

#include "runtime/include/runtime_options.h"
#include "libpandabase/utils/pandargs.h"
#include "options_test_base.h"

namespace panda::test {

class RuntimeOptionsTest : public RuntimeOptionsTestBase {
public:
    RuntimeOptionsTest() = default;
    ~RuntimeOptionsTest() = default;

private:
    void LoadCorrectOptionsList() override;
};

void RuntimeOptionsTest::LoadCorrectOptionsList()
{
    AddTestingOption("runtime-compressed-strings-enabled", "true");
    AddTestingOption("compiler-enable-jit", "true");
    AddTestingOption("sigquit-flag", "1234");
    AddTestingOption("dfx-log", "1234");
    AddTestingOption("start-as-zygote", "true");
    AddTestingOption("gc-trigger-type", "no-gc-for-start-up");
}

// Testing that generator correctly generate options for different languages
TEST_F(RuntimeOptionsTest, TestCorrectOptions)
{
    ASSERT_TRUE(GetParser()->Parse(GetCorrectOptionsList()));
}

// Testing that panda::PandArgParser detect invalid options and types
TEST_F(RuntimeOptionsTest, TestIncorrectOptions)
{
    std::vector<std::string> invalid_options;
    invalid_options.push_back("--InvalidOptionThatNotExistAndNeverWillBeAdded=true");
    ASSERT_FALSE(GetParser()->Parse(invalid_options));
    ASSERT_EQ(GetParser()->GetErrorString(),
              "pandargs: Invalid option \"InvalidOptionThatNotExistAndNeverWillBeAdded\"\n");
}

TEST_F(RuntimeOptionsTest, TestTailArgumets)
{
    GetParser()->EnableTail();
    std::vector<std::string> options_vector = GetCorrectOptionsList();
    panda::PandArg<std::string> file("file", "", "path to pandafile");

    options_vector.push_back("tail1");
    GetParser()->PushBackTail(&file);
    ASSERT_TRUE(GetParser()->Parse(options_vector));

    options_vector.push_back("tail2");
    ASSERT_FALSE(GetParser()->Parse(options_vector));
    ASSERT_EQ(GetParser()->GetErrorString(), "pandargs: Too many tail arguments given\n");

    GetParser()->DisableTail();
    options_vector.pop_back();
    ASSERT_FALSE(GetParser()->Parse(options_vector));
    ASSERT_EQ(GetParser()->GetErrorString(),
              "pandargs: Tail arguments are not enabled\npandargs: Tail found at literal \"tail1\"\n");
}

TEST_F(RuntimeOptionsTest, TestLangSpecificOptions)
{
    ASSERT_TRUE(GetParser()->Parse(GetCorrectOptionsList()));
    ASSERT_EQ(GetRuntimeOptions()->GetGcTriggerType("core"), "no-gc-for-start-up");
}

}  // namespace panda::test
