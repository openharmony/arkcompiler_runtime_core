/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreement in writing, software distributed under the License is distributed
 * on an "AS IS BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 * for the specific language governing permissions and limitations under the License.
 */

/*
 * Tests for OHMurl (@normalized:) import handling in
 * moduleAddImportFromArktsV1ToArktsV1 on an abc containing OHMurl requests
 * (generated with is_ohmurl merge, modules are ARK_TS_V1).
 *
 * Covered scenarios:
 *   - CarrierSameNameAsExistingPlaceholder: creating an ArkTS external
 *     module whose name matches an on-open placeholder must not crash with
 *     bad_variant_access (an on-open placeholder holds the variant's default
 *     alternative - a null Js impl - which GetArkTSImpl() used to throw on)
 *     and must align its target for SAME_TARGET checks
 *   - FreshOhmurlImportResolvesToEntity: importing via an OHMurl carrier
 *     creates a request with the OHMurl verbatim and resolves to the
 *     abc-internal entity
 */

#include <gtest/gtest.h>
#include <cstring>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/c/statuses.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules_ohmurl/modules_ohmurl.abc";

class LibAbcKitArkTSModifyApiModulesOhmurlTest : public ::testing::Test {};

static AbckitCoreModule *FindModuleByName(AbckitFile *file, const std::string &name)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, name.c_str()};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    return ctxFinder.module;
}

// Test: test-kind=api, api=ArktsModifyApiImpl::fileAddExternalModuleArktsV1, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSModifyApiModulesOhmurlTest, CarrierSameNameAsExistingPlaceholder)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    // Carrier name matches the on-open placeholder. Used to throw
    // bad_variant_access (the placeholder holds the variant's default
    // alternative, and GetArkTSImpl() on it is an invalid variant access).
    AbckitArktsV1ExternalModuleCreateParams params {};
    params.name = "@normalized:N&&&modules/greet&";
    auto *carrier = g_implArkM->fileAddExternalModuleArktsV1(file, &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(carrier, nullptr);

    // The placeholder was adopted: it has an ArkTS target now (was UNKNOWN),
    // so SAME_TARGET checks in addImport pass
    auto *coreCarrier = g_implArkI->arktsModuleToCoreModule(carrier);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(g_implI->moduleGetTarget(coreCarrier), ABCKIT_TARGET_ARK_TS_V1);

    // And it is usable for import right away
    auto *entry = FindModuleByName(file, "&modules_ohmurl&");
    ASSERT_NE(entry, nullptr);
    AbckitArktsImportFromDynamicModuleCreateParams ip {};
    ip.name = "regularFunc";
    ip.alias = "regularFunc";
    auto *newImport =
        g_implArkM->moduleAddImportFromArktsV1ToArktsV1(g_implArkI->coreModuleToArktsModule(entry), carrier, &ip);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newImport, nullptr);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleAddImportFromArktsV1ToArktsV1, abc-kind=ArkTS1, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSModifyApiModulesOhmurlTest, FreshOhmurlImportResolvesToEntity)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    auto *entry = FindModuleByName(file, "&modules_ohmurl&");
    ASSERT_NE(entry, nullptr);

    AbckitArktsV1ExternalModuleCreateParams params {};
    params.name = "@normalized:N&&&modules/greet&";
    auto *carrier = g_implArkM->fileAddExternalModuleArktsV1(file, &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(carrier, nullptr);

    AbckitArktsImportFromDynamicModuleCreateParams ip {};
    ip.name = "regularFunc";
    ip.alias = "regularFunc";
    auto *newImport =
        g_implArkM->moduleAddImportFromArktsV1ToArktsV1(g_implArkI->coreModuleToArktsModule(entry), carrier, &ip);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newImport, nullptr);

    // The import resolves to the abc-internal entity, not the carrier placeholder
    auto *coreImport = g_implArkI->arktsImportDescriptorToCoreImportDescriptor(newImport);
    auto *imported = g_implI->importDescriptorGetImportedModule(coreImport);
    ASSERT_NE(imported, nullptr);
    ASSERT_EQ(g_implI->moduleGetTarget(imported), ABCKIT_TARGET_ARK_TS_V1);
    ASSERT_FALSE(g_implI->moduleIsExternal(imported));

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
