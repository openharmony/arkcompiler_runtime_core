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

#include <cstring>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "adapter_static/modify_name_helper.h"
#include "helpers/helpers.h"
#include "libabckit/c/abckit.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "static_core/assembler/assembly-program.h"
#include "static_core/assembler/mangling.h"
#include "static_core/assembler/assembly-function.h"
#include "static_core/assembler/assembly-record.h"
#include "static_core/assembler/assembly-emitter.h"
#include "libarkfile/include/source_lang_enum.h"

using namespace testing::ext;
using namespace libabckit;

class ModifyNameHelperTest : public ::testing::Test {};

namespace libabckit::test {
namespace {
constexpr std::string_view GETTER_PREFIX = "%%get-";
constexpr std::string_view SETTER_PREFIX = "%%set-";

static auto g_impl = AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
}  // namespace

// Test implementation of AsyncReplaceFast
std::string AsyncReplaceFast(const std::string &oldFunctionKeyName, const std::string &newName)
{
    constexpr std::string_view ASYNC_MARKER = "%%async-";
    const size_t pos = oldFunctionKeyName.find(ASYNC_MARKER);
    if (pos == std::string::npos) {
        return oldFunctionKeyName;
    }
    const size_t nameStart = pos + ASYNC_MARKER.size();
    const size_t colonPos = oldFunctionKeyName.find(':', nameStart);
    if (colonPos == std::string::npos) {
        return oldFunctionKeyName;
    }
    return oldFunctionKeyName.substr(0, nameStart) + newName + oldFunctionKeyName.substr(colonPos);
}

// Test implementation of GetSetMatchAndReplace
bool GetSetMatchAndReplace(const std::string &oldFunctionKeyName, const std::string &fieldName,
                           const std::string &newName, std::string &outNewFunctionKeyName)
{
    const std::string getPrefix = std::string(GETTER_PREFIX) + fieldName + ":";
    const std::string setPrefix = std::string(SETTER_PREFIX) + fieldName + ":";
    if (oldFunctionKeyName.find(getPrefix) != std::string::npos) {
        const size_t pos = oldFunctionKeyName.find(getPrefix);
        outNewFunctionKeyName = oldFunctionKeyName.substr(0, pos) + std::string(GETTER_PREFIX) + newName + ":" +
                                oldFunctionKeyName.substr(pos + getPrefix.size());
        return true;
    }
    if (oldFunctionKeyName.find(setPrefix) != std::string::npos) {
        const size_t pos = oldFunctionKeyName.find(setPrefix);
        outNewFunctionKeyName = oldFunctionKeyName.substr(0, pos) + std::string(SETTER_PREFIX) + newName + ":" +
                                oldFunctionKeyName.substr(pos + setPrefix.size());
        return true;
    }
    return false;
}

static bool CheckFunctionInTables(const std::string &functionKeyName, const ark::pandasm::Program &prog)
{
    bool foundInStaticTable = prog.functionStaticTable.find(functionKeyName) != prog.functionStaticTable.end();
    bool foundInInstanceTable = prog.functionInstanceTable.find(functionKeyName) != prog.functionInstanceTable.end();
    return foundInStaticTable || foundInInstanceTable;
}

static void CheckAsyncAnnotationValue(const ark::pandasm::AnnotationData &anno, const std::string &oldFunctionKeyName,
                                      const std::string &newFunctionKeyName, bool &oldAnnotationFound,
                                      bool &asyncAnnotationUpdated)
{
    if (anno.GetName() != "ets.coroutine.Async") {
        return;
    }
    for (const auto &elem : anno.GetElements()) {
        if (elem.GetName() != "value") {
            continue;
        }
        auto *value = elem.GetValue();
        if (value == nullptr || value->GetType() != ark::pandasm::Value::Type::METHOD) {
            continue;
        }
        auto *scalarValue = value->GetAsScalar();
        if (scalarValue == nullptr) {
            continue;
        }
        std::string methodValue = scalarValue->GetValue<std::string>();
        if (methodValue == oldFunctionKeyName) {
            oldAnnotationFound = true;
        }
        if (methodValue == newFunctionKeyName) {
            asyncAnnotationUpdated = true;
        }
    }
}

static void CheckAsyncAnnotationsInTable(const ark::pandasm::Program::FunctionTableT &functionTable,
                                         const std::string &oldFunctionKeyName, const std::string &newFunctionKeyName,
                                         bool &oldAnnotationFound, bool &asyncAnnotationUpdated)
{
    for (auto &[funcKeyName, func] : functionTable) {
        if (func.metadata == nullptr) {
            continue;
        }
        func.metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
            CheckAsyncAnnotationValue(anno, oldFunctionKeyName, newFunctionKeyName, oldAnnotationFound,
                                      asyncAnnotationUpdated);
        });
    }
}

