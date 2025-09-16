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

class LibAbcKitInspectApiEnumsTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::enumGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiEnumsTest, EnumGetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enums/enums_static.abc");

    std::vector<std::string> enumNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            enumNames.emplace_back(enm.GetName());
        }
    }

    ASSERT_EQ(enumNames[0], "EnumA");
    ASSERT_EQ(enumNames.size(), 1);
}

// Test: test-kind=api, api=InspectApiImpl::enumEnumerateMethods, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiEnumsTest, EnumGetAllMethodsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enums/enums_static.abc");

    std::vector<std::string> methodNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            for (const auto &method : enm.GetAllMethods()) {
                methodNames.emplace_back(method.GetName());
            }
        }
    }

    std::vector<std::string> actualMethodNames = {"$_get:enums_static.EnumA;std.core.String;",
                                                  "_cctor_:void;",
                                                  "fromValue:i32;enums_static.EnumA;",
                                                  "getValueOf:std.core.String;enums_static.EnumA;",
                                                  "values:enums_static.EnumA[];",
                                                  "_ctor_:enums_static.EnumA;i32;i32;void;",
                                                  "getName:enums_static.EnumA;std.core.String;",
                                                  "getOrdinal:enums_static.EnumA;i32;",
                                                  "toString:enums_static.EnumA;std.core.String;",
                                                  "valueOf:enums_static.EnumA;i32;"};
    sort(actualMethodNames.begin(), actualMethodNames.end());
    sort(methodNames.begin(), methodNames.end());
    ASSERT_EQ(methodNames, actualMethodNames);
}

// Test: test-kind=api, api=InspectApiImpl::enumEnumerateFields, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiEnumsTest, EnumGetFieldsStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/enums/enums_static.abc");

    std::vector<std::string> fieldNames;

    for (const auto &module : file.GetModules()) {
        for (const auto &enm : module.GetEnums()) {
            for (const auto &field : enm.GetFields()) {
                fieldNames.emplace_back(field.GetName());
            }
        }
    }

    std::vector<std::string> actualFieldName = {
        "_ordinal", "ONE", "TWO", "THREE", "_NamesArray", "_ValuesArray", "_StringValuesArray", "_ItemsArray"};

    sort(actualFieldName.begin(), actualFieldName.end());
    sort(fieldNames.begin(), fieldNames.end());
    ASSERT_EQ(fieldNames, actualFieldName);
}

}  // namespace libabckit::test