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

#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/cpp/abckit_cpp.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "tests/helpers/helpers.h"
#include "tests/helpers/helpers_runtime.h"

namespace {
constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modify.abc";
constexpr auto OUTPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modify_modified.abc";
constexpr auto NEW_NAME = "New";
}  // namespace

namespace libabckit::test {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSModifyApiModulesTest : public ::testing::Test {};

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleSetName, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleSetNameStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    auto arkModule = g_implArkI->coreModuleToArktsModule(module);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arkModule, nullptr);
    g_implArkM->moduleSetName(arkModule, NEW_NAME);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arkModule, nullptr);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    helpers::ModuleByNameContext newFinder = {nullptr, NEW_NAME};
    g_implI->fileEnumerateModules(file, &newFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(newFinder.module, nullptr);

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "modules_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "New", "main"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::ModuleAddField, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleAddField)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/module_static_modify2.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "module_static_modify2"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;

    struct AbckitArktsFieldCreateParams params;
    params.name = "newModuleFieldUnique";
    params.type = g_implM->createType(file, AbckitTypeId::ABCKIT_TYPE_ID_STRING);
    std::string moduleFieldValue = "hello";
    params.value = g_implM->createValueString(file, moduleFieldValue.c_str(), moduleFieldValue.length());
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    params.isStatic = true;
    params.fieldVisibility = AbckitArktsFieldVisibility::PUBLIC;

    auto ret = g_implArkM->moduleAddField(g_implArkI->coreModuleToArktsModule(module), &params);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_TRUE(ret);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::AssertOpenAbc(OUTPUT_PATH, &file);
    ctxFinder = {nullptr, "module_static_modify2"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_NE(ctxFinder.module, nullptr);
    module = ctxFinder.module;

    std::vector<std::string> FieldNames;
    g_implI->moduleEnumerateFields(module, &FieldNames, [](AbckitCoreModuleField *field, void *data) {
        auto ctx = static_cast<std::vector<std::string> *>(data);
        auto filedName = g_implI->abckitStringToString(g_implI->moduleFieldGetName(field));
        (*ctx).emplace_back(filedName);
        return true;
    });
    std::vector<std::string> actualFieldNames = {"newModuleFieldUnique"};
    ASSERT_EQ(FieldNames, actualFieldNames);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_EQ(helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR
                                        "ut/extensions/arkts/modify_api/modules/module_static_modify2.abc",
                                        "module_static_modify2", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "module_static_modify2", "main"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::fileAddExternalModuleArktsV2, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, FileAddExternalModuleArktsV2)
{
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modify.abc",
                                  "modules_static_modify", "main");
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modify.abc", &file);

    const char *externalModuleName = "std.math";
    g_implArkM->fileAddExternalModuleArktsV2(file, externalModuleName);
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc",
                     strlen(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc"));
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc", &file);
    helpers::ModuleByNameContext ctxFinder = {nullptr, externalModuleName};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto name = g_implI->moduleGetName(ctxFinder.module);
    ASSERT_TRUE(helpers::Match(externalModuleName, g_implI->abckitStringToString(name)));
    g_impl->closeFile(file);

    auto modifiedOutput =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc",
                                  "modules_static_modify", "main");
    ASSERT_TRUE(helpers::Match(modifiedOutput, output));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleImportClassFromArktsV2ToArktsV2, abc-kind=ArkTS2,
// category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleImportClassFromArktsV2ToArktsV2)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modify.abc", &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "std.core"};
    g_implI->fileEnumerateExternalModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    auto arkModule = g_implArkI->coreModuleToArktsModule(module);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arkModule, nullptr);

    auto arkClass = g_implArkM->moduleImportClassFromArktsV2toArktsV2(arkModule, "Boolean");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arkClass, nullptr);

    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc",
                     strlen(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc"));
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc", &file);
    g_implI->fileEnumerateExternalModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    module = ctxFinder.module;

    helpers::ClassByNameContext classFinder = {nullptr, "Boolean"};
    g_implI->moduleEnumerateClasses(module, &classFinder, helpers::ClassByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(classFinder.klass, nullptr);

    g_impl->closeFile(file);
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleImportStaticFunction, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleImportStaticFunction)
{
    // 1. add externalModule and write back
    AbckitFile *file = nullptr;
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modify.abc",
                                  "modules_static_modify", "main");
    ASSERT_TRUE(helpers::Match(output, "25\n"));
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modify.abc", &file);
    auto arkModule = g_implArkM->fileAddExternalModuleArktsV2(file, "std.math");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arkModule, nullptr);
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc",
                     strlen(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc"));
    g_impl->closeFile(file);

    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc", &file);
    helpers::ModuleByNameContext ctxFinder = {nullptr, "std.math"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);
    auto module = ctxFinder.module;
    arkModule = g_implArkI->coreModuleToArktsModule(module);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arkModule, nullptr);

    // 2. externalModule import static function
    const char *params[] = {"f64"};
    auto arkFunc = g_implArkM->moduleImportStaticFunctionFromArktsV2ToArktsV2(arkModule, "abs", "f64", params, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arkFunc, nullptr);
    auto *coreFunc = g_implArkI->arktsFunctionToCoreFunction(arkFunc);

    // 3. modify certain function's graph to call imported external function
    auto *method = helpers::FindMethodByName(file, "main");
    AbckitGraph *graph = g_implI->createGraphFromFunction(method);
    auto *callInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
    auto *value = g_implG->gFindOrCreateConstantF64(graph, -302);
    auto *call = g_statG->iCreateCallStatic(graph, coreFunc, 1, value);
    g_implG->iInsertBefore(call, callInst);
    g_implG->iSetInput(callInst, call, 1);
    g_implM->functionSetGraph(method, graph);
    g_impl->destroyGraph(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // 4. write back and execute abc
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc",
                     strlen(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc",
                                  "modules_static_modify", "main");
    ASSERT_TRUE(helpers::Match(output, "302\n"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::moduleImportClassMethod, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleImportClassMethod)
{
    // 1. add externalModule and write back
    AbckitFile *file = nullptr;
    auto output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modify.abc",
                                  "modules_static_modify", "main");
    ASSERT_TRUE(helpers::Match(output, "25\n"));
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modify.abc", &file);
    auto coreModule = g_implArkM->fileAddExternalModuleArktsV2(file, "std.core");
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(coreModule, nullptr);
    // 2. import class method
    const char *params[] = {"std.core.String"};
    auto logFunc =
        g_implArkM->moduleImportClassMethodFromArktsV2ToArktsV2(coreModule, "Console", "log", "void", params, 1);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(logFunc, nullptr);
    auto *coreFunc = g_implArkI->arktsFunctionToCoreFunction(logFunc);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(coreFunc, nullptr);

    auto *method = helpers::FindMethodByName(file, "main");
    AbckitGraph *graph = g_implI->createGraphFromFunction(method);
    auto *returnInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
    auto *loadStaticInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_LOADSTATIC);
    auto str = g_implM->createString(file, "hellostring", strlen("hellostring"));
    AbckitInst *loadString = g_statG->iCreateLoadString(graph, str);
    g_implG->iInsertBefore(loadString, returnInst);
    auto *call = g_statG->iCreateCallStatic(graph, coreFunc, 2, loadStaticInst, loadString);
    g_implG->iInsertBefore(call, returnInst);
    g_implM->functionSetGraph(method, graph);
    g_impl->destroyGraph(graph);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    // 4. write back and execute abc
    g_impl->writeAbc(file, ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc",
                     strlen(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc"));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_impl->closeFile(file);
    output =
        helpers::ExecuteStaticAbc(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/modules/modules_static_modified.abc",
                                  "modules_static_modify", "main");
    ASSERT_TRUE(helpers::Match(output, "25\nhellostring\n"));
}

}  // namespace libabckit::test
