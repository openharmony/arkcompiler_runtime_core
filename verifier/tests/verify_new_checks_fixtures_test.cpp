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

#include "verifier.h"

#include <gtest/gtest.h>
#include <string>

using namespace testing::ext;

namespace panda::verifier {
class VerifierNewChecksFixturesTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    static void ExpectFileValid(const std::string &file_name)
    {
        panda::verifier::Verifier ver {file_name};
        ASSERT_TRUE(ver.CollectIdInfos());
        EXPECT_TRUE(ver.VerifyChecksum());
        EXPECT_TRUE(ver.VerifyConstantPoolIndex());
        EXPECT_TRUE(ver.VerifyConstantPoolContent());
        EXPECT_TRUE(ver.VerifyRegisterIndex());
        EXPECT_TRUE(ver.Verify());
    }
};

/**
 * @tc.name: verifier_fixtures_001
 * @tc.desc: Verify switch and loop bytecode.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_001, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_switch_loops.abc");
}

/**
 * @tc.name: verifier_fixtures_002
 * @tc.desc: Verify Map, Set and WeakMap bytecode.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_002, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_map_set.abc");
}

/**
 * @tc.name: verifier_fixtures_003
 * @tc.desc: Verify async methods and Promise chains.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_003, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_async_methods.abc");
}

/**
 * @tc.name: verifier_fixtures_004
 * @tc.desc: Verify class heritage and super calls.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_004, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_class_heritage.abc");
}

/**
 * @tc.name: verifier_fixtures_005
 * @tc.desc: Verify private instance and static fields.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_005, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_private_fields.abc");
}

/**
 * @tc.name: verifier_fixtures_006
 * @tc.desc: Verify regex and template string bytecode.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_006, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_regex_template.abc");
}

/**
 * @tc.name: verifier_fixtures_007
 * @tc.desc: Verify iterator and for-of bytecode.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_007, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_iterators.abc");
}

/**
 * @tc.name: verifier_fixtures_008
 * @tc.desc: Verify spread call and destructure bytecode.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_008, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_destructure_call.abc");
}

/**
 * @tc.name: verifier_fixtures_009
 * @tc.desc: Verify class methods fixture through public APIs.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_009, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_class_methods.abc");
}

/**
 * @tc.name: verifier_fixtures_010
 * @tc.desc: Verify nested try fixture through public APIs.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_010, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_try_nested.abc");
}

/**
 * @tc.name: verifier_fixtures_011
 * @tc.desc: Verify generator fixture through public APIs.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_011, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_generators.abc");
}

/**
 * @tc.name: verifier_fixtures_012
 * @tc.desc: Verify object literal fixture through public APIs.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_012, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_object_literal.abc");
}

/**
 * @tc.name: verifier_fixtures_013
 * @tc.desc: Verify named export module fixture.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_013, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_named_exports.abc");
}

/**
 * @tc.name: verifier_fixtures_014
 * @tc.desc: Verify ic slot overflow fixture through public APIs.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_014, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_ic_slots_overflow.abc");
}

/**
 * @tc.name: verifier_fixtures_015
 * @tc.desc: Verify literal array ts fixture through public APIs.
 * @tc.type: FUNC
 */
HWTEST_F(VerifierNewChecksFixturesTest, verifier_fixtures_015, TestSize.Level1)
{
    ExpectFileValid(GRAPH_TEST_ABC_DIR "test_literal_array.abc");
}
}  // namespace panda::verifier
