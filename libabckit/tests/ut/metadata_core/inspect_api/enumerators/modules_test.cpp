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

#include <cassert>
#include <cstring>
#include <gtest/gtest.h>

#include "libabckit/include/c/abckit.h"
#include "helpers/helpers.h"
#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/statuses.h"

namespace libabckit::test {

namespace {

auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

class LibAbcKitInspectApiEnumeratorsTest : public ::testing::Test {};

std::unordered_map<std::string, std::vector<std::string>> g_m1Names = {
    {"m0N0N0F0",
     {
         "enumerators0_dynamic",
         "M0N0",
         "M0N0N0",
     }},
    {"M0N0C0",
     {
         "enumerators0_dynamic",
         "M0N0",
         "M0N0C0",
     }},
    {"M0N0C0F0",
     {
         "enumerators0_dynamic",
         "M0N0",
         "M0N0C0",
     }},
    {"M0N0C0F1",
     {
         "enumerators0_dynamic",
         "M0N0",
         "M0N0C0",
     }},
    {"M0N0C0F2",
     {
         "enumerators0_dynamic",
         "M0N0",
         "M0N0C0",
     }},
    {"C", {"enumerators0_dynamic", "M0N0", "A", "B", "C"}},
    {"D", {"enumerators0_dynamic", "M0N0", "A", "B", "C"}},
    {"E", {"enumerators0_dynamic", "M0N0", "A", "B", "C", "D"}},
    {"F", {"enumerators0_dynamic", "M0N0", "A", "B", "C", "D", "E"}},
    {"E^1", {"enumerators0_dynamic", "M0N0", "A", "B", "C", "D"}},
    {"G", {"enumerators0_dynamic", "M0N0", "A", "B", "C", "D", "E^1"}},
    {"I",
     {
         "enumerators0_dynamic",
         "M0N0",
         "A",
         "B",
     }},
    {"m0N0F0",
     {
         "enumerators0_dynamic",
         "M0N0",
     }},
    {"m0N0F1",
     {
         "enumerators0_dynamic",
         "M0N0",
     }},
    {"m0N0F2",
     {
         "enumerators0_dynamic",
         "M0N0",
     }},
    {"M0C0",
     {
         "enumerators0_dynamic",
         "M0C0",
     }},
    {"M0C0F0",
     {
         "enumerators0_dynamic",
         "M0C0",
     }},
    {"M0C0F1",
     {
         "enumerators0_dynamic",
         "M0C0",
     }},
    {"M0C0F2",
     {
         "enumerators0_dynamic",
         "M0C0",
     }},
    {"m0F0",
     {
         "enumerators0_dynamic",
     }},
    {"m0F1",
     {
         "enumerators0_dynamic",
     }},
    {"m0F2",
     {
         "enumerators0_dynamic",
     }},
    {"enumerators0_dynamic.#*@0*#",
     {
         "enumerators0_dynamic",
     }},
    {"enumerators0_dynamic.#*@0*#^1",
     {
         "enumerators0_dynamic",
     }},
    {"enumerators0_dynamic.#&@2&A&B~C>D*#",
     {
         "enumerators0_dynamic",
     }},
    {"enumerators0_dynamic.#&@2&A&B~C>D**#",
     {
         "enumerators0_dynamic",
     }},
    {"H", {"enumerators0_dynamic", "enumerators0_dynamic.#&@2&A&B~C>D**#"}},
    {"func_main_0",
     {
         "enumerators0_dynamic",
     }},
};

std::unordered_map<std::string, std::vector<std::string>> g_m2Names = {
    {"M1N0C0",
     {
         "modules/enumerators1_dynamic",
         "M1N0",
         "M1N0C0",
     }},
    {"M1N0C0F0",
     {
         "modules/enumerators1_dynamic",
         "M1N0",
         "M1N0C0",
     }},
    {"M1N0C0F1",
     {
         "modules/enumerators1_dynamic",
         "M1N0",
         "M1N0C0",
     }},
    {"M1N0C0F2",
     {
         "modules/enumerators1_dynamic",
         "M1N0",
         "M1N0C0",
     }},
    {"m1N0F0",
     {
         "modules/enumerators1_dynamic",
         "M1N0",
     }},
    {"m1N0F1",
     {
         "modules/enumerators1_dynamic",
         "M1N0",
     }},
    {"M1C0",
     {
         "modules/enumerators1_dynamic",
         "M1C0",
     }},
    {"M1C0F0",
     {
         "modules/enumerators1_dynamic",
         "M1C0",
     }},
    {"M1C0F1",
     {
         "modules/enumerators1_dynamic",
         "M1C0",
     }},
    {"M1C0F2",
     {
         "modules/enumerators1_dynamic",
         "M1C0",
     }},
    {"m1F0",
     {
         "modules/enumerators1_dynamic",
     }},
    {"m1F1",
     {
         "modules/enumerators1_dynamic",
     }},
    {"modules/enumerators1_dynamic.#*@0*#",
     {
         "modules/enumerators1_dynamic",
     }},
    {"func_main_0",
     {
         "modules/enumerators1_dynamic",
     }},
};

std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> g_names = {
    {"enumerators0_dynamic", g_m1Names},
    {"modules/enumerators1_dynamic", g_m2Names},
};

}  // namespace

// CC-OFFNXT(huge_method, C_RULE_ID_FUNCTION_SIZE) test helper
static void EnumerateAllMethods()
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enumerators/enumerators0_dynamic.abc", &file);