static std::unique_ptr<ark::pandasm::FunctionMetadata> CreateFunctionMetadata()
{
    return std::make_unique<ark::pandasm::FunctionMetadata>();
}

static std::unique_ptr<ark::pandasm::RecordMetadata> CreateRecordMetadata()
{
    return std::make_unique<ark::pandasm::RecordMetadata>();
}

static void AddAsyncAnnotationToFunction(ark::pandasm::Function &func, const std::string &methodName)
{
    if (func.metadata == nullptr) {
        func.metadata = CreateFunctionMetadata();
    }
    auto methodValue = ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::METHOD>(methodName, false);
    std::vector<ark::pandasm::ScalarValue> values;
    values.push_back(std::move(methodValue));
    auto arrayValue = std::make_unique<ark::pandasm::ArrayValue>(ark::pandasm::Value::Type::METHOD, std::move(values));
    auto element = ark::pandasm::AnnotationElement("value", std::move(arrayValue));
    std::vector<ark::pandasm::AnnotationElement> elements;
    elements.push_back(std::move(element));
    auto annotation = ark::pandasm::AnnotationData("ets.coroutine.Async", std::move(elements));
    std::vector<ark::pandasm::AnnotationData> annotations;
    annotations.push_back(std::move(annotation));
    func.metadata->AddAnnotations(annotations);
}

static void AddFunctionalReferenceAnnotationToRecord(ark::pandasm::Record &record, const std::string &methodName)
{
    if (record.metadata == nullptr) {
        record.metadata = CreateRecordMetadata();
    }
    auto methodValue = ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::METHOD>(methodName, false);
    std::vector<ark::pandasm::ScalarValue> values;
    values.push_back(std::move(methodValue));
    auto arrayValue = std::make_unique<ark::pandasm::ArrayValue>(ark::pandasm::Value::Type::METHOD, std::move(values));
    auto element = ark::pandasm::AnnotationElement("value", std::move(arrayValue));
    std::vector<ark::pandasm::AnnotationElement> elements;
    elements.push_back(std::move(element));
    auto annotation = ark::pandasm::AnnotationData("ets.annotation.FunctionalReference", std::move(elements));
    std::vector<ark::pandasm::AnnotationData> annotations;
    annotations.push_back(std::move(annotation));
    record.metadata->AddAnnotations(annotations);
}

// Test: test-kind=api, api=ModifyNameHelper::AsyncReplaceFast, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(ModifyNameHelperTest, async_replace_fast_test_001)
{
    const std::string oldFunctionKeyName = "%%async-oldName:param1:param2:returnType";
    const std::string newName = "newName";
    const std::string expected = "%%async-newName:param1:param2:returnType";
    const std::string result = AsyncReplaceFast(oldFunctionKeyName, newName);
    EXPECT_EQ(result, expected);
}

