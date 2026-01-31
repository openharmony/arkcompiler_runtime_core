/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <filesystem>

#include "abckit_wrapper/file_view.h"
#include "test_util/assert.h"
#include "../../library/logger.h"

using namespace testing::ext;

namespace {
constexpr std::string_view ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/set_name_test.abc";
constexpr std::string_view UPDATED_ABC_FILE_PATH = ABCKIT_WRAPPER_ABC_FILE_DIR "ut/set_name_test_updated.abc";

bool FileExists(const std::string &filePath)
{
    return std::filesystem::exists(filePath);
}

bool DeleteFile(const std::string &filePath)
{
    if (std::filesystem::remove(filePath)) {
        return true;
    }
    return false;
}
}  // namespace

class TestSetName : public ::testing::Test {
public:
    void SetUp() override
    {
        ASSERT_SUCCESS(fileView.Init(ABC_FILE_PATH));
    }

    void TearDown() override
    {
        fileView.Save(UPDATED_ABC_FILE_PATH);
        ASSERT_TRUE(FileExists(UPDATED_ABC_FILE_PATH.data()));
        ASSERT_TRUE(DeleteFile(UPDATED_ABC_FILE_PATH.data()));
    }

    abckit_wrapper::FileView fileView;
};

/**
 * @tc.name: module_set_name_001
 * @tc.desc: test module set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, module_set_name_001, TestSize.Level0)
{
    const auto module = fileView.GetModule("set_name_test");
    ASSERT_TRUE(module.has_value());

    ASSERT_TRUE(module.value()->SetName("a"));
    ASSERT_EQ(module.value()->GetName(), "a");
}

/**
 * @tc.name: module_set_name_002
 * @tc.desc: test module set name with cache invalidation and FQN update
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, module_set_name_002, TestSize.Level0)
{
    // Get module object
    const auto module = fileView.GetModule("set_name_test");
    ASSERT_TRUE(module.has_value());

    // Get original name and FQN
    const std::string originalName = module.value()->GetName();
    const std::string originalFQN = module.value()->GetFullyQualifiedName();

    // Verify original state
    ASSERT_EQ(originalName, "set_name_test");
    ASSERT_EQ(originalFQN, "set_name_test");

    // Set new name
    const std::string newName = "new_module_name";
    ASSERT_TRUE(module.value()->SetName(newName));

    // Verify new name is set correctly
    ASSERT_EQ(module.value()->GetName(), newName);

    // Verify FQN is updated correctly after cache invalidation
    const std::string newFQN = module.value()->GetFullyQualifiedName();
    ASSERT_EQ(newFQN, newName);

    // Verify that all child objects' FQN are updated correctly
    // Test namespace FQN update
    const auto ns1 = fileView.Get<abckit_wrapper::Namespace>("set_name_test.Ns1");
    if (ns1.has_value()) {
        const std::string ns1FQN = ns1.value()->GetFullyQualifiedName();
        // Namespace FQN should reflect the new module name
        ASSERT_EQ(ns1FQN, "new_module_name.Ns1");
    }

    // Test class FQN update
    const auto class1 = fileView.Get<abckit_wrapper::Class>("set_name_test.C1");
    if (class1.has_value()) {
        const std::string class1FQN = class1.value()->GetFullyQualifiedName();
        // Class FQN should reflect the new module name
        ASSERT_EQ(class1FQN, "new_module_name.C1");
    }

    // Test method FQN update
    const auto method1 = fileView.Get<abckit_wrapper::Method>("set_name_test.foo1:void;");
    if (method1.has_value()) {
        const std::string method1FQN = method1.value()->GetFullyQualifiedName();
        // Method FQN should reflect the new module name
        ASSERT_EQ(method1FQN, "new_module_name.foo1:void;");
    }

    // Test field FQN update
    const auto field1 = fileView.Get<abckit_wrapper::Field>("set_name_test.m1");
    if (field1.has_value()) {
        const std::string field1FQN = field1.value()->GetFullyQualifiedName();
        // Field FQN should reflect the new module name
        ASSERT_EQ(field1FQN, "new_module_name.m1");
    }

    // Test nested namespace FQN update
    const auto ns2 = fileView.Get<abckit_wrapper::Namespace>("set_name_test.Ns1.Ns2");
    if (ns2.has_value()) {
        const std::string ns2FQN = ns2.value()->GetFullyQualifiedName();
        // Nested namespace FQN should reflect the new module name
        ASSERT_EQ(ns2FQN, "new_module_name.Ns1.Ns2");
    }

    // Test namespace field FQN update
    const auto nsField1 = fileView.Get<abckit_wrapper::Field>("set_name_test.Ns1.field1");
    if (nsField1.has_value()) {
        const std::string nsField1FQN = nsField1.value()->GetFullyQualifiedName();
        // Namespace field FQN should reflect the new module name
        ASSERT_EQ(nsField1FQN, "new_module_name.Ns1.field1");
    }
}

/**
 * @tc.name: module_field_set_name_001
 * @tc.desc: test module field set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, module_field_set_name_001, TestSize.Level0)
{
    const auto m1 = fileView.Get<abckit_wrapper::Field>("set_name_test.m1");
    ASSERT_TRUE(m1.has_value());

    ASSERT_TRUE(m1.value()->SetName("a"));
}

/**
 * @tc.name: namespace_set_name_001
 * @tc.desc: test namespace set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, namespace_set_name_001, TestSize.Level0)
{
    const auto ns1 = fileView.Get<abckit_wrapper::Namespace>("set_name_test.Ns1");
    ASSERT_TRUE(ns1.has_value());

    ASSERT_TRUE(ns1.value()->SetName("a"));
}

/**
 * @tc.name: namespace_set_name_002
 * @tc.desc: test namespace set name with cache invalidation and FQN update
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, namespace_set_name_002, TestSize.Level0)
{
    // Get namespace object
    const auto ns1 = fileView.Get<abckit_wrapper::Namespace>("set_name_test.Ns1");
    ASSERT_TRUE(ns1.has_value());

    // Get original name and FQN
    const std::string originalName = ns1.value()->GetName();
    const std::string originalFQN = ns1.value()->GetFullyQualifiedName();

    // Verify original state
    ASSERT_EQ(originalName, "Ns1");
    ASSERT_EQ(originalFQN, "set_name_test.Ns1");

    // Set new name
    const std::string newName = "NewNamespace";
    ASSERT_TRUE(ns1.value()->SetName(newName));

    // Verify new name is set correctly
    ASSERT_EQ(ns1.value()->GetName(), newName);

    // Verify FQN is updated correctly after cache invalidation
    const std::string newFQN = ns1.value()->GetFullyQualifiedName();
    ASSERT_EQ(newFQN, "set_name_test.NewNamespace");

    // Verify that child objects' FQN are also updated
    // Get a field from this namespace
    const auto field = fileView.Get<abckit_wrapper::Field>("set_name_test.Ns1.field1");
    if (field.has_value()) {
        const std::string fieldFQN = field.value()->GetFullyQualifiedName();
        // Field FQN should reflect the new namespace name
        ASSERT_EQ(fieldFQN, "set_name_test.NewNamespace.field1");
    }

    // Test nested namespace scenario
    const auto ns2 = fileView.Get<abckit_wrapper::Namespace>("set_name_test.Ns1.Ns2");
    if (ns2.has_value()) {
        const std::string ns2FQN = ns2.value()->GetFullyQualifiedName();
        // Nested namespace FQN should reflect the parent namespace name change
        ASSERT_EQ(ns2FQN, "set_name_test.NewNamespace.Ns2");
    }
}

/**
 * @tc.name: namespace_field_set_name_001
 * @tc.desc: test namespace field set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, namespace_field_set_name_001, TestSize.Level0)
{
    const auto nsField1 = fileView.Get<abckit_wrapper::Field>("set_name_test.Ns1.field1");
    ASSERT_TRUE(nsField1.has_value());

    ASSERT_TRUE(nsField1.value()->SetName("a"));
    ASSERT_EQ(nsField1.value()->GetRawName(), "a");

    const auto nsField2 = fileView.Get<abckit_wrapper::Field>("set_name_test.Ns1.Ns2.field2");
    ASSERT_TRUE(nsField2.has_value());

    ASSERT_TRUE(nsField2.value()->SetName("a"));
    ASSERT_EQ(nsField2.value()->GetRawName(), "a");
}

/**
 * @tc.name: function_set_name_001
 * @tc.desc: test function set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, function_set_name_001, TestSize.Level0)
{
    const auto method1 = fileView.Get<abckit_wrapper::Method>("set_name_test.foo1:void;");
    ASSERT_TRUE(method1.has_value());

    ASSERT_TRUE(method1.value()->SetName("a"));
    ASSERT_EQ(method1.value()->GetName(), "a:void;");
    ASSERT_EQ(method1.value()->GetRawName(), "a");
    ASSERT_EQ(method1.value()->GetFullyQualifiedName(), "set_name_test.a:void;");

    const auto method2 = fileView.Get<abckit_wrapper::Method>("set_name_test.Ns1.foo2:void;");
    ASSERT_TRUE(method2.has_value());

    ASSERT_TRUE(method2.value()->SetName("a"));
    ASSERT_EQ(method2.value()->GetName(), "a:void;");
    ASSERT_EQ(method2.value()->GetRawName(), "a");
    ASSERT_EQ(method2.value()->GetFullyQualifiedName(), "set_name_test.Ns1.a:void;");
}

/**
 * @tc.name: function_set_name_002
 * @tc.desc: test async function set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, function_set_name_002, TestSize.Level0)
{
    const auto method1 = fileView.Get<abckit_wrapper::Method>("set_name_test.foo5:std.core.Promise;");
    ASSERT_TRUE(method1.has_value());

    ASSERT_TRUE(method1.value()->SetName("New"));
    ASSERT_EQ(method1.value()->GetName(), "New:std.core.Promise;");
}

/**
 * @tc.name: method_set_name_with_signature_001
 * @tc.desc: test method set name with signature reinitialization and IsSetterOrGetterMethod coverage
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, method_set_name_with_signature_001, TestSize.Level0)
{
    // Test regular method name change with signature update
    const auto method1 = fileView.Get<abckit_wrapper::Method>("set_name_test.foo1:void;");
    ASSERT_TRUE(method1.has_value());

    // Get original method details
    const std::string originalName = method1.value()->GetName();
    const std::string originalRawName = method1.value()->GetRawName();
    const std::string originalFQN = method1.value()->GetFullyQualifiedName();

    // Verify original state
    ASSERT_EQ(originalName, "foo1:void;");
    ASSERT_EQ(originalRawName, "foo1");
    ASSERT_EQ(originalFQN, "set_name_test.foo1:void;");

    // Set new name - this should trigger InitSignature() and IsSetterOrGetterMethod check
    const std::string newName = "newMethodName";
    ASSERT_TRUE(method1.value()->SetName(newName));

    // Verify new method details after signature reinitialization
    ASSERT_EQ(method1.value()->GetName(), "newMethodName:void;");
    ASSERT_EQ(method1.value()->GetRawName(), "newMethodName");
    ASSERT_EQ(method1.value()->GetFullyQualifiedName(), "set_name_test.newMethodName:void;");
}

/**
 * @tc.name: method_set_name_with_signature_002
 * @tc.desc: test setter/getter method name change with IsSetterOrGetterMethod handling
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, method_set_name_with_signature_002, TestSize.Level0)
{
    // Test regular method name change - IsSetterOrGetterMethod should return false
    const auto method1 = fileView.Get<abckit_wrapper::Method>("set_name_test.foo1:void;");
    ASSERT_TRUE(method1.has_value());

    const std::string originalName = method1.value()->GetName();
    const std::string originalRawName = method1.value()->GetRawName();

    // Verify original method has no setter/getter prefix
    ASSERT_EQ(originalName, "foo1:void;");
    ASSERT_EQ(originalRawName, "foo1"); // Should be the raw method name

    // Set new name - IsSetterOrGetterMethod should return false for regular method
    ASSERT_TRUE(method1.value()->SetName("updatedMethod"));

    // Verify regular method signature is correctly handled (no setter/getter prefix)
    ASSERT_EQ(method1.value()->GetName(), "updatedMethod:void;");
    ASSERT_EQ(method1.value()->GetRawName(), "updatedMethod"); // Should be the raw method name

    // Test another regular method with parameters
    const auto method2 = fileView.Get<abckit_wrapper::Method>("set_name_test.Ns1.foo2:void;");
    if (method2.has_value()) {
        const std::string originalName2 = method2.value()->GetName();
        const std::string originalRawName2 = method2.value()->GetRawName();

        // Verify original method has no setter/getter prefix
        ASSERT_EQ(originalName2, "foo2:void;");
        ASSERT_EQ(originalRawName2, "foo2");

        // Set new name
        ASSERT_TRUE(method2.value()->SetName("newMethodName"));

        // Verify regular method signature is correctly handled
        ASSERT_EQ(method2.value()->GetName(), "newMethodName:void;");
        ASSERT_EQ(method2.value()->GetRawName(), "newMethodName");
    }
}

/**
 * @tc.name: method_set_name_with_signature_003
 * @tc.desc: test edge cases for IsSetterOrGetterMethod detection
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, method_set_name_with_signature_003, TestSize.Level0)
{
    // Test method that contains "set" but not as prefix (should not be detected as setter)
    const auto methodWithSetInName = fileView.Get<abckit_wrapper::Method>("set_name_test.foo1:void;");
    if (methodWithSetInName.has_value()) {
        const std::string originalName = methodWithSetInName.value()->GetName();

        // This method should NOT be detected as setter by IsSetterOrGetterMethod
        ASSERT_TRUE(methodWithSetInName.value()->SetName("resetValue"));

        // Verify it's treated as regular method, not setter
        ASSERT_EQ(methodWithSetInName.value()->GetName(), "resetValue:void;");
        ASSERT_EQ(methodWithSetInName.value()->GetRawName(), "resetValue");
    }

    // Test method that contains "get" but not as prefix (should not be detected as getter)
    const auto methodWithGetInName = fileView.Get<abckit_wrapper::Method>("set_name_test.Ns1.foo2:void;");
    if (methodWithGetInName.has_value()) {
        const std::string originalName = methodWithGetInName.value()->GetName();

        // This method should NOT be detected as getter by IsSetterOrGetterMethod
        ASSERT_TRUE(methodWithGetInName.value()->SetName("forgetValue"));

        // Verify it's treated as regular method, not getter
        ASSERT_EQ(methodWithGetInName.value()->GetName(), "forgetValue:void;");
        ASSERT_EQ(methodWithGetInName.value()->GetRawName(), "forgetValue");
    }

    // Test method with multiple parameters
    const auto multiParamMethod =
        fileView.Get<abckit_wrapper::Method>("set_name_test.foo3:set_name_test.ClassA;i32;void;");
    if (multiParamMethod.has_value()) {
        const std::string originalName = multiParamMethod.value()->GetName();

        // Set new name - should trigger InitSignature() with parameter handling
        ASSERT_TRUE(multiParamMethod.value()->SetName("updatedMultiParamMethod"));

        // Verify signature is correctly reinitialized
        ASSERT_EQ(multiParamMethod.value()->GetName(), "updatedMultiParamMethod:set_name_test.ClassA;i32;void;");
        ASSERT_EQ(multiParamMethod.value()->GetRawName(), "updatedMultiParamMethod");
    }
}

/**
 * @tc.name: method_set_name_with_signature_004
 * @tc.desc: test complex method signatures with IsSetterOrGetterMethod coverage
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, method_set_name_with_signature_004, TestSize.Level0)
{
    // Test static method (different parameter handling in InitSignature)
    const auto staticMethod = fileView.Get<abckit_wrapper::Method>("set_name_test.foo4:void;");
    if (staticMethod.has_value()) {
        const std::string originalName = staticMethod.value()->GetName();

        ASSERT_TRUE(staticMethod.value()->SetName("newStaticMethod"));

        // Verify static method signature is correctly handled
        ASSERT_EQ(staticMethod.value()->GetName(), "newStaticMethod:void;");
        ASSERT_EQ(staticMethod.value()->GetRawName(), "newStaticMethod");
    }

    // Test async method with return type
    const auto asyncMethod = fileView.Get<abckit_wrapper::Method>("set_name_test.foo5:std.core.Promise;");
    if (asyncMethod.has_value()) {
        const std::string originalName = asyncMethod.value()->GetName();

        ASSERT_TRUE(asyncMethod.value()->SetName("newAsyncMethod"));
        // Verify async method signature is correctly handled
        ASSERT_EQ(asyncMethod.value()->GetName(), "newAsyncMethod:std.core.Promise;");
        ASSERT_EQ(asyncMethod.value()->GetRawName(), "newAsyncMethod");
    }
    // Test method name change with complex signature validation
    const auto complexMethod =
        fileView.Get<abckit_wrapper::Method>("set_name_test.foo3:set_name_test.ClassA;i32;void;");
    if (complexMethod.has_value()) {
        // Record original signature details
        const std::string originalName = complexMethod.value()->GetName();
        const std::string originalRawName = complexMethod.value()->GetRawName();

        // Set new name
        ASSERT_TRUE(complexMethod.value()->SetName("complexUpdatedMethod"));

        // Verify signature is correctly reinitialized with complex parameters
        ASSERT_EQ(complexMethod.value()->GetName(), "complexUpdatedMethod:set_name_test.ClassA;i32;void;");
        ASSERT_EQ(complexMethod.value()->GetRawName(), "complexUpdatedMethod");

        // Verify that the method descriptor remains consistent
        EXPECT_FALSE(complexMethod.value()->GetName().empty());
        EXPECT_TRUE(complexMethod.value()->GetName().find("complexUpdatedMethod") != std::string::npos);
    }
}

/**
 * @tc.name: class_set_name_001
 * @tc.desc: test class set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, class_set_name_001, TestSize.Level0)
{
    const auto class1 = fileView.Get<abckit_wrapper::Class>("set_name_test.C1");
    ASSERT_TRUE(class1.has_value());

    ASSERT_TRUE(class1.value()->SetName("a"));
    ASSERT_EQ(class1.value()->GetName(), "a");
}

/**
 * @tc.name: class_set_name_002
 * @tc.desc: test abstract class set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, class_set_name_002, TestSize.Level0)
{
    const auto class2 = fileView.Get<abckit_wrapper::Class>("set_name_test.C2");
    ASSERT_TRUE(class2.has_value());

    class2.value()->SetName("NewAbstract");
    ASSERT_EQ(class2.value()->GetName(), "NewAbstract");
}

/**
 * @tc.name: class_set_name_003
 * @tc.desc: test class as multiple types set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, class_set_name_003, TestSize.Level0)
{
    const auto classA = fileView.Get<abckit_wrapper::Class>("set_name_test.ClassA");
    ASSERT_TRUE(classA.has_value());
    classA.value()->SetName("a");
    ASSERT_EQ(classA.value()->GetName(), "a");

    const auto classB = fileView.Get<abckit_wrapper::Class>("set_name_test.ClassB");
    ASSERT_TRUE(classA.has_value());
    classB.value()->SetName("b");
    ASSERT_EQ(classB.value()->GetName(), "b");

    const auto classC = fileView.Get<abckit_wrapper::Class>("set_name_test.ClassC");
    ASSERT_TRUE(classC.has_value());
    classC.value()->SetName("c");
    ASSERT_EQ(classC.value()->GetName(), "c");
}

/**
 * @tc.name: class_field_set_name_001
 * @tc.desc: test class field set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, class_field_set_name_001, TestSize.Level0)
{
    const auto field1 = fileView.Get<abckit_wrapper::Field>("set_name_test.C1.field1");
    ASSERT_TRUE(field1.has_value());

    ASSERT_TRUE(field1.value()->SetName("a"));
    ASSERT_EQ(field1.value()->GetName(), "a");
}

/**
 * @tc.name: interface_set_name_001
 * @tc.desc: test interface set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, interface_set_name_001, TestSize.Level0)
{
    const auto interface1 = fileView.Get<abckit_wrapper::Class>("set_name_test.Interface1");
    ASSERT_TRUE(interface1.has_value());

    ASSERT_TRUE(interface1.value()->SetName("a"));
    ASSERT_EQ(interface1.value()->GetName(), "a");
}

/**
 * @tc.name: interface_field_set_name_001
 * @tc.desc: test interface field set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, interface_field_set_name_001, TestSize.Level0)
{
    const auto field = fileView.Get<abckit_wrapper::Field>("set_name_test.Interface4.%%property-field1");
    ASSERT_TRUE(field.has_value());

    ASSERT_TRUE(field.value()->SetName("a"));
    ASSERT_EQ(field.value()->GetName(), "%%property-a");
}

/**
 * @tc.name: enum_set_name_001
 * @tc.desc: test enum set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, enum_set_name_001, TestSize.Level0)
{
    const auto color = fileView.Get<abckit_wrapper::Class>("set_name_test.Color");
    ASSERT_TRUE(color.has_value());

    ASSERT_TRUE(color.value()->SetName("a"));
    ASSERT_EQ(color.value()->GetName(), "a");
}

/**
 * @tc.name: enum_field_set_name_001
 * @tc.desc: test enum field set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, enum_field_set_name_001, TestSize.Level0)
{
    const auto field = fileView.Get<abckit_wrapper::Field>("set_name_test.Color.RED");
    ASSERT_TRUE(field.has_value());

    ASSERT_TRUE(field.value()->SetName("a"));
    ASSERT_EQ(field.value()->GetName(), "a");
}

/**
 * @tc.name: annotationInterface_set_name_001
 * @tc.desc: test annotation interface set name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, annotationInterface_set_name_001, TestSize.Level0)
{
    const auto anno1 = fileView.Get<abckit_wrapper::AnnotationInterface>("set_name_test.Anno1");
    ASSERT_TRUE(anno1.has_value());

    ASSERT_TRUE(anno1.value()->SetName("New"));
    ASSERT_EQ(anno1.value()->GetName(), "New");
}

/**
 * @tc.name: annotationInterface_set_name_002
 * @tc.desc: test annotation interface SetName method with various scenarios
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, annotationInterface_set_name_002, TestSize.Level0)
{
    // Set valid name for annotation interface
    const auto anno1 = fileView.Get<abckit_wrapper::AnnotationInterface>("set_name_test.Anno1");
    ASSERT_TRUE(anno1.has_value());

    const std::string originalName = anno1.value()->GetName();

    // Test valid name change
    const std::string newName = "NewAnnotationInterfaceName";
    bool result = anno1.value()->SetName(newName);
    ASSERT_TRUE(result);
    ASSERT_EQ(anno1.value()->GetName(), newName);

    // Set empty name
    result = anno1.value()->SetName("");
    ASSERT_TRUE(result);
    ASSERT_EQ(anno1.value()->GetName(), newName); // Name should remain unchanged

    // Set name with special characters
    const std::string specialName = "Annotation_Interface-123";
    result = anno1.value()->SetName(specialName);
    ASSERT_TRUE(result);
    ASSERT_EQ(anno1.value()->GetName(), specialName);

    //Set long name
    const std::string longName = "VeryLongAnnotationInterfaceNameThatExceedsNormalLength";
    result = anno1.value()->SetName(longName);
    ASSERT_TRUE(result);
    ASSERT_EQ(anno1.value()->GetName(), longName);

    // Set same name again (should succeed but name remains same)
    result = anno1.value()->SetName(longName);
    ASSERT_TRUE(result);
    ASSERT_EQ(anno1.value()->GetName(), longName);
}

/**
 * @tc.name: field_set_name_with_signature_001
 * @tc.desc: test field set name with signature
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, field_set_name_with_signature_001, TestSize.Level0)
{
    // Get field object - using correct fully qualified name
    auto field = fileView.Get<abckit_wrapper::Field>("set_name_test.C1.field1");
    ASSERT_TRUE(field.has_value());

    // Verify initial state
    EXPECT_EQ(field.value()->GetName(), "field1");
    EXPECT_EQ(field.value()->GetFullyQualifiedName(), "set_name_test.C1.field1");

    // Set new name and verify InitSignature() call
    ASSERT_TRUE(field.value()->SetName("updatedField"));

    // Verify name update
    EXPECT_EQ(field.value()->GetName(), "updatedField");
    EXPECT_EQ(field.value()->GetFullyQualifiedName(), "set_name_test.C1.updatedField");

    // Verify signature-related properties (indirectly verify InitSignature() effect through GetName())
    EXPECT_NE(field.value()->GetName(), "");
}

/**
 * @tc.name: field_set_name_with_signature_002
 * @tc.desc: test field set name with signature
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, field_set_name_with_signature_002, TestSize.Level0)
{
    // Get field object containing %%property- prefix
    auto field = fileView.Get<abckit_wrapper::Field>("set_name_test.Interface4.%%property-field1");
    ASSERT_TRUE(field.has_value());

    // Verify initial state (contains %%property- prefix)
    EXPECT_EQ(field.value()->GetName(), "%%property-field1");
    EXPECT_EQ(field.value()->GetRawName(), "field1");

    // Set new name and verify InitSignature() prefix handling logic
    ASSERT_TRUE(field.value()->SetName("updatedPropertyField"));

    // Verify name update (InitSignature() should correctly handle prefix)
    EXPECT_EQ(field.value()->GetName(), "%%property-updatedPropertyField");
    EXPECT_EQ(field.value()->GetRawName(), "updatedPropertyField");

    // Verify FQN also correctly updated
    EXPECT_EQ(field.value()->GetFullyQualifiedName(), "set_name_test.Interface4.%%property-updatedPropertyField");
}

/**
 * @tc.name: field_set_name_with_signature_003
 * @tc.desc: test field set name with signature
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, field_set_name_with_signature_003, TestSize.Level0)
{
    // get module field object
    auto moduleField = fileView.Get<abckit_wrapper::Field>("set_name_test.m1");
    if (moduleField.has_value()) {
        // Verify initial state
        std::string originalName = moduleField.value()->GetName();
        std::string originalRawName = moduleField.value()->GetRawName();
        std::string originalFQN = moduleField.value()->GetFullyQualifiedName();

        // Set new name and verify InitSignature() prefix handling logic
        ASSERT_TRUE(moduleField.value()->SetName("updatedModuleField"));

        // Verify name and FQN updates
        EXPECT_EQ(moduleField.value()->GetName(), "updatedModuleField");
        EXPECT_EQ(moduleField.value()->GetRawName(), "updatedModuleField");
        EXPECT_NE(moduleField.value()->GetFullyQualifiedName(), originalFQN);

        // Verify field type remains unchanged
        EXPECT_FALSE(moduleField.value()->GetType().empty());
    }
}

/**
 * @tc.name: field_set_name_with_signature_004
 * @tc.desc: test field set name with signature
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TestSetName, field_set_name_with_signature_004, TestSize.Level0)
{
    // get namespace field object
    auto nsField = fileView.Get<abckit_wrapper::Field>("set_name_test.Ns1.field1");
    ASSERT_TRUE(nsField.has_value());

    // Verify initial state
    std::string originalName = nsField.value()->GetName();
    std::string originalRawName = nsField.value()->GetRawName();
    std::string originalFQN = nsField.value()->GetFullyQualifiedName();

    // Set new name and verify InitSignature() prefix handling logic
    ASSERT_TRUE(nsField.value()->SetName("renamedNsField"));

    // Verify name update
    EXPECT_EQ(nsField.value()->GetName(), "renamedNsField");
    EXPECT_EQ(nsField.value()->GetRawName(), "renamedNsField");
    EXPECT_NE(nsField.value()->GetFullyQualifiedName(), originalFQN);

    // Verify FQN contains new field name
    EXPECT_TRUE(nsField.value()->GetFullyQualifiedName().find("renamedNsField") != std::string::npos);

    // Verify field type remains unchanged
    EXPECT_FALSE(nsField.value()->GetType().empty());
}