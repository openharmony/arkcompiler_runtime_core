/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "libabckit/cpp/abckit_cpp.h"
#include "tests/helpers/helpers.h"
#include "tests/helpers/helpers_runtime.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSModifyApiNamespacesTest : public ::testing::Test {};

// Test: test-kind=api, api=ArktsModifyApiImpl::namespaceSetName, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiNamespacesTest, NamespaceSetNameStatic)
{
    std::string input = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/namespaces/namespace_static_modify.abc";
    std::string output =
        ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/namespaces/namespace_static_modify_modified.abc";

    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(input.c_str(), &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "namespace_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_NE(ctxFinder.module, nullptr);

    helpers::NamespaceByNameContext nsCtxFinder = {nullptr, "NS1"};
    g_implI->moduleEnumerateNamespaces(ctxFinder.module, &nsCtxFinder, helpers::NamespaceByNameFinder);
    ASSERT_NE(nsCtxFinder.n, nullptr);

    auto arktsNs = g_implArkI->coreNamespaceToArktsNamespace(nsCtxFinder.n);
    g_implArkM->namespaceSetName(arktsNs, "abc");

    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->writeAbc(file, output.c_str(), output.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::AssertOpenAbc(output.c_str(), &file);

    ctxFinder = {nullptr, "namespace_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_NE(ctxFinder.module, nullptr);

    nsCtxFinder = {nullptr, "NS1"};
    g_implI->moduleEnumerateNamespaces(ctxFinder.module, &nsCtxFinder, helpers::NamespaceByNameFinder);
    EXPECT_EQ(nsCtxFinder.n, nullptr);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test