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

#ifdef STATIC_ABC_VERIFIER_USE_ARKFILE

#include "verifier.h"
#include "test_helper.h"

#include <gtest/gtest.h>
#include <fstream>
#include <cstring>

namespace ark::static_verifier {

class DeepVerifierTest : public testing::Test {};

#ifdef DEEP_TEST_ABC_DIR

static std::string GetTestAbcPath(const char *name)
{
    return std::string(DEEP_TEST_ABC_DIR) + "/" + name;
}

// ===================== Real ABC file tests =====================

TEST_F(DeepVerifierTest, EtsStdlibPassesDeepVerification)
{
    auto path = GetTestAbcPath("plugins/ets/etsstdlib.abc");
    std::ifstream f(path);
    if (!f.good()) {
        GTEST_SKIP() << "Test file not available: " << path;
    }

    StaticAbcVerifier verifier(path);
    auto result = verifier.Verify();
    EXPECT_TRUE(result.valid) << "etsstdlib.abc should pass all checks";
    if (!result.valid) {
        for (const auto &err : result.errors) {
            std::cerr << "  [E" << static_cast<int>(err.code) << "] " << err.message << "\n";
        }
    }
}

TEST_F(DeepVerifierTest, ArkStdlibPassesDeepVerification)
{
    auto path = GetTestAbcPath("pandastdlib/arkstdlib.abc");
    std::ifstream f(path);
    if (!f.good()) {
        GTEST_SKIP() << "Test file not available: " << path;
    }

    StaticAbcVerifier verifier(path);
    auto result = verifier.Verify();
    EXPECT_TRUE(result.valid) << "arkstdlib.abc should pass all checks";
    if (!result.valid) {
        for (const auto &err : result.errors) {
            std::cerr << "  [E" << static_cast<int>(err.code) << "] " << err.message << "\n";
        }
    }
}

#endif  // DEEP_TEST_ABC_DIR

// ===================== Synthetic tests for deep error paths =====================

TEST_F(DeepVerifierTest, MinimalValidAbcPassesDeepChecks)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    auto result = verifier.Verify();
    EXPECT_TRUE(result.valid);
}

TEST_F(DeepVerifierTest, DeepChecksSkippedWhenBasicFails)
{
    auto data = test::BuildMinimalValidAbc();
    data[0] = 'X';

    StaticAbcVerifier verifier(data.data(), data.size());
    auto result = verifier.Verify();
    EXPECT_FALSE(result.valid);

    bool hasDeepError = false;
    for (const auto &err : result.errors) {
        if (static_cast<int>(err.code) >= static_cast<int>(ErrorCode::INVALID_SOURCE_LANG)) {
            hasDeepError = true;
        }
    }
    EXPECT_FALSE(hasDeepError) << "Deep checks should be skipped when basic checks fail";
}

TEST_F(DeepVerifierTest, VerifyClassesDeepNoClasses)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyClassesDeep());
}

TEST_F(DeepVerifierTest, VerifyMethodsAndCodeNoClasses)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyMethodsAndCode());
}

TEST_F(DeepVerifierTest, VerifyLiteralArraysDeepNoArrays)
{
    auto data = test::BuildMinimalValidAbc();
    StaticAbcVerifier verifier(data.data(), data.size());
    EXPECT_TRUE(verifier.VerifyLiteralArraysDeep());
}

TEST_F(DeepVerifierTest, SyntheticClassesSkipDeepChecks)
{
    auto data = test::BuildAbcWithClasses(3);
    StaticAbcVerifier verifier(data.data(), data.size());
    auto result = verifier.Verify();
    EXPECT_TRUE(result.valid);
}

}  // namespace ark::static_verifier

#endif  // STATIC_ABC_VERIFIER_USE_ARKFILE
