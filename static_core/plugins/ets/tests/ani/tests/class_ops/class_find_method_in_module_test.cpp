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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,  readability-magic-numbers)
namespace ark::ets::ani::testing {

class ClassFindMethodInModuleTest : public AniTest {
public:
    static constexpr ani_int VAL1 = 2;
    static constexpr ani_int VAL2 = 3;
};
TEST_F(ClassFindMethodInModuleTest, find_func_in_module)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("L@abcModule/class_find_method_in_module_test/nsa/A;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);
    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "func", "II:I", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":V", &ctor), ANI_OK);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &object), ANI_OK);
    ani_value args[2];  // NOLINT(modernize-avoid-c-arrays)
    args[0].i = VAL1;
    args[1].i = VAL2;

    ani_int res = 0;
    // Call the method and verify the return value.
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &res, args), ANI_OK);
    ASSERT_EQ(res, VAL1 + VAL2);
}

TEST_F(ClassFindMethodInModuleTest, find_method_combine_scenes_001)
{
    ani_module module {};
    ASSERT_EQ(env_->FindModule("L@abcModule/class_find_method_in_module_test;", &module), ANI_OK);
    ASSERT_NE(module, nullptr);

    ani_namespace ns {};
    ASSERT_EQ(env_->Module_FindNamespace(module, "Ltest001;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_class cls {};
    ASSERT_EQ(env_->Namespace_FindClass(ns, "LTestA001;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "sum", "II:I", &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(cls, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_value args[2U];
    const ani_int value1 = 2;
    const ani_int value2 = 3;
    args[0U].i = value1;
    args[1U].i = value2;

    ani_int sum = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &sum, args), ANI_OK);
    ASSERT_EQ(sum, value1 + value2);
}

}  // namespace ark::ets::ani::testing
   // NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays, readability-magic-numbers)