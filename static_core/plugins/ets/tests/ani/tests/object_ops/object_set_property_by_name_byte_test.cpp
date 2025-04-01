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

class ObjectGetPropertyByNameByteTest : public AniTest {
public:
    ani_object NewCar()
    {
        auto carRef = CallEtsFunction<ani_ref>("object_set_property_by_name_byte_test", "newCarObject");
        return static_cast<ani_object>(carRef);
    }

    ani_object NewC1()
    {
        auto c1Ref = CallEtsFunction<ani_ref>("object_set_property_by_name_byte_test", "newC1");
        return static_cast<ani_object>(c1Ref);
    }

    ani_object NewC2()
    {
        auto c2Ref = CallEtsFunction<ani_ref>("object_set_property_by_name_byte_test", "newC2");
        return static_cast<ani_object>(c2Ref);
    }
};

TEST_F(ObjectGetPropertyByNameByteTest, set_field_property)
{
    ani_object car = NewCar();

    ani_byte highPerformance;
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "highPerformance", &highPerformance), ANI_OK);
    ASSERT_EQ(highPerformance, 0U);

    ASSERT_EQ(env_->Object_SetPropertyByName_Byte(car, "highPerformance", 1U), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "highPerformance", &highPerformance), ANI_OK);
    ASSERT_EQ(highPerformance, 1U);
}

TEST_F(ObjectGetPropertyByNameByteTest, set_setter_property)
{
    ani_object car = NewCar();

    ani_byte ecoFriendly;
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "ecoFriendly", &ecoFriendly), ANI_OK);
    ASSERT_EQ(ecoFriendly, 0U);

    ASSERT_EQ(env_->Object_SetPropertyByName_Byte(car, "ecoFriendly", 1U), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(car, "ecoFriendly", &ecoFriendly), ANI_OK);
    ASSERT_EQ(ecoFriendly, 1U);
}

TEST_F(ObjectGetPropertyByNameByteTest, invalid_argument)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_SetPropertyByName_Byte(nullptr, "highPerformance", 1U), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_SetPropertyByName_Byte(car, nullptr, 1U), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameByteTest, set_field_property_invalid_type)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_SetPropertyByName_Byte(car, "manufacturer", 1U), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetPropertyByNameByteTest, set_setter_property_invalid_type)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_SetPropertyByName_Byte(car, "model", 1U), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetPropertyByNameByteTest, set_interface_field)
{
    ani_object c1 = NewC1();

    ani_byte prop;
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(c1, "prop", &prop), ANI_OK);
    ASSERT_EQ(prop, 0U);

    ASSERT_EQ(env_->Object_SetPropertyByName_Byte(c1, "prop", 1U), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(c1, "prop", &prop), ANI_OK);
    ASSERT_EQ(prop, 1U);
}

TEST_F(ObjectGetPropertyByNameByteTest, set_interface_property)
{
    ani_object c2 = NewC2();

    ani_byte prop;
    ASSERT_EQ(env_->Object_SetPropertyByName_Byte(c2, "prop", 0U), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(c2, "prop", &prop), ANI_OK);
    ASSERT_EQ(prop, 0U);

    ASSERT_EQ(env_->Object_SetPropertyByName_Byte(c2, "prop", 1U), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Byte(c2, "prop", &prop), ANI_OK);
    ASSERT_EQ(prop, 1U);
}

}  // namespace ark::ets::ani::testing
