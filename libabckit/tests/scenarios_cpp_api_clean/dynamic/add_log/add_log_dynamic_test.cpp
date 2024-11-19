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

#include "libabckit/include/cpp/abckit_cpp.h"

#include "helpers/helpers.h"
#include "helpers/helpers_runtime.h"

#include <optional>
#include <string_view>
#include <regex>

#include <gtest/gtest.h>

namespace libabckit::test {

namespace {

using FunctionCallback = std::function<bool(abckit::core::Function)>;
using ClassCallback = std::function<bool(abckit::core::Class)>;
using NamespaceCallback = std::function<bool(abckit::core::Namespace)>;

class GTestAssertErrorHandler final : public abckit::IErrorHandler {
public:
    void HandleError(abckit::Exception &&err) override
    {
        EXPECT_TRUE(false) << "Abckit expection raised: " << err.What();
    }
};

struct UserData {
    std::string_view print;
    std::string_view date;
    std::string_view getTime;
    std::string_view str;
    std::string_view consume;
};

std::string_view GetMethodName(const abckit::core::Function &method)
{
    const auto name = method.GetName();
    return name.substr(0, name.find(':'));
}

void EnumerateAllMethods(const abckit::File &file, const FunctionCallback &fnUserCallback)
{
    ClassCallback clsCallback;

    FunctionCallback fnCallback = [&](const abckit::core::Function &fun) -> bool {
        if (!fnUserCallback(fun)) {
            return false;
        }
        fun.EnumerateNestedFunctions(fnCallback);
        fun.EnumerateNestedClasses(clsCallback);
        return true;
    };

    clsCallback = [&](const abckit::core::Class &cls) -> bool {
        cls.EnumerateMethods(fnCallback);
        return true;
    };

    NamespaceCallback nsCallback = [&](const abckit::core::Namespace &ns) -> bool {
        ns.EnumerateNamespaces(nsCallback);
        ns.EnumerateClasses(clsCallback);
        ns.EnumerateTopLevelFunctions(fnCallback);
        return true;
    };

    file.EnumerateModules([&](const abckit::core::Module &mod) -> bool {
        mod.EnumerateNamespaces(nsCallback);
        mod.EnumerateClasses(clsCallback);
        mod.EnumerateTopLevelFunctions(fnCallback);
        return true;
    });
}

abckit::Instruction CreateProlog(abckit::Graph &graph, const UserData &userData)
{
    const abckit::BasicBlock startBB = graph.GetStartBb();
    EXPECT_GT(startBB.GetSuccCount(), 0);
    auto bb = startBB.GetSuccByIdx(0);

    auto iStr = graph.DynIsa().CreateLoadString(userData.str);
    bb.AddInstFront(iStr);

    auto iPrint = graph.DynIsa().CreateTryldglobalbyname(userData.print).InsertAfter(iStr);
    auto iCallArg = graph.DynIsa().CreateCallArg1(iPrint, iStr).InsertAfter(iPrint);
    auto iDateClass = graph.DynIsa().CreateTryldglobalbyname(userData.date).InsertAfter(iCallArg);
    auto iDateObj = graph.DynIsa().CreateNewobjrange(iDateClass).InsertAfter(iDateClass);
    auto iGetTime = graph.DynIsa().CreateLdObjByName(iDateObj, userData.getTime).InsertAfter(iDateObj);
    auto iTimeStart = graph.DynIsa().CreateCallThis0(iGetTime, iDateObj).InsertAfter(iGetTime);
    return iTimeStart;
}

void CreateEpilog(abckit::Graph &graph, const abckit::BasicBlock &bb, const abckit::Instruction &iTimeStart,
                  const UserData &userData)
{
    for (abckit::Instruction inst = bb.GetFirstInst(); !!inst; inst = inst.GetNext()) {
        if (inst.GetOpcodeDyn() != ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED) {
            continue;
        }

        auto iUndef = inst.GetPrev();
        auto iDateClass = graph.DynIsa().CreateTryldglobalbyname(userData.date).InsertBefore(iUndef);

        auto iDateObj = graph.DynIsa().CreateNewobjrange(iDateClass).InsertAfter(iDateClass);
        auto iGetTime = graph.DynIsa().CreateLdObjByName(iDateObj, userData.getTime).InsertAfter(iDateObj);
        auto iTimeEnd = graph.DynIsa().CreateCallThis0(iGetTime, iDateObj).InsertAfter(iGetTime);
        auto iConsume = graph.DynIsa().CreateLoadString(userData.consume).InsertAfter(iTimeEnd);
        auto iPrint = graph.DynIsa().CreateTryldglobalbyname(userData.print).InsertAfter(iConsume);
        auto iCallPrintConsume = graph.DynIsa().CreateCallArg1(iPrint, iConsume).InsertAfter(iPrint);
        auto iSub = graph.DynIsa().CreateSub2(iTimeStart, iTimeEnd).InsertAfter(iCallPrintConsume);
        auto iCallPrintSub = graph.DynIsa().CreateCallArg1(iPrint, iSub).InsertAfter(iSub);
    }
}

void TransformIr(abckit::Graph &graph, const UserData &userData)
{
    const abckit::Instruction iTimeStart = CreateProlog(graph, userData);
    graph.EnumerateBasicBlocksRpo([&](const abckit::BasicBlock &bb) { CreateEpilog(graph, bb, iTimeStart, userData); });
}

}  // namespace

class AbckitScenarioCPPTestClean : public ::testing::Test {};

// CC-OFFNXT(huge_method, C_RULE_ID_FUNCTION_SIZE) test, solid logic
// Test: test-kind=scenario, abc-kind=ArkTS1, category=positive
TEST_F(AbckitScenarioCPPTestClean, LibAbcKitTestDynamicAddLogClean)
{
    const std::string testSandboxPath = ABCKIT_ABC_DIR "scenarios_cpp_api_clean/dynamic/add_log/";
    const std::string srcAbcPath = testSandboxPath + "add_log_dynamic.abc";
    const std::string dstAbcPath = testSandboxPath + "add_log_dynamic_modified.abc";

    const UserData data {
        "print",                                        // .print
        "Date",                                         // .date
        "getTime",                                      // .getTime
        "file: src/MyClass, function: MyClass.handle",  // .str
        "Ellapsed time:",                               // .consume
    };

    // ExecuteDynamicAbc is helper function needed for testing. Вoes not affect the logic of IR transformation
    auto output = helpers::ExecuteDynamicAbc(srcAbcPath, "add_log_dynamic");
    EXPECT_TRUE(helpers::Match(output, "abckit\n"));

    {
        abckit::File file(srcAbcPath, std::make_unique<GTestAssertErrorHandler>());

        std::optional<abckit::core::Function> handleMethod;
        EnumerateAllMethods(file, [&](const abckit::core::Function &method) -> bool {
            auto methodName = GetMethodName(method);
            if (methodName == "handle") {
                handleMethod = method;
                return false;
            }
            return true;
        });
        EXPECT_TRUE(handleMethod.has_value());

        abckit::Graph graph = handleMethod->GetGraph();
        TransformIr(graph, data);
        handleMethod->SetGraph(graph);

        file.WriteAbc(dstAbcPath);
    }

    output = helpers::ExecuteDynamicAbc(dstAbcPath, "add_log_dynamic");
    EXPECT_TRUE(helpers::Match(output,
                               "file: src/MyClass, function: MyClass.handle\n"
                               "abckit\n"
                               "Ellapsed time:\n"
                               "\\d+\n"));
}

}  // namespace libabckit::test
