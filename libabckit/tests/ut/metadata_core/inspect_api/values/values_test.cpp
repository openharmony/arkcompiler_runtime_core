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
#include <cstddef>
#include <cstring>

#include "libabckit/include/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/include/c/metadata_core.h"
#include "libabckit/src/metadata_inspect_impl.h"  // NOTE(mredkin)

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitInspectApiValuesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::valueGetU1, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetU1_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    auto val = g_implI->valueGetU1(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(val);
    // Write output file
    g_impl->writeAbc(file, "/tmp/values_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetFile, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetFile_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(g_implI->valueGetFile(res), file);
    // Write output file
    g_impl->writeAbc(file, "/tmp/values_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetDouble, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetDouble_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    const double implVal = 1.2;
    auto *res = g_implM->createValueDouble(file, implVal);
    auto val = g_implI->valueGetDouble(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, implVal);
    // Write output file
    g_impl->writeAbc(file, "/tmp/values_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetString, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetString_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueString(file, "test");
    auto val = g_implI->valueGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->impl, "test");
    // Write output file
    g_impl->writeAbc(file, "/tmp/values_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralArrayValue, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, CreateLiteralArrayValue_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_dynamic.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test"));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2U);
    auto *litArr = g_implI->arrayValueGetLiteralArray(arr);
    std::vector<AbckitLiteral *> newArr;
    g_implI->literalArrayEnumerateElements(litArr, &newArr, [](AbckitFile *, AbckitLiteral *lit, void *data) {
        (reinterpret_cast<std::vector<AbckitLiteral *> *>(data))->emplace_back(lit);
        return true;
    });

    ASSERT_EQ(newArr.size(), 2U);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arr, nullptr);

    ASSERT_EQ(g_implI->literalGetString(newArr[0])->impl, "test");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_TRUE(g_implI->literalGetBool(newArr[1]));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // Write output file
    g_impl->writeAbc(file, "/tmp/values_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::arrayValueGetLiteralArray, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ArrayValueGetLiteralArray_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_dynamic.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test"));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2);
    auto *larr = g_implI->arrayValueGetLiteralArray(arr);
    size_t counter = 0;
    g_implI->literalArrayEnumerateElements(larr, &counter, [](AbckitFile *, AbckitLiteral *, void *data) {
        (*(reinterpret_cast<uint32_t *>(data)))++;
        return true;
    });
    ASSERT_EQ(counter, 2U);

    // Write output file
    g_impl->writeAbc(file, "/tmp/values_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetType, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetType_1)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_dynamic.abc", &file);
    auto *res = g_implM->createValueString(file, "test");
    auto val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_STRING);
    res = g_implM->createValueU1(file, true);
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_U1);
    const double implVal = 1.2;
    res = g_implM->createValueDouble(file, implVal);
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_F64);
    // Write output file
    g_impl->writeAbc(file, "/tmp/values_dynamic.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetU1, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetU1_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    auto val = g_implI->valueGetU1(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(val);
    // Write output file
    g_impl->writeAbc(file, "/tmp/values_static.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetDouble, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetDouble_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    const double implVal = 1.2;
    auto *res = g_implM->createValueDouble(file, implVal);
    auto val = g_implI->valueGetDouble(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val, implVal);
    // Write output file
    g_impl->writeAbc(file, "/tmp/values_static.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetString, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetString_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueString(file, "test");
    auto val = g_implI->valueGetString(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->impl, "test");
    // Write output file
    g_impl->writeAbc(file, "/tmp/values_static.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ModifyApiImpl::createLiteralArrayValue, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, CreateLiteralArrayValue_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_static.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test"));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2);

    ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(arr, nullptr);

    // Write output file
    g_impl->writeAbc(file, "/tmp/values_static.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetFile, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetFile_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueU1(file, true);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(g_implI->valueGetFile(res), file);
    // Write output file
    g_impl->writeAbc(file, "/tmp/values_static.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::arrayValueGetLiteralArray, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ArrayValueGetLiteralArray_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/modify_api/values/values_static.abc", &file);
    std::vector<AbckitValue *> abcArr;
    abcArr.emplace_back(g_implM->createValueString(file, "test"));
    abcArr.emplace_back(g_implM->createValueU1(file, true));
    auto *arr = g_implM->createLiteralArrayValue(file, abcArr.data(), 2);
    auto *larr = g_implI->arrayValueGetLiteralArray(arr);

    ASSERT_NE(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(larr, nullptr);

    // Write output file
    g_impl->writeAbc(file, "/tmp/values_static.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::valueGetType, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitInspectApiValuesTest, ValueGetType_2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/values/values_static.abc", &file);
    auto *res = g_implM->createValueString(file, "test");
    auto val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_STRING);
    res = g_implM->createValueU1(file, true);
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_U1);
    const double implVal = 1.2;
    res = g_implM->createValueDouble(file, implVal);
    val = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(val->id, ABCKIT_TYPE_ID_F64);
    // Write output file
    g_impl->writeAbc(file, "/tmp/values_static.abc");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test