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
#include <string>

#include "libabckit/cpp/abckit_cpp.h"

#include <set>

namespace libabckit::test {

class LibAbcKitInspectApiModulesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateFields, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleGetFieldsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc");

    std::vector<std::string> fieldNames;

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &field : module.GetFields()) {
            fieldNames.emplace_back(field.GetName());
        }
    }

    ASSERT_EQ(fieldNames[0], "moduleField");
    ASSERT_EQ(fieldNames.size(), 1);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateInterfaces, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleGetInterfacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc");

    std::vector<std::string> interfacesNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &iface : module.GetInterfaces()) {
            interfacesNames.emplace_back(iface.GetName());
        }
    }

    ASSERT_EQ(interfacesNames[0], "InterfaceA");
    ASSERT_EQ(interfacesNames.size(), 1);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateEnums, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleGetEnumsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc");

    std::vector<std::string> enumNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            enumNames.emplace_back(enm.GetName());
        }
    }

    ASSERT_EQ(enumNames[0], "EnumA");
    ASSERT_EQ(enumNames.size(), 1);
}

// Test: test-kind=api, api=InspectApiImpl::moduleEnumerateNamespaces, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModulesTest, ModuleGetNamespacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/modules/modules_static.abc");

    std::set<std::string> gotNamespaceNames;
    std::set<std::string> expectNamespacesNames = {"Ns1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            gotNamespaceNames.emplace(ns.GetName());
        }
    }

    ASSERT_EQ(gotNamespaceNames, expectNamespacesNames);
}
}  // namespace libabckit::test