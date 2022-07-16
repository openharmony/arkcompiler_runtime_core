/**
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_TESTS_OPTIONS_TEST_BASE_H
#define PANDA_RUNTIME_TESTS_OPTIONS_TEST_BASE_H

#include <gtest/gtest.h>
#include <vector>

#include "runtime/include/runtime_options.h"
#include "libpandabase/utils/pandargs.h"

namespace panda::test {
class RuntimeOptionsTestBase : public testing::Test {
public:
    RuntimeOptionsTestBase() : runtime_options_("AAA") {}
    virtual ~RuntimeOptionsTestBase() = default;

    void SetUp() override
    {
        runtime_options_.AddOptions(&pa_parser_);
        LoadCorrectOptionsList();
    }

    void TearDown() override {}

    panda::PandArgParser *GetParser()
    {
        return &pa_parser_;
    }

    const std::vector<std::string> &GetCorrectOptionsList() const
    {
        return correct_options_list_;
    }

    RuntimeOptions *GetRuntimeOptions()
    {
        return &runtime_options_;
    }

protected:
    void AddTestingOption(std::string opt, std::string value)
    {
        correct_options_list_.push_back("--" + opt + "=" + value);
    }

private:
    virtual void LoadCorrectOptionsList() = 0;

    RuntimeOptions runtime_options_;
    panda::PandArgParser pa_parser_;
    std::vector<std::string> correct_options_list_;
};
}  // namespace panda::test

#endif  // PANDA_RUNTIME_TESTS_OPTIONS_TEST_BASE_H
