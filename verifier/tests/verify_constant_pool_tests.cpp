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
#include <stdlib.h>

#include "file.h"
#include "utils/logger.h"

using namespace testing::ext;
namespace panda::verifier{
class VerifierConstantPool : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
* @tc.name: verifier_constant_pool_001
* @tc.desc: Verify abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_001, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_constant_pool1.abc";
    panda::verifier::Verifier ver {file_name};

    EXPECT_TRUE(ver.VerifyConstantPool());
}

/**
* @tc.name: verifier_constant_pool_002
* @tc.desc: Verify the method id of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_002, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_constant_pool2.abc";
    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_TRUE(ver.VerifyConstantPool());
    }
    std::vector<uint8_t> string_id = {0x0c, 0x00};
    
    uint32_t code_id = 0x02f0; // the code id which contains method id in abc file

    // For format in this instruction, the method id needs to be offset by one byte
    long int method_id = static_cast<long int>(code_id)+1;
	
    constexpr char const *mode = "wbe";
    FILE *fp = fopen(file_name.c_str(), mode);
    if (fp == nullptr) {
        LOG(ERROR, VERIFIER) << "Failed to open file " << file_name;
    }
    EXPECT_TRUE(fp != nullptr);

    fseek(fp, 0, SEEK_SET);
    fseek(fp, method_id, SEEK_CUR);

    auto size = fwrite(string_id.data(), sizeof(decltype(string_id.back())), string_id.size(), fp);
    fclose(fp);
    fp = nullptr;
    EXPECT_TRUE(size == string_id.size());

    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_FALSE(ver.VerifyConstantPool());
    }
}

/**
* @tc.name: verifier_constant_pool_003
* @tc.desc: Verify the literal id of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_003, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_constant_pool3.abc";
    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_TRUE(ver.VerifyConstantPool());
    }
    std::vector<uint8_t> string_id = {0x00, 0x01};
    
    uint32_t code_id = 0x0306; // the code id which contains literal id in abc file

    // For format in this instruction, the literal id needs to be offset by three byte
    long int literal_id = static_cast<long int>(code_id)+3;
	
    constexpr char const *mode = "wbe";
    FILE *fp = fopen(file_name.c_str(), mode);
    if (fp == nullptr) {
        LOG(ERROR, VERIFIER) << file_name << ",open fail";
    }
    EXPECT_TRUE(fp != nullptr);
    fseek(fp, 0, SEEK_SET);
    fseek(fp, literal_id, SEEK_CUR);

    auto size = fwrite(string_id.data(), sizeof(decltype(string_id.back())), string_id.size(), fp);
    fclose(fp);
    fp = nullptr;
    EXPECT_TRUE(size == string_id.size());

    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_FALSE(ver.VerifyConstantPool());
    }
}

/**
* @tc.name: verifier_constant_pool_004
* @tc.desc: Verify the string id of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_004, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_constant_pool4.abc";
    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_TRUE(ver.VerifyConstantPool());
    }
    std::vector<uint8_t> new_string_id = {0x00, 0x01};
    
    uint32_t code_id = 0x0322; // the code id which contains string id in abc file

    // For format in this instruction, the literal id needs to be offset by three byte
    long int string_id = static_cast<long int>(code_id)+3;
	
    constexpr char const *mode = "wbe";
    FILE *fp = fopen(file_name.c_str(), mode);
    if (fp == nullptr) {
        LOG(ERROR, VERIFIER) << file_name << ",open fail";
    }
    EXPECT_TRUE(fp != nullptr);
    fseek(fp, 0, SEEK_SET);
    fseek(fp, string_id, SEEK_CUR);

    auto size = fwrite(new_string_id.data(), sizeof(decltype(new_string_id.back())), new_string_id.size(), fp);
    fclose(fp);
    fp = nullptr;
    EXPECT_TRUE(size == new_string_id.size());

    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_FALSE(ver.VerifyConstantPool());
    }
}

/**
* @tc.name: verifier_constant_pool_005
* @tc.desc: Verify the method content of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_005, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_constant_pool5.abc";
    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_TRUE(ver.VerifyConstantPool());
    }
    std::vector<uint8_t> method_content = {0x00, 0x02};
    
    uint32_t method_id = 0x000d;
	
    constexpr char const *mode = "wbe";
    FILE *fp = fopen(file_name.c_str(), mode);
    if (fp == nullptr) {
        LOG(ERROR, VERIFIER) << file_name << ",open fail";
    }
    EXPECT_TRUE(fp != nullptr);
    fseek(fp, 0, SEEK_SET);
    fseek(fp, method_id, SEEK_CUR);

    auto size = fwrite(method_content.data(), sizeof(decltype(method_content.back())), method_content.size(), fp);
    fclose(fp);
    fp = nullptr;
    EXPECT_TRUE(size == method_content.size());

    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_FALSE(ver.Verify());
    }
}

/**
* @tc.name: verifier_constant_pool_006
* @tc.desc: Verify the literal content of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_006, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_constant_pool6.abc";
    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_TRUE(ver.VerifyConstantPool());
    }
    std::vector<uint8_t> literal_content = {0x00, 0x00};
    
    uint32_t literal_id = 0x000f;
	
    constexpr char const *mode = "wbe";
    FILE *fp = fopen(file_name.c_str(), mode);
    if (fp == nullptr) {
        LOG(ERROR, VERIFIER) << file_name << ",open fail";
    }
    EXPECT_TRUE(fp != nullptr);
    fseek(fp, 0, SEEK_SET);
    fseek(fp, literal_id, SEEK_CUR);

    auto size = fwrite(literal_content.data(), sizeof(decltype(literal_content.back())), literal_content.size(), fp);
    fclose(fp);
    fp = nullptr;
    EXPECT_TRUE(size == literal_content.size());

    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_FALSE(ver.Verify());
    }
}

/**
* @tc.name: verifier_constant_pool_007
* @tc.desc: Verify the string content of the abc file.
* @tc.type: FUNC
* @tc.require: file path and name
*/
HWTEST_F(VerifierConstantPool, verifier_constant_pool_007, TestSize.Level1)
{
    const std::string file_name = GRAPH_TEST_ABC_DIR "test_constant_pool7.abc";
    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_TRUE(ver.VerifyConstantPool());
    }
    std::vector<char> string_content = {'a', 'b'};
    
    uint32_t string_id = 0x0c00;
	
    constexpr char const *mode = "wbe";
    FILE *fp = fopen(file_name.c_str(), mode);
    if (fp == nullptr) {
        LOG(ERROR, VERIFIER) << file_name << ",open fail";
    }
    EXPECT_TRUE(fp != nullptr);
    fseek(fp, 0, SEEK_SET);
    fseek(fp, string_id, SEEK_CUR);

    auto size = fwrite(string_content.data(), sizeof(decltype(string_content.back())), string_content.size(), fp);
    fclose(fp);
    fp = nullptr;
    EXPECT_TRUE(size == string_content.size());

    {
        panda::verifier::Verifier ver {file_name};
        EXPECT_FALSE(ver.Verify());
    }
}

};
