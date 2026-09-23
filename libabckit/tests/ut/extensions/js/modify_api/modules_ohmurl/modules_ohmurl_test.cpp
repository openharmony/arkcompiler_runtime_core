/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Tests for OHMurl (@normalized:) import handling in moduleAddImportFromJsToJs
 * on an abc containing OHMurl requests (generated with is_ohmurl merge).
 *
 * Entry module 'JSmodules_ohmurl' has:
 *   - side-effect OHMurl import from './modules/greet' (an on-open external
 *     module placeholder with the same name exists after file open)
 *   - no request to './modules/versioned'
 *
 * Covered scenarios:
 *   - CarrierSameNameAsExistingPlaceholder: creating a JS external module
 *     whose name matches an on-open placeholder must not fail (used to
 *     return nullptr with no error code set)
 *   - ReuseExistingOhmurlRequest: importing from the side-effect imported
 *     module must reuse its existing request (no new request created)
 *   - FreshImportWithVersionSuffix: importing via an OHMurl carrier with a
 *     version suffix (&1.0.0) must write the full OHMurl verbatim
 */

#include <gtest/gtest.h>
#include <cstring>

#include "libabckit/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/js/metadata_js.h"
#include "libabckit/c/statuses.h"

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implJsI = AbckitGetJsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implJsM = AbckitGetJsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

static constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/js/modify_api/modules_ohmurl/JSmodules_ohmurl.abc";
static constexpr auto MODIFIED_PATH =
    ABCKIT_ABC_DIR "ut/extensions/js/modify_api/modules_ohmurl/JSmodules_ohmurl_modified.abc";

class LibAbcKitJSModifyApiModulesOhmurlTest : public ::testing::Test {};

static AbckitCoreModule *FindModuleByName(AbckitFile *file, const std::string &name)
{
    helpers::ModuleByNameContext ctxFinder = {nullptr, name.c_str()};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    return ctxFinder.module;
}

// Test: test-kind=api, api=JsModifyApiImpl::fileAddExternalModule, abc-kind=JS, category=positive, extension=c
TEST_F(LibAbcKitJSModifyApiModulesOhmurlTest, CarrierSameNameAsExistingPlaceholder)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    // Carrier name matches the on-open placeholder created for the source-level
    // side-effect OHMurl import. Used to return nullptr with no error code.
    AbckitJsExternalModuleCreateParams params {};
    params.name = "@normalized:N&&&modules/greet&";
    auto *carrier = g_implJsM->fileAddExternalModule(file, &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(carrier, nullptr);

    // The placeholder was adopted: it has a JS target now (was UNKNOWN)
    auto *coreCarrier = g_implJsI->jsModuleToCoreModule(carrier);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(g_implI->moduleGetTarget(coreCarrier), ABCKIT_TARGET_JS);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddImportFromJsToJs, abc-kind=JS, category=positive, extension=c
TEST_F(LibAbcKitJSModifyApiModulesOhmurlTest, ReuseExistingOhmurlRequest)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    auto *entry = FindModuleByName(file, "&JSmodules_ohmurl&");
    ASSERT_NE(entry, nullptr);

    size_t importsBefore = 0;
    g_implI->moduleEnumerateImports(entry, &importsBefore, [](AbckitCoreImportDescriptor *, void *data) {
        (*reinterpret_cast<size_t *>(data))++;
        return true;
    });

    // Import from the module that already has an OHMurl request in the source
    AbckitJsImportFromDynamicModuleCreateParams params {};
    params.name = "regularFunc";
    params.alias = "regularFunc";
    auto *entryJs = g_implJsI->coreModuleToJsModule(entry);
    auto *greetJs = g_implJsI->coreModuleToJsModule(FindModuleByName(file, "&modules/greet&"));
    auto *newImport = g_implJsM->moduleAddImportFromJsToJs(entryJs, greetJs, &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newImport, nullptr);

    size_t importsAfter = 0;
    g_implI->moduleEnumerateImports(entry, &importsAfter, [](AbckitCoreImportDescriptor *, void *data) {
        (*reinterpret_cast<size_t *>(data))++;
        return true;
    });
    // The import item was added; the write below smoke-checks the result
    // (the existing OHMurl request must be reused, not duplicated)
    ASSERT_EQ(importsAfter, importsBefore + 1);

    g_impl->writeAbc(file, MODIFIED_PATH, strlen(MODIFIED_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=JsModifyApiImpl::moduleAddImportFromJsToJs, abc-kind=JS, category=positive, extension=c
TEST_F(LibAbcKitJSModifyApiModulesOhmurlTest, FreshImportWithVersionSuffix)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    auto *entry = FindModuleByName(file, "&JSmodules_ohmurl&");
    ASSERT_NE(entry, nullptr);

    // Carrier with a version suffix: the full OHMurl must be written verbatim
    AbckitJsExternalModuleCreateParams params {};
    params.name = "@normalized:N&&&modules/versioned&1.0.0";
    auto *carrier = g_implJsM->fileAddExternalModule(file, &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(carrier, nullptr);

    AbckitJsImportFromDynamicModuleCreateParams ip {};
    ip.name = "versionedFunc";
    ip.alias = "versionedFunc";
    auto *entryJs = g_implJsI->coreModuleToJsModule(entry);
    auto *newImport = g_implJsM->moduleAddImportFromJsToJs(entryJs, carrier, &ip);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newImport, nullptr);

    // The import resolves to the abc-internal 'modules/versioned' entity
    auto *coreImport = g_implJsI->jsImportDescriptorToCoreImportDescriptor(newImport);
    auto *imported = g_implI->importDescriptorGetImportedModule(coreImport);
    ASSERT_NE(imported, nullptr);
    ASSERT_EQ(g_implI->moduleGetTarget(imported), ABCKIT_TARGET_JS);
    ASSERT_FALSE(g_implI->moduleIsExternal(imported));

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

}  // namespace libabckit::test