    size_t funcsCounter = 0;
    std::vector<std::string> curNames {};
    std::string curModuleName {};

    std::function<void(AbckitCoreFunction *)> cbFunc = [&](AbckitCoreFunction *f) {
        auto funcName = helpers::AbckitStringToString(g_implI->functionGetName(f));
        ASSERT_EQ(g_names.count(curModuleName), 1);
        auto &moduleNames = g_names[curModuleName];
        ASSERT_EQ(moduleNames.count(funcName.data()), 1);
        auto &funcNames = moduleNames[funcName.data()];
        ASSERT_EQ(funcNames.size(), curNames.size());
        ASSERT_TRUE(std::equal(funcNames.begin(), funcNames.end(), curNames.begin()));
        funcsCounter++;
        curNames.emplace_back(funcName);
        g_implI->functionEnumerateNestedFunctions(f, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(m);
            return true;
        });
        curNames.pop_back();
    };

    std::function<void(AbckitCoreClass *)> cbClass = [&](AbckitCoreClass *c) {
        auto className = helpers::AbckitStringToString(g_implI->classGetName(c));
        curNames.emplace_back(className);
        g_implI->classEnumerateMethods(c, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(m);
            return true;
        });
        curNames.pop_back();
    };

    std::function<void(AbckitCoreNamespace *)> cbNamespce = [&](AbckitCoreNamespace *n) {
        auto namespaceName = helpers::AbckitStringToString(g_implI->namespaceGetName(n));
        curNames.emplace_back(namespaceName);
        g_implI->namespaceEnumerateNamespaces(n, &cbNamespce, [](AbckitCoreNamespace *n, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreNamespace *)> *>(cb))(n);
            return true;
        });
        g_implI->namespaceEnumerateClasses(n, &cbClass, [](AbckitCoreClass *c, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
            return true;
        });
        g_implI->namespaceEnumerateTopLevelFunctions(n, &cbFunc, [](AbckitCoreFunction *f, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(f);
            return true;
        });
        curNames.pop_back();
    };

    std::function<void(AbckitCoreModule *)> cbModule = [&](AbckitCoreModule *m) {
        curModuleName = helpers::AbckitStringToString(g_implI->moduleGetName(m));
        curNames.emplace_back(curModuleName);
        g_implI->moduleEnumerateNamespaces(m, &cbNamespce, [](AbckitCoreNamespace *n, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreNamespace *)> *>(cb))(n);
            return true;
        });
        g_implI->moduleEnumerateClasses(m, &cbClass, [](AbckitCoreClass *c, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreClass *)> *>(cb))(c);
            return true;
        });
        g_implI->moduleEnumerateTopLevelFunctions(m, &cbFunc, [](AbckitCoreFunction *m, void *cb) {
            (*reinterpret_cast<std::function<void(AbckitCoreFunction *)> *>(cb))(m);
            return true;
        });
        curNames.pop_back();
    };

    g_implI->fileEnumerateModules(file, &cbModule, [](AbckitCoreModule *m, void *cb) {
        (*reinterpret_cast<std::function<void(AbckitCoreModule *)> *>(cb))(m);
        return true;
    });
    ASSERT_EQ(funcsCounter, g_m1Names.size() + g_m2Names.size());

    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

