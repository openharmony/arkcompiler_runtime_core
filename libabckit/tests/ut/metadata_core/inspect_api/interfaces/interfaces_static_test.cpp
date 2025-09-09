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
#include <string>
#include <optional>

#include "libabckit/cpp/abckit_cpp.h"
#include "helpers/helpers.h"

namespace libabckit::test {

class LibAbcKitInspectApiInterfacesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::interfaceGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"InterfaceA", "InterfaceB", "InterfaceC"};

    for (const auto &module : file.GetModules()) {
        for (const auto &iface : module.GetInterfaces()) {
            gotInterfaceNames.emplace(iface.GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateMethods, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetAllMethodsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotMethodNames;

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceA");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &method : iface.GetAllMethods()) {
            gotMethodNames.emplace(method.GetName());
        }
    }

    std::set<std::string> expectMethodNames = {
        "<get>fieldA:interfaces_static.InterfaceA;std.core.String;", "<get>fieldB:interfaces_static.InterfaceA;f64;",
        "<set>fieldA:interfaces_static.InterfaceA;std.core.String;void;", "bar:interfaces_static.InterfaceA;void;"};
    ASSERT_EQ(gotMethodNames, expectMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateFields, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceGetFieldsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotFieldNames;
    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceA");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &field : iface.GetFields()) {
            gotFieldNames.emplace(field.GetName());
        }
    }

    std::set<std::string> expectFieldNames = {"fieldA", "fieldB"};
    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateSuperInterfaces, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceEnumerateSuperInterfacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"InterfaceA"};
    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceB");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &superInterface : iface.GetSuperInterfaces()) {
            gotInterfaceNames.emplace(superInterface.GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateSubInterfaces, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceEnumerateSubInterfacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"InterfaceB"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceA");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &subInterface : iface.GetSubInterfaces()) {
            gotInterfaceNames.emplace(subInterface.GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceEnumerateClasses, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiInterfacesTest, InterfaceEnumerateClassesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/interfaces/interfaces_static.abc");

    std::set<std::string> gotClassNames;
    std::set<std::string> expectClassNames = {"ClassB"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "InterfaceC");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &klass : iface.GetClasses()) {
            gotClassNames.emplace(klass.GetName());
        }
    }

    ASSERT_EQ(gotClassNames, expectClassNames);
}

}  // namespace libabckit::test