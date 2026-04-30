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

#include <cstdio>
#include <gtest/gtest.h>
#include "assembly-emitter.h"
#include "assembly-parser.h"
#include "abc2program/abc2program_driver.h"

namespace ark::abc2program::test {

class NovalueAbc2ProgramTest : public testing::Test {
protected:
    void SetUp() override
    {
        auto source = R"(
            .record N <external>
            .function novalue foo(N a0) <static, access.function=public> {
                return.void
            }
            .function novalue bar() <static, access.function=public> {
                return.void
            }
        )";
        auto res = ark::pandasm::Parser().Parse(source);
        ASSERT_TRUE(res);
        tmpFile_ = std::tmpnam(nullptr);
        ASSERT_TRUE(ark::pandasm::AsmEmitter::Emit(tmpFile_, res.Value()));
    }

    void TearDown() override
    {
        if (!tmpFile_.empty()) {
            std::remove(tmpFile_.c_str());
        }
    }

    std::string tmpFile_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(NovalueAbc2ProgramTest, ReturnTypeIsX)
{
    Abc2ProgramDriver driver;
    ASSERT_TRUE(driver.Compile(tmpFile_));
    const auto &prog = driver.GetProgram();

    size_t count = 0;
    for (const auto &[name, func] : prog.functionStaticTable) {
        ASSERT_EQ(func.returnType.GetComponentName(), "novalue");
        ASSERT_EQ(func.returnType.GetRank(), 0);
        ++count;
    }
    ASSERT_EQ(count, 2U);
}

TEST_F(NovalueAbc2ProgramTest, ParamIsReferenceNotX)
{
    Abc2ProgramDriver driver;
    ASSERT_TRUE(driver.Compile(tmpFile_));
    const auto &prog = driver.GetProgram();

    bool found = false;
    for (const auto &[name, func] : prog.functionStaticTable) {
        if (name.find("foo") != std::string::npos) {
            ASSERT_EQ(func.params.size(), 1);
            ASSERT_EQ(func.params[0].type.GetComponentName(), "N");
            found = true;
        }
    }
    ASSERT_TRUE(found);
}

}  // namespace ark::abc2program::test
