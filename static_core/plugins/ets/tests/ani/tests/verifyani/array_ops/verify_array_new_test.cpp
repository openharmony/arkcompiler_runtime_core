/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "verify_ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
namespace ark::ets::ani::verify::testing {

class ArrayNewTest : public VerifyAniTest {
protected:
    ani_size length_ = 3U;
};

TEST_F(ArrayNewTest, wrong_env)
{
    ani_ref undefinedRef {};
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_array res {};
    ASSERT_EQ(env_->c_api->Array_New(nullptr, length_, undefinedRef, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref"},
        {"result", "ani_array *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_New", testLines);
}

TEST_F(ArrayNewTest, wrong_ref)
{
    ani_array res {};
    ASSERT_EQ(env_->Array_New(length_, nullptr, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref", "wrong reference"},
        {"result", "ani_array *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_New", testLines);
}

TEST_F(ArrayNewTest, wrong_result)
{
    ani_ref undefinedRef {};
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->Array_New(length_, undefinedRef, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref"},
        {"result", "ani_array *", "wrong pointer for storing 'ani_array'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_New", testLines);
}

TEST_F(ArrayNewTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Array_New(nullptr, length_, nullptr, nullptr), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref", "wrong reference"},
        {"result", "ani_array *", "wrong pointer for storing 'ani_array'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_New", testLines);
}

TEST_F(ArrayNewTest, throw_error)
{
    ani_ref undefinedRef {};
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);

    ThrowError();

    ani_array res {};
    ASSERT_EQ(env_->Array_New(length_, undefinedRef, &res), ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"length", "ani_size"},
        {"initial_element", "ani_ref"},
        {"result", "ani_array *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("Array_New", testLines);
}

TEST_F(ArrayNewTest, zero_length)
{
    ani_ref undefinedRef {};
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_array res {};
    ASSERT_EQ(env_->Array_New(0U, undefinedRef, &res), ANI_OK);
    ASSERT_NE(res, nullptr);

    ani_size length;
    ASSERT_EQ(env_->Array_GetLength(res, &length), ANI_OK);
    ASSERT_EQ(length, 0U);
}

TEST_F(ArrayNewTest, null_initial_element)
{
    ani_ref nullRef {};
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ani_array res {};
    ASSERT_EQ(env_->Array_New(length_, nullRef, &res), ANI_OK);
    ASSERT_NE(res, nullptr);
}

TEST_F(ArrayNewTest, string_initial_element)
{
    ani_string str;
    std::string testStr = "test";
    ASSERT_EQ(env_->String_NewUTF8(testStr.c_str(), testStr.size(), &str), ANI_OK);
    ani_array res {};
    ASSERT_EQ(env_->Array_New(length_, str, &res), ANI_OK);
    ASSERT_NE(res, nullptr);
}

TEST_F(ArrayNewTest, object_initial_element)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);
    ani_method ctor {};
    ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
    ani_object obj {};
    ASSERT_EQ(env_->Object_New(cls, ctor, &obj), ANI_OK);

    ani_array res {};
    ASSERT_EQ(env_->Array_New(length_, obj, &res), ANI_OK);
    ASSERT_NE(res, nullptr);
}

TEST_F(ArrayNewTest, success)
{
    ani_ref undefinedRef {};
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ani_array res {};
    ASSERT_EQ(env_->Array_New(length_, undefinedRef, &res), ANI_OK);
    ASSERT_NE(res, nullptr);
}

}  // namespace ark::ets::ani::verify::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)