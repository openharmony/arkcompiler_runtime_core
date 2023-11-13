/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "verifier.h"

#include <gtest/gtest.h>
#include <string>
#include <cstdlib>

#include "file.h"
#include "utils/logger.h"

using namespace testing::ext;
namespace panda::verifier {
class VerifierTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
    static constexpr uint32_t MAGIC_LEN = 8U;
    static constexpr uint32_t ABCFILE_OFFSET = 12U;
};

HWTEST_F(VerifierTest, verifier_test_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_checksum.abc";
    panda::verifier::Verifier ver {file_name};
    EXPECT_TRUE(ver.VerifyChecksum());
}

/**
* @tc.name: verifier_test_001
* @tc.desc: Verify the modified abc file checksum value function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierTest, verifier_test_002, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_checksum.abc";
    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_TRUE(ver.VerifyChecksum());
    }
    std::vector<uint8_t> bytes = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x11};
	
    constexpr char const *mode = "wbe";
    FILE *fp = fopen(file_name.c_str(), mode);
    if (fp == nullptr) {
        LOG(ERROR, VERIFIER) << file_name << ", open failed";
    }
    EXPECT_TRUE(fp != nullptr);
    fseek(fp, 0, SEEK_SET);
    fseek(fp, ABCFILE_OFFSET, SEEK_CUR);

    auto size = fwrite(bytes.data(), sizeof(decltype(bytes.back())), bytes.size(), fp);
    fclose(fp);
    fp = nullptr;
    EXPECT_TRUE(size == bytes.size());

    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_FALSE(ver.VerifyChecksum());
    }
}


/**
* @tc.name: verifier_test_003
* @tc.desc: Verify the modified checksum bitwidth abc file checksum value function.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierTest, verifier_test_003, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_checksum_bit.abc";
    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_TRUE(ver.VerifyChecksum());
    }
    std::vector<uint8_t> bytes = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

    FILE *fp = fopen(file_name.c_str(), "wbe");
    if (fp == nullptr) {
        LOG(ERROR, VERIFIER) << file_name << ", open failed";
    }
    EXPECT_TRUE(fp != nullptr);

    fseek(fp, 0, SEEK_SET);
    fseek(fp, MAGIC_LEN, SEEK_CUR);

    auto size = fwrite(bytes.data(), sizeof(decltype(bytes.back())), bytes.size(), fp);
    fclose(fp);
    fp = nullptr;
    EXPECT_TRUE(size == bytes.size());

    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_FALSE(ver.VerifyChecksum());
    }
}

};
