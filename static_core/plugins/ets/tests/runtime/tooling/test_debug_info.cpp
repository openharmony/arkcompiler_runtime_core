/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "debug_info_extractor.h"
#include "file.h"
#include "gtest/gtest.h"
#include "os/filesystem.h"

namespace ark::test {

class DebugInfoTest : public testing::Test {
public:
    void SetUp() override {}

    void TearDown() override {}

protected:
    std::string_view GetTestFile()
    {
        auto testFile = std::getenv("TEST_FILE");
        if (testFile == nullptr) {
            std::cerr << "Environment variable 'TEST_FILE' must be set" << std::endl;
            std::abort();
        }
        return testFile;
    }
};

TEST_F(DebugInfoTest, GetSourceFile)
{
    auto file = panda_file::OpenPandaFile(GetTestFile());
    panda_file::DebugInfoExtractor extractor(file.get());
    auto methods = extractor.GetMethodIdList();
    ASSERT_FALSE(methods.empty());
    auto sourceFilePath = extractor.GetSourceFile(methods[0]);
    ASSERT_TRUE(os::IsFileExists(sourceFilePath));
    ASSERT_TRUE(os::GetAbsolutePath(sourceFilePath) == sourceFilePath);
}

}  // namespace ark::test
