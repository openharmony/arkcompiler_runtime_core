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
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkM = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitArkTSModifyApiModulesTest : public ::testing::Test {
protected:
    std::unique_ptr<AbckitCoreFunctionParam> CreateTestParameter(AbckitFile *file, const char *name,
                                                                 AbckitTypeId typeId)
    {
        auto *paramNameStr = g_implM->createString(file, name, strlen(name));
        EXPECT_NE(paramNameStr, nullptr);

        auto *paramType = g_implM->createType(file, typeId);
        EXPECT_NE(paramType, nullptr);

        auto param = std::make_unique<AbckitCoreFunctionParam>();
        param->name = paramNameStr;
        param->type = paramType;
        param->function = nullptr;
        param->defaultValue = nullptr;
        auto arktsParam = std::make_unique<AbckitArktsFunctionParam>();
        arktsParam->core = param.get();
        param->impl.emplace<std::unique_ptr<AbckitArktsFunctionParam>>(std::move(arktsParam));

        return param;
    }

    AbckitArktsFunction *CreateTestFunction(AbckitArktsModule *module, AbckitFile *file, const char *funcName,
                                            AbckitTypeId returnTypeId,
                                            const std::vector<std::unique_ptr<AbckitCoreFunctionParam>> &params)
    {
        auto *returnType = g_implM->createType(file, returnTypeId);
        EXPECT_NE(returnType, nullptr);

        // parameter array ends with nullptr
        std::vector<AbckitArktsFunctionParam *> paramPtrs;
        for (const auto &param : params) {
            paramPtrs.push_back(param->GetArkTSImpl());
        }
        paramPtrs.push_back(nullptr);

        AbckitArktsFunctionCreateParams createParams;
        createParams.name = funcName;
        createParams.params = paramPtrs.data();
        createParams.returnType = returnType;
        createParams.isAsync = false;

        auto *newFunction = g_implArkM->moduleAddFunction(module, &createParams);
        EXPECT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        EXPECT_NE(newFunction, nullptr);

        return newFunction;
    }

    // create a simple function body use iCreateInst API
    void AddSimpleFunctionBody(AbckitArktsFunction *function, int32_t addValue)
    {
        auto *graph = g_implI->createGraphFromFunction(function->core);
        ASSERT_NE(graph, nullptr);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

        auto *param0 = g_implG->gGetParameter(graph, 0);
        ASSERT_NE(param0, nullptr);

        auto *constValue = g_implG->gFindOrCreateConstantI32(graph, addValue);
        ASSERT_NE(constValue, nullptr);

        auto *addInst = g_statG->iCreateAdd(graph, param0, constValue);
        ASSERT_NE(addInst, nullptr);

        auto *newReturnInst = g_statG->iCreateReturn(graph, addInst);
        ASSERT_NE(newReturnInst, nullptr);

        auto *originalReturnInst = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
        g_implG->iInsertBefore(addInst, originalReturnInst);
        g_implG->iInsertBefore(newReturnInst, originalReturnInst);
        g_implG->iRemove(originalReturnInst);

        g_implM->functionSetGraph(function->core, graph);
        g_impl->destroyGraph(graph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    }

    // modify main function call the new created function
    void ModifyMainFunction(AbckitFile *file, AbckitArktsFunction *testFunction, int32_t argValue)
    {
        auto *mainFunc = helpers::FindMethodByName(file, "main");
        auto *mainGraph = g_implI->createGraphFromFunction(mainFunc);
        ASSERT_NE(mainGraph, nullptr);

        auto *consoleLog = helpers::FindFirstInst(mainGraph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
        ASSERT_NE(consoleLog, nullptr);

        auto *constArg = g_implG->gFindOrCreateConstantI32(mainGraph, argValue);
        auto *callInst = g_statG->iCreateCallStatic(mainGraph, testFunction->core, 1, constArg);
        ASSERT_NE(callInst, nullptr);

        g_implG->iInsertBefore(callInst, consoleLog);
        g_implG->iSetInput(consoleLog, callInst, 1);

        g_implM->functionSetGraph(mainFunc, mainGraph);
        g_impl->destroyGraph(mainGraph);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    }
};

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

TEST_F(LibAbcKitArkTSModifyApiModulesTest, ModuleAddFunctionStatic)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(INPUT_PATH, &file);

    helpers::ModuleByNameContext ctxFinder = {nullptr, "modules_static_modify"};
    g_implI->fileEnumerateModules(file, &ctxFinder, helpers::ModuleByNameFinder);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(ctxFinder.module, nullptr);

    auto *arkModule = g_implArkI->coreModuleToArktsModule(ctxFinder.module);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(arkModule, nullptr);

    std::vector<std::unique_ptr<AbckitCoreFunctionParam>> params;
    params.push_back(CreateTestParameter(file, "x", ABCKIT_TYPE_ID_I32));

    auto *testFunction = CreateTestFunction(arkModule, file, "testFunction", ABCKIT_TYPE_ID_I32, params);
    ASSERT_NE(testFunction, nullptr);

    for (auto &param : params) {
        param.release();
    }

    AddSimpleFunctionBody(testFunction, 10);

    ModifyMainFunction(file, testFunction, 5);

    g_impl->writeAbc(file, OUTPUT_PATH, strlen(OUTPUT_PATH));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    ASSERT_EQ(helpers::ExecuteStaticAbc(OUTPUT_PATH, "modules_static_modify", "main"), "15\n");
}

}  // namespace libabckit::test
