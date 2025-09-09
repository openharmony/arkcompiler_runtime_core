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

class LibAbcKitInspectApiModuleFieldsTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::classFieldIsPublic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModuleFieldsTest, ClassFieldIsPublicStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"C1F1", "C1F4"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            if (field.IsPublic()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldIsProtected, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModuleFieldsTest, ClassFieldIsProtectedStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"C1F2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            if (field.IsProtected()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldIsPrivate, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModuleFieldsTest, ClassFieldIsPrivateStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"C1F3"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            if (field.IsPrivate()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::classFieldIsStatic, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModuleFieldsTest, ClassFieldIsStaticStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"C1F4"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            if (field.IsStatic()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

// Test: test-kind=api, api=InspectApiImpl::interfaceFieldIsReadonly, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiModuleFieldsTest, InterfaceFieldIsReadonlyStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/fields/fields_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"I1F2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "I1");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &field : iface.GetFields()) {
            if (field.IsReadonly()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

}  // namespace libabckit::test