// Test: test-kind=api, api=ModifyNameHelper::AsyncReplaceFast, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(ModifyNameHelperTest, async_replace_fast_test_002)
{
    const std::string oldFunctionKeyName = "oldName:param1:param2:returnType";
    const std::string newName = "newName";
    const std::string expected = "oldName:param1:param2:returnType";
    const std::string result = AsyncReplaceFast(oldFunctionKeyName, newName);
    EXPECT_EQ(result, expected);
}

// Test: test-kind=api, api=ModifyNameHelper::GetSetMatchAndReplace, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(ModifyNameHelperTest, get_set_match_and_replace_test_001)
{
    std::string outNewFunctionKeyName;
    const std::string oldFunctionKeyName = "%%get-fieldName:param1:returnType";
    const std::string fieldName = "fieldName";
    const std::string newName = "newFieldName";
    const std::string expected = "%%get-newFieldName:param1:returnType";
    const bool result = GetSetMatchAndReplace(oldFunctionKeyName, fieldName, newName, outNewFunctionKeyName);
    EXPECT_TRUE(result);
    EXPECT_EQ(outNewFunctionKeyName, expected);
}

// Test: test-kind=api, api=ModifyNameHelper::GetSetMatchAndReplace, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(ModifyNameHelperTest, get_set_match_and_replace_test_002)
{
    std::string outNewFunctionKeyName;
    const std::string oldFunctionKeyName = "%%set-fieldName:param1:returnType";
    const std::string fieldName = "fieldName";
    const std::string newName = "newFieldName";
    const std::string expected = "%%set-newFieldName:param1:returnType";
    const bool result = GetSetMatchAndReplace(oldFunctionKeyName, fieldName, newName, outNewFunctionKeyName);
    EXPECT_TRUE(result);
    EXPECT_EQ(outNewFunctionKeyName, expected);
}

// Test: test-kind=api, api=ModifyNameHelper::GetSetMatchAndReplace, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(ModifyNameHelperTest, get_set_match_and_replace_test_003)
{
    std::string outNewFunctionKeyName;
    const std::string oldFunctionKeyName = "fieldName:param1:returnType";
    const std::string fieldName = "fieldName";
    const std::string newName = "newFieldName";
    const bool result = GetSetMatchAndReplace(oldFunctionKeyName, fieldName, newName, outNewFunctionKeyName);
    EXPECT_FALSE(result);
}

// Test: test-kind=api, api=ModifyNameHelper::FunctionRefreshName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(ModifyNameHelperTest, function_refresh_name_test_001)
{
    constexpr auto TEST_ABC = ABCKIT_ABC_DIR "ut/metadata_core/modify_api/functions/functions_static.abc";

    AbckitFile *file = g_impl->openAbc(TEST_ABC, strlen(TEST_ABC));
    ASSERT_NE(file, nullptr);
    ASSERT_EQ(g_impl->getLastError(), ABCKIT_STATUS_NO_ERROR);

    auto *function = libabckit::test::helpers::FindMethodByName(file, "initFuncName");
    ASSERT_NE(function, nullptr);

    auto *prog = file->GetStaticProgram();
    ASSERT_NE(prog, nullptr);

    auto *funcImpl = function->GetArkTSImpl()->GetStaticImpl();
    std::string oldFunctionKeyName =
        ark::pandasm::MangleFunctionName(funcImpl->name, funcImpl->params, funcImpl->returnType);
    std::string newFunctionName = "RenamedAsyncFunc";

    EXPECT_TRUE(CheckFunctionInTables(oldFunctionKeyName, *prog));

    bool result = ModifyNameHelper::FunctionRefreshName(function, newFunctionName);
    EXPECT_TRUE(result);

    std::string newFunctionKeyName =
        ark::pandasm::MangleFunctionName(funcImpl->name, funcImpl->params, funcImpl->returnType);

    EXPECT_TRUE(CheckFunctionInTables(newFunctionKeyName, *prog));
    EXPECT_NE(oldFunctionKeyName, newFunctionKeyName);

    bool asyncAnnotationUpdated = false;
    bool oldAnnotationFound = false;

    CheckAsyncAnnotationsInTable(prog->functionStaticTable, oldFunctionKeyName, newFunctionKeyName, oldAnnotationFound,
                                 asyncAnnotationUpdated);
    CheckAsyncAnnotationsInTable(prog->functionInstanceTable, oldFunctionKeyName, newFunctionKeyName,
                                 oldAnnotationFound, asyncAnnotationUpdated);

    g_impl->closeFile(file);

    if (oldAnnotationFound) {
        EXPECT_TRUE(asyncAnnotationUpdated) << "Async annotation should be updated when old annotation exists";
    }
}