struct TestStructType {
    size_t anonFuncsCounter = 0;
    size_t anonFuncsNumber = 0;
    std::string nameModuleToSearch;
    std::vector<std::string> namesOfAnonFuncs;
};

static void EnumerateAnonymousFunctions(const char *abcFilePath, TestStructType &testStruct)
{
    AbckitFile *file = nullptr;
    helpers::AssertOpenAbc(abcFilePath, &file);

    g_implI->fileEnumerateModules(file, &testStruct, [](AbckitCoreModule *m, void *data) {
        auto testStruct = reinterpret_cast<TestStructType *>(data);

        auto strModule = g_implI->moduleGetName(m);
        EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);
        auto nameModule = helpers::AbckitStringToString(strModule);
        if (nameModule != testStruct->nameModuleToSearch) {
            return true;
        }
        g_implI->moduleEnumerateAnonymousFunctions(m, data, [](AbckitCoreFunction *func, void *data1) {
            auto testStruct = reinterpret_cast<TestStructType *>(data1);

            auto strFunc = g_implI->functionGetName(func);
            EXPECT_TRUE(g_impl->getLastError() == ABCKIT_STATUS_NO_ERROR);

            auto nameFunc = helpers::AbckitStringToString(strFunc);

            auto &funcs = testStruct->namesOfAnonFuncs;

            auto iter = std::find_if(funcs.begin(), funcs.end(),
                                     [&nameFunc](const std::string &name) { return nameFunc == name; });

            EXPECT_TRUE(iter != funcs.end());
            testStruct->anonFuncsCounter++;
            return true;
        });
        return true;
    });

    ASSERT_EQ(testStruct.anonFuncsCounter, testStruct.anonFuncsNumber);
    g_impl->closeFile(file);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=InspectApiImpl::fileEnumerateModules, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicFileEnumerateModules)
{
    EnumerateAllMethods();
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateNamespaces, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicModuleEnumerateNamespaces)
{
    EnumerateAllMethods();
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateClasses, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicModuleEnumerateClasses)
{
    EnumerateAllMethods();
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateTopLevelFunctions, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicModuleEnumerateTopLevelFunctions)
{
    EnumerateAllMethods();
}

// Test: test-kind=api, api=InspectApiImpl::namespaceGetName, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicNamespaceGetName)
{
    EnumerateAllMethods();
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateNamespaces, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicNamespaceEnumerateNamespaces)
{
    EnumerateAllMethods();
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateClasses, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicNamespaceEnumerateClasses)
{
    EnumerateAllMethods();
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateTopLevelFunctions, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicNamespaceEnumerateTopLevelFunctions)
{
    EnumerateAllMethods();
}

// Test: test-kind=api, api=InspectApiImpl::classEnumerateMethods, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicClassEnumerateMethods)
{
    EnumerateAllMethods();
}

// Test: test-kind=api, api=InspectApiImpl::functionEnumerateNestedFunctions, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicFunctionEnumerateNestedFunctions)
{
    EnumerateAllMethods();
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateAnonymousFunctions, abc-kind=ArkTS1, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, DynamicClassEnumerateAnonymousFunctions)
{
    TestStructType testStruct;
    testStruct.anonFuncsNumber = 4U;
    testStruct.nameModuleToSearch = "enumerators0_dynamic";
    testStruct.namesOfAnonFuncs = std::vector<std::string> {
        "enumerators0_dynamic.#&@2&A&B~C>D*#",
        "enumerators0_dynamic.#&@2&A&B~C>D**#",
        "enumerators0_dynamic.#*@0*#",
        "enumerators0_dynamic.#*@0*#^1",
    };

    EnumerateAnonymousFunctions(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enumerators/enumerators0_dynamic.abc",
                                testStruct);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateAnonymousFunctions, abc-kind=ArkTS2, category=positive
TEST_F(LibAbcKitInspectApiEnumeratorsTest, StaticClassEnumerateAnonymousFunctions)
{
    TestStructType testStruct;
    testStruct.anonFuncsNumber = 2U;
    testStruct.nameModuleToSearch = "enumerators_static";
    testStruct.namesOfAnonFuncs = std::vector<std::string> {"lambda$invoke$0:void;", "lambda$invoke$1:void;"};

    EnumerateAnonymousFunctions(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enumerators/enumerators_static.abc",
                                testStruct);
}

}  // namespace libabckit::test
