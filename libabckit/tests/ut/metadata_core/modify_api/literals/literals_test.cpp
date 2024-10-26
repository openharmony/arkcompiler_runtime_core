/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <cstring>

#include "libabckit/include/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/include/c/metadata_core.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitModifyApiLiteralsTest : public ::testing::Test {};

// Test: test-kind=api, api=ModifyApiImpl::createLiteralBool, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralBool_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralU8, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralU8_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU8(file, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralU16, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralU16_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU16(file, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralMethodAffiliate, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralMethodAffiliate_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralMethodAffiliate(file, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralU32, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralU32_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU32(file, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralU64, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralU64_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU64(file, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralFloat, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralFloat_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralFloat(file, 1.0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralDouble, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralDouble_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralDouble(file, 1.0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralLiteralArray, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralLiteralArray_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    const double val1 = 0.7;
    const int val2 = 7U;
    std::array<AbckitLiteral *, 2U> statlitarr = {g_implM->createLiteralDouble(file, val1),
                                                  g_implM->createLiteralU32(file, val2)};
    AbckitLiteralArray *litarr = g_implM->createLiteralArray(file, statlitarr.data(), 2U);
    AbckitLiteral *res = g_implM->createLiteralLiteralArray(file, litarr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralString, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralString_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralString(file, "asdf");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralMethod, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralMethod_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "foo"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralArray, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralArray_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    auto arr = std::vector<AbckitLiteral *>();
    AbckitLiteral *res1 = g_implM->createLiteralString(file, "asdf");
    AbckitLiteral *res2 = g_implM->createLiteralDouble(file, 1.0);
    arr.emplace_back(res1);
    arr.emplace_back(res2);
    auto litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(litArr, nullptr);

    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralArray, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralArray_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_dynamic.abc", &file);
    auto arr = std::vector<AbckitLiteral *>();
    AbckitLiteral *res1 = g_implM->createLiteralString(file, "asdf");
    AbckitLiteral *res2 = g_implM->createLiteralDouble(file, 1.0);
    arr.emplace_back(res1);
    arr.emplace_back(res2);
    auto litArr = g_implM->createLiteralArray(file, arr.data(), arr.size());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(litArr, nullptr);

    // Write output file
    g_impl->writeAbc(file, "/tmp/literals_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralBool, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralBool_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralBool(file, true);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralU8, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralU8_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU8(file, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralU16, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralU16_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU16(file, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralMethodAffiliate, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralMethodAffiliate_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralMethodAffiliate(file, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralU32, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralU32_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU32(file, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralU64, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralU64_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralU64(file, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralFloat, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralFloat_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralFloat(file, 1.0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralDouble, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralDouble_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralDouble(file, 1.0);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralLiteralArray, abc-kind=ArkTS2, category=negative
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralLiteralArray_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    const double val1 = 0.7;
    const int val2 = 7U;
    std::array<AbckitLiteral *, 2U> statlitarr = {g_implM->createLiteralDouble(file, val1),
                                                  g_implM->createLiteralU32(file, val2)};
    AbckitLiteralArray *litarr = g_implM->createLiteralArray(file, statlitarr.data(), 2U);
    AbckitLiteral *res = g_implM->createLiteralLiteralArray(file, litarr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_UNSUPPORTED);
    ASSERT_EQ(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralString, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralString_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralString(file, "asdf");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralMethod, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitModifyApiLiteralsTest, CreateLiteralMethod_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static.abc", &file);
    AbckitLiteral *res = g_implM->createLiteralMethod(file, helpers::FindMethodByName(file, "foo"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(res, nullptr);
    // Write output file
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/metadata_core/modify_api/literals/literals_static_modified.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test