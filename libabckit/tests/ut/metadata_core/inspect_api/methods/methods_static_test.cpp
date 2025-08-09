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

#include <set>
#include <string>
#include <gtest/gtest.h>

#include "libabckit/cpp/abckit_cpp.h"
#include "helpers/helpers.h"

namespace libabckit::test {

class LibAbcKitInspectApiMethodsTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::functionEnumerateParameters, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionGetParametersStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotParamTypeNames;
    std::set<std::string> expectParamTypeNames = {"std.core.String", "escompat.Array"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetFunctionByName(module, "m0F1:std.core.String;escompat.Array;void;");
        ASSERT_NE(result, std::nullopt);
        const auto &func = result.value();
        for (const auto &param : func.GetParameters()) {
            gotParamTypeNames.emplace(param.GetType().GetName());
        }
    }

    ASSERT_EQ(gotParamTypeNames, expectParamTypeNames);
}

// Test: test-kind=api, api=InspectApiImpl::functionGetReturnType, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiMethodsTest, FunctionGetReturnTypeStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/methods/methods_static_other.abc");

    std::set<std::string> gotReturnTypeNames;
    std::set<std::string> expectReturnTypeNames = {"void"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &func : module.GetTopLevelFunctions()) {
            if (func.GetName() == "m0F1:std.core.String;escompat.Array;void;") {
                gotReturnTypeNames.emplace(func.GetReturnType().GetName());
            }
        }
    }

    ASSERT_EQ(gotReturnTypeNames, expectReturnTypeNames);
}
}  // namespace libabckit::test