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

#include "libabckit/cpp/abckit_cpp.h"

#include "helpers/helpers_runtime.h"
#include "helpers/helpers.h"
#include "libabckit/c/metadata_core.h"

#include <gtest/gtest.h>
#include <string_view>

namespace libabckit::test {

class LibAbcKitNameUtilTest : public ::testing::Test {};
// CC-OFFNXT(WordsTool.95) sensitive word conflict
using namespace abckit;

// Test: test-kind=api, api=NameUtil::GetName, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(LibAbcKitNameUtilTest, TestNameUtilGetName)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    for (auto &module : file.GetModules()) {
        // Test module name
        std::string moduleName = module.GetName();
        EXPECT_FALSE(moduleName.empty());

        // Test class names
        for (auto &klass : module.GetClasses()) {
            std::string className = klass.GetName();
            EXPECT_FALSE(className.empty());
        }

        // Test namespace names
        for (auto &ns : module.GetNamespaces()) {
            std::string nsName = ns.GetName();
            EXPECT_FALSE(nsName.empty());
        }

        // Test enum names
        for (auto &enm : module.GetEnums()) {
            std::string enumName = enm.GetName();
            EXPECT_FALSE(enumName.empty());
        }

        // Test interface names
        for (auto &iface : module.GetInterfaces()) {
            std::string ifaceName = iface.GetName();
            EXPECT_FALSE(ifaceName.empty());
        }
    }
}

// Test: test-kind=api, api=NameUtil::GetName, abc-kind=ArkTS1, category=positive, extension=cpp
TEST_F(LibAbcKitNameUtilTest, TestNameUtilGetNameOnly)
{
    abckit::File file(ABCKIT_ABC_DIR "cpp/tests/cpp_test_dynamic.abc");

    for (auto &module : file.GetModules()) {
        // Test module name
        std::string moduleName = module.GetName();
        EXPECT_FALSE(moduleName.empty());

        // Test class names
        for (auto &klass : module.GetClasses()) {
            std::string className = klass.GetName();
            EXPECT_FALSE(className.empty());
        }

        // Test namespace names
        for (auto &ns : module.GetNamespaces()) {
            std::string nsName = ns.GetName();
            EXPECT_FALSE(nsName.empty());
        }

        // Test enum names
        for (auto &enm : module.GetEnums()) {
            std::string enumName = enm.GetName();
            EXPECT_FALSE(enumName.empty());
        }

        // Test interface names
        for (auto &iface : module.GetInterfaces()) {
            std::string ifaceName = iface.GetName();
            EXPECT_FALSE(ifaceName.empty());
        }

        // Test function names
        for (auto &function : module.GetTopLevelFunctions()) {
            std::string functionName = function.GetName();
            EXPECT_FALSE(functionName.empty());
        }
    }
}

}  // namespace libabckit::test
