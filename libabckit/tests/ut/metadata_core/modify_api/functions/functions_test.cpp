/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <cstring>
#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"
#include "libabckit/c/abckit.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/metadata_core.h"
#include "metadata_inspect_impl.h"
#include "libabckit/src/helpers_common.h"
namespace libabckit::test {
namespace {

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implM = AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_arktsModifyApiImpl = AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_statG = AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implG = AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
}  // namespace

class LibAbcKitModifyApiFunctionsTest : public ::testing::Test {};

static constexpr auto INITIAL_STATIC = ABCKIT_ABC_DIR "ut/metadata_core/modify_api/functions/functions_static.abc";
static constexpr auto MODIFIED_STATIC =
    ABCKIT_ABC_DIR "ut/metadata_core/modify_api/functions/functions_static_modified.abc";

// Test: test-kind=api, api=FunctionSetNameStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiFunctionsTest, StaticFunctionSetName)
{
    auto output = helpers::ExecuteStaticAbc(INITIAL_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(output, "123\nclass prop\n1\n123\nfoo\n"));
    const char *newName = "RenamedFunc";
    helpers::TransformMethod(
        INITIAL_STATIC, MODIFIED_STATIC, "InitFuncName",
        [&](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            AbckitArktsFunction *arktsFunc = method->GetArkTSImpl();
            bool res = g_arktsModifyApiImpl->functionSetName(arktsFunc, newName);
            ASSERT_TRUE(res);
        },
        [](AbckitGraph *) {});

    AbckitFile *modifiedFile = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(modifiedFile, nullptr);

    auto *modifiedMethod = helpers::FindMethodByName(modifiedFile, "RenamedFunc");
    ASSERT_NE(modifiedMethod, nullptr);

    AbckitString *modifiedStr = g_implI->functionGetName(modifiedMethod);
    ASSERT_NE(modifiedStr, nullptr);
    auto modifiedFuncName = helpers::AbckitStringToString(modifiedStr);
    ASSERT_TRUE(modifiedFuncName.find(newName) != std::string::npos);

    g_impl->closeFile(modifiedFile);

    auto modifiedOutput = helpers::ExecuteStaticAbc(MODIFIED_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(modifiedOutput, output));
}

// Test: test-kind=api, api=FunctionSetNameStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiFunctionsTest, InstanceFunctionSetName)
{
    auto output = helpers::ExecuteStaticAbc(INITIAL_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(output, "123\nclass prop\n1\n123\nfoo\n"));
    const char *newName = "RenamedFunc";
    helpers::TransformMethod(
        INITIAL_STATIC, MODIFIED_STATIC, "foo",
        [&](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            AbckitArktsFunction *arktsFunc = method->GetArkTSImpl();
            bool res = g_arktsModifyApiImpl->functionSetName(arktsFunc, newName);
            ASSERT_TRUE(res);
        },
        [](AbckitGraph *) {});
    AbckitFile *modifiedFile = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(modifiedFile, nullptr);

    auto *modifiedMethod = helpers::FindMethodByName(modifiedFile, "RenamedFunc");
    ASSERT_NE(modifiedMethod, nullptr);

    AbckitString *modifiedStr = g_implI->functionGetName(modifiedMethod);
    ASSERT_NE(modifiedStr, nullptr);
    auto modifiedFuncName = helpers::AbckitStringToString(modifiedStr);
    ASSERT_TRUE(modifiedFuncName.find(newName) != std::string::npos);

    g_impl->closeFile(modifiedFile);

    auto modifiedOutput = helpers::ExecuteStaticAbc(MODIFIED_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(modifiedOutput, output));
}

static void TransformMethodReturnTypeToString(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    auto *originalRet = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    auto *res = g_implM->createValueString(file, "test", strlen("test"));
    auto *type = g_implI->valueGetType(res);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    ASSERT_NE(type, nullptr);

    auto *arktsFunc = method->GetArkTSImpl();
    bool success = g_arktsModifyApiImpl->functionSetReturnType(arktsFunc, type);
    ASSERT_TRUE(success) << "Failed to set return type to function";

    auto *str = g_implM->createString(file, "hello", strlen("hello"));
    ASSERT_NE(str, nullptr);
    auto *loadStr = g_statG->iCreateLoadString(graph, str);
    ASSERT_NE(loadStr, nullptr);
    g_implG->iInsertBefore(loadStr, originalRet);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *newReturnInst = g_statG->iCreateReturn(graph, loadStr);
    ASSERT_NE(newReturnInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::ReplaceInst(originalRet, newReturnInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static AbckitInst *SearchStaticCallInBasicBlock(AbckitBasicBlock *bb, AbckitCoreFunction *targetFunc)
{
    for (auto *inst = g_implG->bbGetFirstInst(bb); inst != nullptr; inst = g_implG->iGetNext(inst)) {
        auto opcode = g_statG->iGetOpcode(inst);
        if (opcode == ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC) {
            auto *func = g_implG->iGetFunction(inst);
            if (func == targetFunc) {
                return inst;
            }
        }
    }
    return nullptr;
}

static AbckitInst *FindStaticCall(AbckitGraph *graph, AbckitCoreFunction *targetFunc)
{
    std::vector<AbckitBasicBlock *> bbs;
    g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
        reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
        return true;
    });

    for (auto *bb : bbs) {
        AbckitInst *result = SearchStaticCallInBasicBlock(bb, targetFunc);
        if (result != nullptr) {
            return result;
        }
    }
    return nullptr;
}

static void CollectUsersFromBasicBlock(AbckitBasicBlock *bb, AbckitInst *targetInst,
                                       std::vector<std::pair<AbckitInst *, int>> &users)
{
    for (auto *inst = g_implG->bbGetFirstInst(bb); inst != nullptr; inst = g_implG->iGetNext(inst)) {
        int inputCount = g_implG->iGetInputCount(inst);
        for (int i = 0; i < inputCount; i++) {
            if (g_implG->iGetInput(inst, i) == targetInst) {
                users.emplace_back(inst, i);
            }
        }
    }
}

static std::vector<std::pair<AbckitInst *, int>> CollectInstructionUsers(AbckitGraph *graph, AbckitInst *targetInst)
{
    std::vector<std::pair<AbckitInst *, int>> users;
    std::vector<AbckitBasicBlock *> bbs;
    g_implG->gVisitBlocksRpo(graph, &bbs, [](AbckitBasicBlock *bb, void *data) {
        reinterpret_cast<std::vector<AbckitBasicBlock *> *>(data)->emplace_back(bb);
        return true;
    });

    for (auto *bb : bbs) {
        CollectUsersFromBasicBlock(bb, targetInst, users);
    }
    return users;
}

static void ReplaceFunctionCall(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph,
                                const char *oldFuncName, const char *newFuncName)
{
    auto *oldFunc = helpers::FindMethodByName(file, oldFuncName);
    ASSERT_NE(oldFunc, nullptr);

    AbckitInst *oldCall = FindStaticCall(graph, oldFunc);
    ASSERT_NE(oldCall, nullptr);

    auto *newFunc = helpers::FindMethodByName(file, newFuncName);
    ASSERT_NE(newFunc, nullptr);
    AbckitInst *loadI32 = g_implG->gFindOrCreateConstantI32(graph, 2);
    auto *newCall = g_statG->iCreateCallStatic(graph, newFunc, 2, loadI32, loadI32);
    ASSERT_NE(newCall, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    g_implG->iInsertBefore(newCall, oldCall);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto users = CollectInstructionUsers(graph, oldCall);
    for (auto &[userInst, inputIndex] : users) {
        g_implG->iSetInput(userInst, newCall, inputIndex);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    }

    g_implG->iRemove(oldCall);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

static void TransformMethodReturnTypeToI32(AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph)
{
    auto *typeName = g_implM->createString(file, "i32", strlen("i32"));
    ASSERT_NE(typeName, nullptr);
    auto *type = GetOrCreateType(file, ABCKIT_TYPE_ID_I32, 0, nullptr, typeName);
    auto *arktsFunc = method->GetArkTSImpl();
    bool success = g_arktsModifyApiImpl->functionSetReturnType(arktsFunc, type);
    ASSERT_TRUE(success) << "Failed to set return type to function";

    AbckitInst *loadI32 = g_implG->gFindOrCreateConstantI32(graph, 5);
    ASSERT_NE(loadI32, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    auto *originalRet = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
    ASSERT_NE(originalRet, nullptr);
    auto *newReturnInst = g_statG->iCreateReturn(graph, loadI32);
    ASSERT_NE(newReturnInst, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::ReplaceInst(originalRet, newReturnInst);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
}

// Test: test-kind=api, api=FunctionSetReturnTypeStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiFunctionsTest, StaticFunctionSetReturnType1)
{
    auto output = helpers::ExecuteStaticAbc(INITIAL_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(output, "123\nclass prop\n1\n123\nfoo\n"));

    helpers::TransformMethod(INITIAL_STATIC, MODIFIED_STATIC, "add2", TransformMethodReturnTypeToString,
                             [](AbckitGraph *) {});
    helpers::TransformMethod(
        MODIFIED_STATIC, MODIFIED_STATIC, "main",
        [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            ReplaceFunctionCall(file, method, graph, "add3", "add2");
        },
        [](AbckitGraph *) {});

    auto modifiedOutput = helpers::ExecuteStaticAbc(MODIFIED_STATIC, "functions_static", "main");
    auto *file = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_NE(file, nullptr);
    auto *retType = g_implI->functionGetReturnType(helpers::FindMethodByName(file, "add2"));
    ASSERT_NE(retType, nullptr);
    auto typeId = g_implI->typeGetTypeId(retType);
    ASSERT_EQ(typeId, ABCKIT_TYPE_ID_REFERENCE);
    g_impl->closeFile(file);
    EXPECT_TRUE(helpers::Match(modifiedOutput, "123\nclass prop\n1\nhello\nfoo\n"));
}
// Test: test-kind=api, api=FunctionSetReturnTypeStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiFunctionsTest, StaticFunctionSetReturnType2)
{
    helpers::TransformMethod(INITIAL_STATIC, MODIFIED_STATIC, "add2", TransformMethodReturnTypeToI32,
                             [](AbckitGraph *) {});
    helpers::TransformMethod(
        MODIFIED_STATIC, MODIFIED_STATIC, "main",
        [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            ReplaceFunctionCall(file, method, graph, "add1", "add2");
        },
        [](AbckitGraph *) {});
    auto *file = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_NE(file, nullptr);
    auto *retType = g_implI->functionGetReturnType(helpers::FindMethodByName(file, "add2"));
    ASSERT_NE(retType, nullptr);
    auto typeId = g_implI->typeGetTypeId(retType);
    ASSERT_EQ(typeId, ABCKIT_TYPE_ID_I32);
    g_impl->closeFile(file);
    auto modifiedOutput = helpers::ExecuteStaticAbc(MODIFIED_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(modifiedOutput, "123\nclass prop\n5\n123\nfoo\n"));
}

// Test: test-kind=api, api=FunctionSetReturnTypeStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiFunctionsTest, StaticFunctionSetReturnType3)
{
    helpers::TransformMethod(
        INITIAL_STATIC, MODIFIED_STATIC, "add1",
        [&](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            // number to reference
            auto *originalRet = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);

            auto *meth = helpers::FindMethodByName(file, "foo");
            ASSERT_NE(meth, nullptr);
            auto *cls = g_implI->functionGetParentClass(meth);
            ASSERT_NE(cls, nullptr);
            auto *typeName = g_implM->createString(file, "TestClass", strlen("TestClass"));
            ASSERT_NE(typeName, nullptr);
            auto *type = GetOrCreateType(file, ABCKIT_TYPE_ID_REFERENCE, 0, cls, typeName);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(type, nullptr);

            auto *arktsFunc = method->GetArkTSImpl();
            bool success = g_arktsModifyApiImpl->functionSetReturnType(arktsFunc, type);
            ASSERT_TRUE(success) << "Failed to set return type to function";

            auto *obj = g_statG->iCreateNewObject(graph, cls);
            ASSERT_NE(obj, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            g_implG->iInsertBefore(obj, originalRet);

            auto *newReturnInst = g_statG->iCreateReturn(graph, obj);
            ASSERT_NE(newReturnInst, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            helpers::ReplaceInst(originalRet, newReturnInst);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [](AbckitGraph *) {});
    auto *file = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_NE(file, nullptr);
    auto *retType = g_implI->functionGetReturnType(helpers::FindMethodByName(file, "add1"));
    ASSERT_NE(retType, nullptr);
    auto typeId = g_implI->typeGetTypeId(retType);
    ASSERT_EQ(typeId, ABCKIT_TYPE_ID_REFERENCE);
    g_impl->closeFile(file);
}

// Test: test-kind=api, api=FunctionSetReturnTypeStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiFunctionsTest, StaticFunctionSetReturnType4)
{
    helpers::TransformMethod(
        INITIAL_STATIC, MODIFIED_STATIC, "add1",
        [&](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            // f64 to void
            auto *typeName = g_implM->createString(file, "void", strlen("void"));
            ASSERT_NE(typeName, nullptr);
            auto *type = GetOrCreateType(file, ABCKIT_TYPE_ID_VOID, 0, nullptr, typeName);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(type, nullptr);

            auto *arktsFunc = method->GetArkTSImpl();
            bool success = g_arktsModifyApiImpl->functionSetReturnType(arktsFunc, type);
            ASSERT_TRUE(success) << "Failed to set return type to function";

            AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN);
            ASSERT_NE(ret, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            helpers::ReplaceInst(ret, g_statG->iCreateReturnVoid(graph));
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [](AbckitGraph *) {});
    auto *file = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_NE(file, nullptr);
    auto *retType = g_implI->functionGetReturnType(helpers::FindMethodByName(file, "add1"));
    ASSERT_NE(retType, nullptr);
    auto typeId = g_implI->typeGetTypeId(retType);
    ASSERT_EQ(typeId, ABCKIT_TYPE_ID_VOID);
    g_impl->closeFile(file);
}
// Test: test-kind=api, api=FunctionSetReturnTypeStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiFunctionsTest, InstanceFunctionSetReturnType)
{
    helpers::TransformMethod(
        INITIAL_STATIC, MODIFIED_STATIC, "foo",
        [&](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            // void to int
            auto *typeName = g_implM->createString(file, "i32", strlen("i32"));
            ASSERT_NE(typeName, nullptr);
            auto *type = GetOrCreateType(file, ABCKIT_TYPE_ID_I32, 0, nullptr, typeName);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(type, nullptr);

            auto *arktsFunc = method->GetArkTSImpl();
            bool success = g_arktsModifyApiImpl->functionSetReturnType(arktsFunc, type);
            ASSERT_TRUE(success) << "Failed to set return type to function";
            AbckitInst *ret = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID);
            ASSERT_NE(ret, nullptr);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            helpers::ReplaceInst(ret, g_statG->iCreateReturn(graph, g_implG->gFindOrCreateConstantI32(graph, 1)));
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        },
        [](AbckitGraph *) {});
    auto modifiedOutput = helpers::ExecuteStaticAbc(MODIFIED_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(modifiedOutput, "123\nclass prop\n1\n123\nfoo\n"));
    auto *file = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_NE(file, nullptr);
    auto *retType = g_implI->functionGetReturnType(helpers::FindMethodByName(file, "foo"));
    ASSERT_NE(retType, nullptr);
    auto typeId = g_implI->typeGetTypeId(retType);
    ASSERT_EQ(typeId, ABCKIT_TYPE_ID_I32);
    g_impl->closeFile(file);
}

void ValidateFunctionParameters(AbckitFile *file, const char *methodName,
                                const std::set<std::string> &expectedParamTypes)
{
    auto *method = helpers::FindMethodByName(file, methodName);
    ASSERT_NE(method, nullptr);

    std::set<std::string> gotParamTypeNames;

    bool success = g_implI->functionEnumerateParameters(
        method, &gotParamTypeNames, [](AbckitCoreFunctionParam *param, void *data) -> bool {
            auto *typeNames = static_cast<std::set<std::string> *>(data);

            auto *paramType = g_implI->functionParamGetType(param);

            auto *typeName = g_implI->typeGetName(paramType);
            if (typeName) {
                typeNames->emplace(helpers::AbckitStringToString(typeName));
            }

            return true;
        });

    ASSERT_TRUE(success);
    ASSERT_EQ(gotParamTypeNames, expectedParamTypes);
}
// Test: test-kind=api, api=FunctionAddParameter, abc-kind=ArkTS2, category=positive, extension=c
// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
TEST_F(LibAbcKitModifyApiFunctionsTest, InstanceFunctionAddParameter)
{
    auto output = helpers::ExecuteStaticAbc(INITIAL_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(output, "123\nclass prop\n1\n123\nfoo\n"));

    AbckitFile *file = g_impl->openAbc(INITIAL_STATIC, strlen(INITIAL_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "main", [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
        auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL);
        ASSERT_NE(call, nullptr);
        auto *inputObj = g_implG->iGetInput(call, 0);
        ASSERT_NE(inputObj, nullptr);
        auto *inputStr = g_implG->iGetInput(call, 1);
        ASSERT_NE(inputStr, nullptr);
        AbckitInst *newInput = g_implG->gFindOrCreateConstantI32(graph, 15);
        ASSERT_NE(newInput, nullptr);
        auto *mFoo = helpers::FindMethodByName(file, "foo");
        auto *newCall = g_statG->iCreateCallVirtual(graph, inputObj, mFoo, 2, inputStr, newInput);
        helpers::ReplaceInst(call, newCall);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    g_impl->closeFile(file);

    file = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "foo", [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
        auto *type = g_implM->createType(file, ABCKIT_TYPE_ID_I32);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_NE(type, nullptr);

        auto param = std::make_unique<AbckitCoreFunctionParam>();
        param->function = method;

        auto paramNameStr = std::make_unique<AbckitString>();
        paramNameStr->impl = "newIntParam";
        param->name = paramNameStr.get();
        param->type = type;

        auto arktsParam = std::make_unique<AbckitArktsFunctionParam>();
        arktsParam->core = param.get();
        param->impl = std::move(arktsParam);

        auto *arktsFunc = method->GetArkTSImpl();
        bool success = g_arktsModifyApiImpl->functionAddParameter(arktsFunc, param->GetArkTSImpl());
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        ASSERT_TRUE(success) << "Failed to add int parameter to function";
    });
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    g_impl->closeFile(file);

    AbckitFile *modifiedFile = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_NE(modifiedFile, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<std::string> expectedParamTypes = {"functions_static.A", "i32", "std.core.String"};
    ValidateFunctionParameters(modifiedFile, "foo", expectedParamTypes);
    g_impl->closeFile(modifiedFile);
    auto modifiedOutput = helpers::ExecuteStaticAbc(MODIFIED_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(modifiedOutput, output));
}

// Test: test-kind=api, api=FunctionAddParameter, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitModifyApiFunctionsTest, StaticFunctionAddParamString)
{
    auto output = helpers::ExecuteStaticAbc(INITIAL_STATIC, "functions_static", "main");

    AbckitFile *file = g_impl->openAbc(INITIAL_STATIC, strlen(INITIAL_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "main", [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
        auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
        ASSERT_NE(call, nullptr);
        auto *inputStr = g_implG->iGetInput(call, 0);
        ASSERT_NE(inputStr, nullptr);
        auto *inputObj = g_implG->iGetInput(call, 1);
        ASSERT_NE(inputObj, nullptr);
        auto *mInit = helpers::FindMethodByName(file, "InitFuncName");
        auto *newCall = g_statG->iCreateCallStatic(graph, mInit, 3, inputStr, inputObj, inputStr);
        helpers::ReplaceInst(call, newCall);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    g_impl->closeFile(file);

    file = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(
        file, "InitFuncName", [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            AbckitArktsFunction *arktsFunc = method->GetArkTSImpl();

            auto param = std::make_unique<AbckitCoreFunctionParam>();
            param->function = method;

            auto paramNameStr = std::make_unique<AbckitString>();
            paramNameStr->impl = "newParam";
            param->name = paramNameStr.get();

            auto paramTypeObj = std::make_unique<AbckitType>();
            paramTypeObj->id = ABCKIT_TYPE_ID_STRING;
            paramTypeObj->rank = 0;
            paramTypeObj->klass = nullptr;
            param->type = paramTypeObj.get();

            auto arktsParam = std::make_unique<AbckitArktsFunctionParam>();
            arktsParam->core = param.get();
            param->impl = std::move(arktsParam);

            bool success = g_arktsModifyApiImpl->functionAddParameter(arktsFunc, param->GetArkTSImpl());
            ASSERT_TRUE(success) << "Failed to add parameter to function";
        });
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    g_impl->closeFile(file);

    AbckitFile *modifiedFile = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_NE(modifiedFile, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<std::string> expectedParamTypes = {"functions_static.MyClass", "std.core.String"};
    ValidateFunctionParameters(modifiedFile, "InitFuncName", expectedParamTypes);
    g_impl->closeFile(modifiedFile);
    auto modifiedOutput = helpers::ExecuteStaticAbc(MODIFIED_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(modifiedOutput, output));
}

// Test: test-kind=api, api=FunctionAddParameter, abc-kind=ArkTS2, category=positive, extension=c
// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
TEST_F(LibAbcKitModifyApiFunctionsTest, StaticFunctionAddParamReferenceType)
{
    auto output = helpers::ExecuteStaticAbc(INITIAL_STATIC, "functions_static", "main");

    AbckitFile *file = g_impl->openAbc(INITIAL_STATIC, strlen(INITIAL_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "main", [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
        auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
        ASSERT_NE(call, nullptr);
        auto *input0 = g_implG->iGetInput(call, 0);
        ASSERT_NE(input0, nullptr);
        auto *input1 = g_implG->iGetInput(call, 1);
        ASSERT_NE(input1, nullptr);
        auto *mInit = helpers::FindMethodByName(file, "InitFuncName");
        ASSERT_NE(mInit, nullptr);
        auto *initObj = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT);
        ASSERT_NE(initObj, nullptr);
        auto *newCall = g_statG->iCreateCallStatic(graph, mInit, 3, input0, input1, initObj);
        ASSERT_NE(newCall, nullptr);
        helpers::ReplaceInst(call, newCall);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    g_impl->closeFile(file);

    file = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(
        file, "InitFuncName", [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            auto *meth = helpers::FindMethodByName(file, "foo");
            ASSERT_NE(meth, nullptr);

            auto *cls = g_implI->functionGetParentClass(meth);
            ASSERT_NE(cls, nullptr);

            auto *type = g_implM->createReferenceType(file, cls);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(type, nullptr);
            ASSERT_EQ(g_implI->typeGetTypeId(type), ABCKIT_TYPE_ID_REFERENCE);
            ASSERT_EQ(g_implI->typeGetReferenceClass(type), cls);

            auto param = std::make_unique<AbckitCoreFunctionParam>();
            param->function = method;

            auto paramNameStr = std::make_unique<AbckitString>();
            paramNameStr->impl = "newRefParam";
            param->name = paramNameStr.get();

            param->type = type;

            auto arktsParam = std::make_unique<AbckitArktsFunctionParam>();
            arktsParam->core = param.get();
            param->impl = std::move(arktsParam);

            auto *arktsFunc = method->GetArkTSImpl();
            bool success = g_arktsModifyApiImpl->functionAddParameter(arktsFunc, param->GetArkTSImpl());
            ASSERT_TRUE(success) << "Failed to add reference type parameter to function";
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
        });
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    g_impl->closeFile(file);

    AbckitFile *modifiedFile = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_NE(modifiedFile, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    std::set<std::string> expectedParamTypes = {"functions_static.A", "functions_static.MyClass", "std.core.String"};
    ValidateFunctionParameters(modifiedFile, "InitFuncName", expectedParamTypes);
    g_impl->closeFile(modifiedFile);
    auto modifiedOutput = helpers::ExecuteStaticAbc(MODIFIED_STATIC, "functions_static", "main");
    EXPECT_TRUE(helpers::Match(modifiedOutput, output));
}
// Test: test-kind=api, api=FunctionAddParameter, abc-kind=ArkTS2, category=positive, extension=c
// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
TEST_F(LibAbcKitModifyApiFunctionsTest, StaticFunctionAddIntParam)
{
    auto output = helpers::ExecuteStaticAbc(INITIAL_STATIC, "functions_static", "main");

    AbckitFile *file = g_impl->openAbc(INITIAL_STATIC, strlen(INITIAL_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(file, "main", [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
        auto *call = helpers::FindFirstInst(graph, ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC);
        ASSERT_NE(call, nullptr);
        auto *input0 = g_implG->iGetInput(call, 0);
        ASSERT_NE(input0, nullptr);
        auto *input1 = g_implG->iGetInput(call, 1);
        ASSERT_NE(input1, nullptr);
        auto *mInit = helpers::FindMethodByName(file, "InitFuncName");
        ASSERT_NE(mInit, nullptr);
        auto *newInput = g_implG->gFindOrCreateConstantI32(graph, 12);
        ASSERT_NE(newInput, nullptr);
        auto *newCall = g_statG->iCreateCallStatic(graph, mInit, 3, input0, input1, newInput);
        ASSERT_NE(newCall, nullptr);
        helpers::ReplaceInst(call, newCall);
        ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    });
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    g_impl->closeFile(file);

    file = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    helpers::TransformMethod(
        file, "InitFuncName", [](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            auto *type = g_implM->createType(file, ABCKIT_TYPE_ID_I32);
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_NE(type, nullptr);

            auto param = std::make_unique<AbckitCoreFunctionParam>();
            param->function = method;

            auto paramNameStr = std::make_unique<AbckitString>();
            paramNameStr->impl = "newIntParam";
            param->name = paramNameStr.get();
            param->type = type;

            auto arktsParam = std::make_unique<AbckitArktsFunctionParam>();
            arktsParam->core = param.get();
            param->impl = std::move(arktsParam);

            auto *arktsFunc = method->GetArkTSImpl();
            bool success = g_arktsModifyApiImpl->functionAddParameter(arktsFunc, param->GetArkTSImpl());
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
            ASSERT_TRUE(success) << "Failed to add int parameter to function";
        });
    g_impl->writeAbc(file, MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    g_impl->closeFile(file);

    AbckitFile *modifiedFile = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_NE(modifiedFile, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    std::set<std::string> expectedParamTypes = {"functions_static.MyClass", "i32", "std.core.String"};
    ValidateFunctionParameters(modifiedFile, "InitFuncName", expectedParamTypes);

    g_impl->closeFile(modifiedFile);

    auto modifiedOutput = helpers::ExecuteStaticAbc(MODIFIED_STATIC, "functions_static", "main");
    ASSERT_TRUE(helpers::Match(modifiedOutput, output));
}

TEST_F(LibAbcKitModifyApiFunctionsTest, StaticFunctionRemoveParamExist)
{
    auto output = helpers::ExecuteStaticAbc(INITIAL_STATIC, "functions_static", "main");
    helpers::TransformMethod(
        INITIAL_STATIC, MODIFIED_STATIC, "add1",
        [&](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            AbckitArktsFunction *arktsFunc = method->GetArkTSImpl();

            bool success = g_arktsModifyApiImpl->functionRemoveParameter(arktsFunc, 0);
            ASSERT_TRUE(success) << "Failed to remove parameter from function";
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

            // delete the parameter in graph
            AbckitInst *deletedParma = g_implG->gGetParameter(graph, 0);
            AbckitInst *newInput = g_implG->gFindOrCreateConstantI32(graph, 12);
            // find all users of deletedParma, and set the input of user to a constant
            auto cbVisitUsers = [](AbckitInst *user, void *data) {
                [[maybe_unused]] AbckitInst *input = g_implG->iGetInput(user, 0);
                g_implG->iSetInput(user, reinterpret_cast<AbckitInst *>(data), 0);
                return true;
            };
            g_implG->iVisitUsers(deletedParma, newInput, cbVisitUsers);
            g_implG->iRemove(deletedParma);
        },
        [](AbckitGraph *) {});

    AbckitFile *modifiedFile = g_impl->openAbc(MODIFIED_STATIC, strlen(MODIFIED_STATIC));
    ASSERT_NE(modifiedFile, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);
    // check the number of parameters is reduced
    auto *modifiedMethod = helpers::FindMethodByName(modifiedFile, "add1");
    ASSERT_NE(modifiedMethod, nullptr);
    std::set<std::string> expectedParamTypes = {"i32"};
    ValidateFunctionParameters(modifiedFile, "add1", expectedParamTypes);
    g_impl->closeFile(modifiedFile);
}

TEST_F(LibAbcKitModifyApiFunctionsTest, StaticFunctionRemoveParamNonExist)
{
    auto output = helpers::ExecuteStaticAbc(INITIAL_STATIC, "functions_static", "main");
    helpers::TransformMethod(
        INITIAL_STATIC, MODIFIED_STATIC, "add1",
        [&](AbckitFile *file, AbckitCoreFunction *method, AbckitGraph *graph) {
            AbckitArktsFunction *arktsFunc = method->GetArkTSImpl();

            bool success = g_arktsModifyApiImpl->functionRemoveParameter(arktsFunc, 2);
            ASSERT_FALSE(success) << "Failed to remove parameter from function";
            ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_BAD_ARGUMENT);
        },
        [](AbckitGraph *) {});
}

}  // namespace libabckit::test
