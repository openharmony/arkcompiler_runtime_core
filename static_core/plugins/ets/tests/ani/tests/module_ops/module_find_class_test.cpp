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

#include "ani_gtest.h"

namespace ark::ets::ani::testing {

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
class ModuleFindClassTest : public AniTest {};

TEST_F(ModuleFindClassTest, find_class)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "ATest", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ASSERT_EQ(env_->Module_FindClass(module, "BTest", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ASSERT_EQ(env_->Module_FindClass(module, "ops.C", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
}

TEST_F(ModuleFindClassTest, find_interface)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "AA", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(kclass, "foo", "d:d", &method), ANI_OK);
    ASSERT_NE(method, nullptr);
    ASSERT_EQ(env_->Module_FindClass(module, "BB", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ASSERT_EQ(env_->Class_FindMethod(kclass, "eat", "i:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ModuleFindClassTest, find_abstract_class)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "Person", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(kclass, "addMethod", "ii:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ModuleFindClassTest, find_final_class)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "Child", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(kclass, "addMethod", "ii:i", &method), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ModuleFindClassTest, invalid_arg_class)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "ATest;", &kclass), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindClass(module, nullptr, &kclass), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindClass(module, "", &kclass), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Module_FindClass(module, "LATest", &kclass), ANI_NOT_FOUND);
}

TEST_F(ModuleFindClassTest, invalid_arg_result)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ASSERT_EQ(env_->Module_FindClass(module, "ATest", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ModuleFindClassTest, invalid_arg_descriptor)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ani_class kclass1 {};
    ASSERT_EQ(env_->Module_FindClass(module, "ATest", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    kclass1 = kclass;
    ASSERT_EQ(env_->Module_FindClass(module, "AAAAAATest", &kclass), ANI_NOT_FOUND);
    ASSERT_EQ(kclass, kclass1);
}

TEST_F(ModuleFindClassTest, invalid_env)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_class kclass {};
    ASSERT_EQ(env_->c_api->Module_FindClass(nullptr, module, "ATest", &kclass), ANI_INVALID_ARGS);
    ASSERT_EQ(kclass, nullptr);
}

TEST_F(ModuleFindClassTest, invalid_module)
{
    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(nullptr, "ATest", &kclass), ANI_INVALID_ARGS);
    ASSERT_EQ(kclass, nullptr);
}

TEST_F(ModuleFindClassTest, many_descriptor)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);
    ani_class kclass {};
    char end = 'J';
    const int32_t loopCount = 3;
    for (int32_t i = 0; i < loopCount; i++) {
        const std::string str = std::string(1, (random() % (end - 'A') + 'A'));
        ASSERT_EQ(env_->Module_FindClass(module, str.c_str(), &kclass), ANI_OK);
    }
}

TEST_F(ModuleFindClassTest, allType)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "Person", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    kclass = nullptr;
    ASSERT_EQ(env_->Module_FindClass(module, "Child", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    kclass = nullptr;
    ASSERT_EQ(env_->Module_FindClass(module, "Student", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);

    ani_static_field field {};
    ASSERT_EQ(env_->Class_FindStaticField(kclass, "c", &field), ANI_OK);
    ASSERT_NE(field, nullptr);
    ani_int x = 1;
    ani_int y = 0;
    ASSERT_EQ(env_->Class_SetStaticField_Int(kclass, field, x), ANI_OK);
    ASSERT_EQ(env_->Class_GetStaticField_Int(kclass, field, &y), ANI_OK);
    ASSERT_EQ(y, 1);
}

TEST_F(ModuleFindClassTest, find_B_in_namespace_A)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "BBBTest", &kclass), ANI_NOT_FOUND);
    ASSERT_EQ(kclass, nullptr);

    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "aaa_test", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    kclass = nullptr;
    ASSERT_EQ(env_->Namespace_FindClass(ns, "BBBTest", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
}

TEST_F(ModuleFindClassTest, find_B_extends_A)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "Child", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    kclass = nullptr;
    ASSERT_EQ(env_->Module_FindClass(module, "Person", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
}

TEST_F(ModuleFindClassTest, find_C_extends_B_extends_A)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "CT", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    kclass = nullptr;
    ASSERT_EQ(env_->Module_FindClass(module, "BT", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    kclass = nullptr;
    ASSERT_EQ(env_->Module_FindClass(module, "AT", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
}

TEST_F(ModuleFindClassTest, find_generic_class)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_class kclass {};
    ASSERT_EQ(env_->Module_FindClass(module, "Container", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ani_method method {};
    // add(T), generic method
    ASSERT_EQ(env_->Class_FindMethod(kclass, "add", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);
    // create ContainerTest object
    ASSERT_EQ(env_->Module_FindClass(module, "ContainerTest", &kclass), ANI_OK);
    ASSERT_NE(kclass, nullptr);
    ani_method cMethod {};
    ASSERT_EQ(env_->Class_FindMethod(kclass, "<ctor>", "i:", &cMethod), ANI_OK);
    ASSERT_NE(cMethod, nullptr);

    ani_object object {};
    const int32_t defaultSize = 100;
    ASSERT_EQ(env_->Object_New(kclass, cMethod, &object, defaultSize), ANI_OK);
    ASSERT_NE(object, nullptr);

    // create Int object
    ani_object testIntObject = {};
    ani_class intClass {};
    ASSERT_EQ(env_->FindClass("std.core.Int", &intClass), ANI_OK);
    ASSERT_NE(intClass, nullptr);
    ani_method intCMethod {};
    ASSERT_EQ(env_->Class_FindMethod(intClass, "<ctor>", "i:", &intCMethod), ANI_OK);
    ASSERT_NE(intCMethod, nullptr);
    const int32_t testIntValue = 1;
    ASSERT_EQ(env_->Object_New(intClass, intCMethod, &testIntObject, testIntValue), ANI_OK);
    ASSERT_NE(testIntObject, nullptr);

    ASSERT_EQ(env_->Object_CallMethod_Void(object, method, testIntObject), ANI_OK);
    ASSERT_NE(method, nullptr);
}

TEST_F(ModuleFindClassTest, check_initialization)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("@abcModule.module_find_class_test", &module), ANI_OK);

    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_class_test", false));
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_class_test.ATest"));
    ani_class cls {};
    ASSERT_EQ(env_->Module_FindClass(module, "ATest", &cls), ANI_OK);
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_class_test.ATest"));
    ASSERT_FALSE(IsRuntimeClassInitialized("@abcModule.module_find_class_test", false));
}

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
}  // namespace ark::ets::ani::testing
