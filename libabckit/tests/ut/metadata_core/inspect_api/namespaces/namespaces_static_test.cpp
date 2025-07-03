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

#include "libabckit/cpp/abckit_cpp.h"

#include <set>

namespace libabckit::test {

class LibAbcKitInspectApiNamespacesTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::namespaceGetInterfacesStatic, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceGetInterfacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectedInterfaceNames = {"I1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &iface : ns.GetInterfaces()) {
                gotInterfaceNames.emplace(iface.GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectedInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceGetEnumsStatic, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceGetEnumsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotEnumNames;
    std::set<std::string> expectedEnumNames = {"E1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &enm : ns.GetEnums()) {
                gotEnumNames.emplace(enm.GetName());
            }
        }
    }

    ASSERT_EQ(gotEnumNames, expectedEnumNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceGetFieldsStatic, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceGetFieldsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectedFieldNames = {"F1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &field : ns.GetFields()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectedFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceEnumerateTopLevelFunctions, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceEnumerateTopLevelFunctionsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotFunctionNames;
    std::set<std::string> expectedFunctionNames = {"M1:void;", "_cctor_:void;"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &function : ns.GetTopLevelFunctions()) {
                gotFunctionNames.emplace(function.GetName());
            }
        }
    }

    ASSERT_EQ(gotFunctionNames, expectedFunctionNames);
}

// Test: test-kind=api, api=InspectApiImpl::namespaceGetNamespacesStatic, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitInspectApiNamespacesTest, NamespaceGetNamespacesStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/namespaces/namespaces_static.abc");

    std::set<std::string> gotNamespaceNames;
    std::set<std::string> expectedNamespaceNames = {"N2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ns : module.GetNamespaces()) {
            for (const auto &childNs : ns.GetNamespaces()) {
                gotNamespaceNames.emplace(childNs.GetName());
            }
        }
    }

    ASSERT_EQ(gotNamespaceNames, expectedNamespaceNames);
}
}  // namespace libabckit::test