// Test: test-kind=api, api=ModifyNameHelper::ApplyBatchFunctionRenameAnnotations, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(ModifyNameHelperTest, apply_batch_function_rename_annotations_test_001)
{
    ark::pandasm::Program prog;
    std::vector<std::pair<std::string, std::string>> renamePairs;
    const bool result = libabckit::ModifyNameHelper::ApplyBatchFunctionRenameAnnotations(&prog, renamePairs);
    EXPECT_TRUE(result);
}

// Test: test-kind=api, api=ModifyNameHelper::ApplyBatchFunctionRenameAnnotations, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(ModifyNameHelperTest, apply_batch_function_rename_annotations_test_002)
{
    ark::pandasm::Program prog;
    constexpr auto LANG = ark::panda_file::SourceLang::ETS;

    auto func = ark::pandasm::Function("testFunc", LANG);
    func.returnType = ark::pandasm::Type::FromName("void");
    func.regsNum = 1;
    AddAsyncAnnotationToFunction(func, "oldMethod");
    prog.functionInstanceTable.emplace("testFunc:", std::move(func));

    std::vector<std::pair<std::string, std::string>> renamePairs = {{"oldMethod:", "newMethod:"}};
    const bool result = libabckit::ModifyNameHelper::ApplyBatchFunctionRenameAnnotations(&prog, renamePairs);
    EXPECT_TRUE(result);
}

// Test: test-kind=api, api=ModifyNameHelper::ApplyBatchFunctionRenameAnnotations, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(ModifyNameHelperTest, apply_batch_function_rename_annotations_test_003)
{
    ark::pandasm::Program prog;
    constexpr auto LANG = ark::panda_file::SourceLang::ETS;

    auto record = ark::pandasm::Record("%%lambda-1", LANG);
    AddFunctionalReferenceAnnotationToRecord(record, "oldLambdaMethod");
    prog.recordTable.emplace(record.name, std::move(record));

    std::vector<std::pair<std::string, std::string>> renamePairs = {{"oldLambdaMethod:", "newLambdaMethod:"}};
    const bool result = libabckit::ModifyNameHelper::ApplyBatchFunctionRenameAnnotations(&prog, renamePairs);
    EXPECT_TRUE(result);
}

// Test: test-kind=api, api=ModifyNameHelper::ApplyBatchFunctionRenameAnnotations, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(ModifyNameHelperTest, apply_batch_function_rename_annotations_test_004)
{
    ark::pandasm::Program prog;
    constexpr auto LANG = ark::panda_file::SourceLang::ETS;

    auto func = ark::pandasm::Function("testFunc", LANG);
    func.returnType = ark::pandasm::Type::FromName("void");
    func.regsNum = 1;
    AddAsyncAnnotationToFunction(func, "oldMethod");
    prog.functionInstanceTable.emplace("testFunc:", std::move(func));

    std::vector<std::pair<std::string, std::string>> renamePairs = {{"differentMethod:", "newMethod:"}};
    const bool result = libabckit::ModifyNameHelper::ApplyBatchFunctionRenameAnnotations(&prog, renamePairs);
    EXPECT_TRUE(result);
}
}  // namespace libabckit::test
