/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

class ObjectSetFieldIntTest : public AniTest {
public:
    void GetTestData(ani_object *packResult, ani_field *fieldIntResult, ani_field *fieldStringResult)
    {
        auto packRef = CallEtsFunction<ani_ref>("object_set_field_int_test", "newPackObject");

        ani_class cls {};
        ASSERT_EQ(env_->FindClass("Lobject_set_field_int_test/Pack;", &cls), ANI_OK);

        ani_field fieldInt {};
        ASSERT_EQ(env_->Class_FindField(cls, "int_value", &fieldInt), ANI_OK);

        ani_field fieldString {};
        ASSERT_EQ(env_->Class_FindField(cls, "string_value", &fieldString), ANI_OK);

        *packResult = static_cast<ani_object>(packRef);
        *fieldIntResult = fieldInt;
        *fieldStringResult = fieldString;
    }
};

TEST_F(ObjectSetFieldIntTest, set_field_int)
{
    ani_object pack {};
    ani_field fieldInt {};
    ani_field fieldString {};
    const ani_int value1 = 2;
    const ani_int value2 = 3;
    GetTestData(&pack, &fieldInt, &fieldString);

    ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_int_test", "checkIntValue", pack, 0), ANI_TRUE);

    const int32_t loopCount = 3;
    for (int i = 1; i <= loopCount; i++) {
        ASSERT_EQ(env_->Object_SetField_Int(pack, fieldInt, value1), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_int_test", "checkIntValue", pack, value1), ANI_TRUE);

        ani_int result = 0;
        ASSERT_EQ(env_->Object_GetField_Int(pack, fieldInt, &result), ANI_OK);
        ASSERT_EQ(result, value1);

        ASSERT_EQ(env_->Object_SetField_Int(pack, fieldInt, value2), ANI_OK);
        ASSERT_EQ(CallEtsFunction<ani_boolean>("object_set_field_int_test", "checkIntValue", pack, value2), ANI_TRUE);

        ASSERT_EQ(env_->Object_GetField_Int(pack, fieldInt, &result), ANI_OK);
        ASSERT_EQ(result, value2);
    }
}

TEST_F(ObjectSetFieldIntTest, set_field_int_invalid_field_type)
{
    ani_object pack {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldInt, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Int(pack, fieldString, 2U), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldIntTest, set_field_int_invalid_args_object)
{
    ani_object pack {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldInt, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Int(nullptr, fieldInt, 2U), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldIntTest, set_field_int_invalid_args_field)
{
    ani_object pack {};
    ani_field fieldInt {};
    ani_field fieldString {};
    GetTestData(&pack, &fieldInt, &fieldString);

    ASSERT_EQ(env_->Object_SetField_Int(pack, nullptr, 2U), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
