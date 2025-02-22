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
#include <cstring>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class VariableSetValueRefTest : public AniTest {};

TEST_F(VariableSetValueRefTest, set_int_value_normal)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "stringValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    auto ref = CallEtsFunction<ani_ref>("getStringObject");
    const uint32_t bufferSize = 10U;
    char utfBuffer[bufferSize] = {};

    auto str = reinterpret_cast<ani_string>(ref);
    ani_size strLen = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(str, &strLen), ANI_OK);
    ASSERT_EQ(strLen, 4U);

    ani_size substrOffset = 0U;
    ani_size substrSize = strLen;
    strLen = 0U;
    auto status = env_->String_GetUTF8SubString(str, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &strLen);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "test");

    ani_string string = {};
    status = env_->String_NewUTF8("abcdefg", 7U, &string);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, string), ANI_OK);

    ref = CallEtsFunction<ani_ref>("getStringObject");
    str = reinterpret_cast<ani_string>(ref);
    strLen = 0U;
    ASSERT_EQ(env_->String_GetUTF8Size(str, &strLen), ANI_OK);
    ASSERT_EQ(strLen, 7U);

    substrSize = strLen;
    strLen = 0U;
    status = env_->String_GetUTF8SubString(str, substrOffset, substrSize, utfBuffer, sizeof(utfBuffer), &strLen);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_STREQ(utfBuffer, "abcdefg");
}

TEST_F(VariableSetValueRefTest, set_Ref_value_invalid_args_variable_1)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "shortValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ani_string value = {};
    auto status = env_->String_NewUTF8("abcdefg", 7U, &value);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, value), ANI_INVALID_TYPE);
}

TEST_F(VariableSetValueRefTest, set_Ref_value_invalid_args_variable_2)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_string value = {};
    auto status = env_->String_NewUTF8("abcdefg", 7U, &value);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_EQ(env_->Variable_SetValue_Ref(nullptr, value), ANI_INVALID_ARGS);
}

TEST_F(VariableSetValueRefTest, set_Ref_value_invalid_args_variable_3)
{
    ani_namespace ns {};
    ASSERT_EQ(env_->FindNamespace("Lanyns;", &ns), ANI_OK);
    ASSERT_NE(ns, nullptr);

    ani_variable variable {};
    ASSERT_EQ(env_->Namespace_FindVariable(ns, "stringValue", &variable), ANI_OK);
    ASSERT_NE(variable, nullptr);

    ASSERT_EQ(env_->Variable_SetValue_Ref(variable, nullptr), ANI_INVALID_ARGS);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)