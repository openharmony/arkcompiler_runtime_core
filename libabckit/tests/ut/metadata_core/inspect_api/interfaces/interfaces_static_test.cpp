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
#include <vector>
#include <algorithm>
#include <string>

#include "libabckit/cpp/abckit_cpp.h"

namespace libabckit::test {

class LibAbcKitInspectApiInterfacesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::interfaceGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::vector<std::string> interfaceNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &iface : module.GetInterfaces()) {
            interfaceNames.emplace_back(iface.GetName());
        }
    }

    ASSERT_EQ(interfaceNames[0], "InterfaceA");
    ASSERT_EQ(interfaceNames.size(), 1);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateMethods, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetAllMethodsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::vector<std::string> methodNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &iface : module.GetInterfaces()) {
            for (const auto &method : iface.GetAllMethods()) {
                methodNames.emplace_back(method.GetName());
            }
        }
    }

    std::vector<std::string> actualMethodNames = {
        "<get>fieldA:interfaces_static.InterfaceA;std.core.String;", "<get>fieldB:interfaces_static.InterfaceA;f64;",
        "<set>fieldA:interfaces_static.InterfaceA;std.core.String;void;", "bar:interfaces_static.InterfaceA;void;"};
    sort(methodNames.begin(), methodNames.end());

    ASSERT_EQ(methodNames, actualMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateFields, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetFieldsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::vector<std::string> fieldNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &iface : module.GetInterfaces()) {
            for (const auto &field : iface.GetFields()) {
                fieldNames.emplace_back(field.GetName());
            }
        }
    }

    std::vector<std::string> actualFieldNames = {"fieldA", "fieldB"};
    sort(fieldNames.begin(), fieldNames.end());

    ASSERT_EQ(fieldNames, actualFieldNames);
}

}  // namespace libabckit::test