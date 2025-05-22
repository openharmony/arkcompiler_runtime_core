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

#include "ani/ani.h"
#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class GetSuperClassTest : public AniTest {
public:
    template <bool HAS_SUPERCLASS>
    void CheckGetSuperClass(const char *clsName)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass(clsName, &cls), ANI_OK);
        ASSERT_NE(cls, nullptr);

        ani_type typeRef = cls;
        ASSERT_EQ(env_->Type_GetSuperClass(typeRef, &cls), ANI_OK);
        if constexpr (HAS_SUPERCLASS) {
            ASSERT_NE(cls, nullptr);
        } else {
            ASSERT_EQ(cls, nullptr);
        }
    }
};

TEST_F(GetSuperClassTest, get_super_class_A)
{
    CheckGetSuperClass<true>("Lget_super_class_test/A;");
}

TEST_F(GetSuperClassTest, get_super_class_B)
{
    CheckGetSuperClass<true>("Lget_super_class_test/B;");
}

TEST_F(GetSuperClassTest, try_get_object_superclass)
{
    CheckGetSuperClass<false>("Lstd/core/Object;");
}

TEST_F(GetSuperClassTest, ani_invalid_args)
{
    ani_class cls;
    ASSERT_EQ(env_->Type_GetSuperClass(nullptr, &cls), ANI_INVALID_ARGS);
}

TEST_F(GetSuperClassTest, get_super_class_combine_scenes_001)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("Lget_super_class_test/Student;", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_type typeRef = cls;
    ani_class clsResult {};
    ASSERT_EQ(env_->Type_GetSuperClass(typeRef, &clsResult), ANI_OK);
    ASSERT_NE(clsResult, nullptr);

    ani_method constructorMethod {};
    ASSERT_EQ(env_->Class_FindMethod(clsResult, "<ctor>", nullptr, &constructorMethod), ANI_OK);
    ASSERT_NE(constructorMethod, nullptr);

    ani_method method {};
    ASSERT_EQ(env_->Class_FindMethod(clsResult, "add_method", nullptr, &method), ANI_OK);
    ASSERT_NE(method, nullptr);

    ani_object object {};
    ASSERT_EQ(env_->Object_New(clsResult, constructorMethod, &object), ANI_OK);
    ASSERT_NE(object, nullptr);

    ani_value args[2U];
    ani_int arg1 = 2U;
    ani_int arg2 = 3U;
    args[0U].i = arg1;
    args[1U].i = arg2;
    ani_int result = 0;
    ASSERT_EQ(env_->Object_CallMethod_Int_A(object, method, &result, args), ANI_OK);
    ASSERT_EQ(result, arg1 + arg2);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
