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
#include "tests/helpers/helpers_runtime.h"
#include "tests/helpers/helpers.h"

namespace {
constexpr auto INPUT_PATH = ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_static_modify.abc";
constexpr auto OUTPUT_PATH =
    ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_static_modify_updated.abc";
}  // namespace

namespace libabckit::test {
class LibAbcKitArkTSModifyApiAnnotationsTest : public ::testing::Test {};

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationSetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiAnnotationsTest, ClassAnnotationSetNameStatic)
{
    abckit::File file(INPUT_PATH);

    std::set<std::string> gotAnnotationNames;
    std::set<std::string> expectAnnotationNames = {"New", "Anno2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "Class1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        for (const auto &anno : klass.GetAnnotations()) {
            if (anno.GetName() == "Anno1") {
                abckit::arkts::Annotation(anno).SetName("New");
            }
        }
        for (const auto &anno : klass.GetAnnotations()) {
            gotAnnotationNames.emplace(anno.GetName());
        }
    }
    file.WriteAbc(OUTPUT_PATH);

    ASSERT_EQ(gotAnnotationNames, expectAnnotationNames);
    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "annotations_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "annotations_static_modify", "main"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationSetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiAnnotationsTest, ClassFieldAnnotationSetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_static_modify.abc");

    std::set<std::string> gotAnnotationNames;
    std::set<std::string> expectAnnotationNames = {"New", "Anno2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetClassByName(module, "Class1");
        ASSERT_NE(result, std::nullopt);
        const auto &klass = result.value();
        const auto &fields = klass.GetFields();
        const auto &field = fields[0];
        for (const auto &anno : field.GetAnnotations()) {
            if (anno.GetName() == "Anno1") {
                abckit::arkts::Annotation(anno).SetName("New");
            }
        }
        for (const auto &anno : field.GetAnnotations()) {
            gotAnnotationNames.emplace(anno.GetName());
        }
    }
    file.WriteAbc(OUTPUT_PATH);

    ASSERT_EQ(gotAnnotationNames, expectAnnotationNames);
    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "annotations_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "annotations_static_modify", "main"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationSetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiAnnotationsTest, InterfaceAnnotationSetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_static_modify.abc");

    std::set<std::string> gotAnnotationNames;
    std::set<std::string> expectAnnotationNames = {"New", "Anno2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "Interface1");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        for (const auto &anno : iface.GetAnnotations()) {
            if (anno.GetName() == "Anno1") {
                abckit::arkts::Annotation(anno).SetName("New");
            }
        }
        for (const auto &anno : iface.GetAnnotations()) {
            gotAnnotationNames.emplace(anno.GetName());
        }
    }
    file.WriteAbc(OUTPUT_PATH);

    ASSERT_EQ(gotAnnotationNames, expectAnnotationNames);
    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "annotations_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "annotations_static_modify", "main"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationSetName, abc-kind=ArkTS2, category=positive, extension=c
TEST_F(LibAbcKitArkTSModifyApiAnnotationsTest, InterfaceFieldAnnotationSetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_static_modify.abc");

    std::set<std::string> gotAnnotationNames;
    std::set<std::string> expectAnnotationNames = {"New", "Anno2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        auto result = helpers::GetInterfaceByName(module, "Interface1");
        ASSERT_NE(result, std::nullopt);
        const auto &iface = result.value();
        const auto &fields = iface.GetFields();
        const auto &field = fields[0];
        for (const auto &anno : field.GetAnnotations()) {
            if (anno.GetName() == "Anno1") {
                abckit::arkts::Annotation(anno).SetName("New");
            }
        }
        for (const auto &anno : field.GetAnnotations()) {
            gotAnnotationNames.emplace(anno.GetName());
        }
    }
    file.WriteAbc(OUTPUT_PATH);

    ASSERT_EQ(gotAnnotationNames, expectAnnotationNames);
    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "annotations_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "annotations_static_modify", "main"));
}

// Test: test-kind=api, api=ArktsModifyApiImpl::annotationInterfaceSetName, abc-kind=ArkTS2, category=positive,
// extension=c
TEST_F(LibAbcKitArkTSModifyApiAnnotationsTest, AnnotationInterfaceSetNameStatic)
{
    abckit::File file(ABCKIT_ABC_DIR "ut/extensions/arkts/modify_api/annotations/annotations_static_modify.abc");

    std::set<std::string> classGotAnnotationNames;
    std::set<std::string> classFieldGotAnnotationNames;
    std::set<std::string> interfaceGotAnnotationNames;
    std::set<std::string> interfaceFieldGotAnnotationNames;
    std::set<std::string> expectAnnotationNames = {"New", "Anno2"};

    for (const auto &module : file.GetModules()) {
        if (module.IsExternal()) {
            continue;
        }
        for (const auto &anno : module.GetAnnotationInterfaces()) {
            if (anno.GetName() == "Anno1") {
                abckit::arkts::AnnotationInterface(anno).SetName("New");
            }
        }
        auto result1 = helpers::GetClassByName(module, "Class1");
        const auto &klass = result1.value();
        const auto &classFields = klass.GetFields();
        const auto &classField = classFields[0];
        for (const auto &anno : klass.GetAnnotations()) {
            classGotAnnotationNames.emplace(anno.GetName());
        }
        for (const auto &anno : classField.GetAnnotations()) {
            classFieldGotAnnotationNames.emplace(anno.GetName());
        }

        auto result2 = helpers::GetInterfaceByName(module, "Interface1");
        const auto &iface = result2.value();
        const auto &interfaceFields = iface.GetFields();
        const auto &interfaceField = interfaceFields[0];
        for (const auto &anno : iface.GetAnnotations()) {
            interfaceGotAnnotationNames.emplace(anno.GetName());
        }
        for (const auto &anno : interfaceField.GetAnnotations()) {
            interfaceFieldGotAnnotationNames.emplace(anno.GetName());
        }
    }
    file.WriteAbc(OUTPUT_PATH);

    ASSERT_EQ(classGotAnnotationNames, expectAnnotationNames);
    ASSERT_EQ(classFieldGotAnnotationNames, expectAnnotationNames);
    ASSERT_EQ(interfaceGotAnnotationNames, expectAnnotationNames);
    ASSERT_EQ(interfaceFieldGotAnnotationNames, expectAnnotationNames);
    ASSERT_EQ(helpers::ExecuteStaticAbc(INPUT_PATH, "annotations_static_modify", "main"),
              helpers::ExecuteStaticAbc(OUTPUT_PATH, "annotations_static_modify", "main"));
}
}  // namespace libabckit::test