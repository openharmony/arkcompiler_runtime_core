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

class LibAbcKitInspectApiAnnotationsTest : public ::testing::Test {};

// Test: test-kind=api, api=InspectApiImpl::annotationInterfaceGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, AnnotationInterfaceGetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI1", "AI2", "AI3", "AI4"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ai : module.GetAnnotationInterfaces()) {
            gotInterfaceNames.emplace(ai.GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, AnnotationGetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc");

    std::set<std::string> gotAnnotationNames;
    std::set<std::string> expectAnnotationNames = {"AI1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &anno : klass.GetAnnotations()) {
            gotAnnotationNames.emplace(anno.GetName());
        }
    }

    ASSERT_EQ(gotAnnotationNames, expectAnnotationNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationIsExternal, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, AnnotationIsExternalStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc");

    std::set<std::string> gotInternalAnnotationNames;
    std::set<std::string> gotExternalAnnotationNames;
    std::set<std::string> expectInternalAnnotationNames = {"AI4"};
    std::set<std::string> expectExternalAnnotationNames = {"EnclosingClass", "InnerClass"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetNamespaceByName(module, "Ns1");
        ASSERT_NE(result, std::nullopt);
        const auto &ns = result.value();
        const auto &classes = ns.GetClasses();
        const auto &klass = classes.front();
        for (const auto &anno : klass.GetAnnotations()) {
            if (anno.IsExternal()) {
                gotExternalAnnotationNames.emplace(anno.GetName());
                continue;
            }
            gotInternalAnnotationNames.emplace(anno.GetName());
        }
    }

    ASSERT_EQ(gotInternalAnnotationNames, expectInternalAnnotationNames);
    ASSERT_EQ(gotExternalAnnotationNames, expectExternalAnnotationNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, ClassAnnotationGetInterfaceStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &klass : module.GetClasses()) {
            for (const auto &anno : klass.GetAnnotations()) {
                gotInterfaceNames.emplace(anno.GetInterface().GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, InterfaceAnnotationGetInterfaceStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI3"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "I1");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &anno : iface.GetAnnotations()) {
            gotInterfaceNames.emplace(anno.GetInterface().GetName());
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, ClassFieldAnnotationGetInterfaceStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "C1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &field : klass.GetFields()) {
            for (const auto &anno : field.GetAnnotations()) {
                gotInterfaceNames.emplace(anno.GetInterface().GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, InterfaceFieldAnnotationGetInterfaceStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "I1");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &field : iface.GetFields()) {
            for (const auto &anno : field.GetAnnotations()) {
                gotInterfaceNames.emplace(anno.GetInterface().GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, FunctionAnnotationGetInterfaceStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc");

    std::set<std::string> gotInterfaceNames;
    std::set<std::string> expectInterfaceNames = {"AI3"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &function : module.GetTopLevelFunctions()) {
            for (const auto &anno : function.GetAnnotations()) {
                gotInterfaceNames.emplace(anno.GetInterface().GetName());
            }
        }
    }

    ASSERT_EQ(gotInterfaceNames, expectInterfaceNames);
}

// Test: test-kind=api, api=InspectApiImpl::annotationGetInterface, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitInspectApiAnnotationsTest, AnnotationInterfaceGetAnnotationInterfaceFieldStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/metadata_core/inspect_api/annotations/annotations_static.abc");

    std::set<std::string> gotFieldNames;
    std::set<std::string> expectFieldNames = {"f1"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &ai : module.GetAnnotationInterfaces()) {
            for (const auto &field : ai.GetFields()) {
                gotFieldNames.emplace(field.GetName());
            }
        }
    }

    ASSERT_EQ(gotFieldNames, expectFieldNames);
}

}  // namespace libabckit::test