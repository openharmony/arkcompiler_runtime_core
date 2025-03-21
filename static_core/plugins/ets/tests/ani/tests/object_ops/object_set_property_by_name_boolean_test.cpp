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

class ObjectSetPropertyByNameBooleanTest : public AniTest {
public:
    ani_object NewCar()
    {
        auto carRef = CallEtsFunction<ani_ref>("object_set_property_by_name_boolean_test", "newCarObject");
        return static_cast<ani_object>(carRef);
    }

    ani_object NewC1()
    {
        auto c1Ref = CallEtsFunction<ani_ref>("object_set_property_by_name_boolean_test", "newC1");
        return static_cast<ani_object>(c1Ref);
    }

    ani_object NewC2()
    {
        auto c2Ref = CallEtsFunction<ani_ref>("object_set_property_by_name_boolean_test", "newC2");
        return static_cast<ani_object>(c2Ref);
    }
};

TEST_F(ObjectSetPropertyByNameBooleanTest, set_field_property)
{
    ani_object car = NewCar();

    ani_boolean highPerformance;
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "highPerformance", &highPerformance), ANI_OK);
    ASSERT_EQ(highPerformance, ANI_FALSE);

    ASSERT_EQ(env_->Object_SetPropertyByName_Boolean(car, "highPerformance", ANI_TRUE), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "highPerformance", &highPerformance), ANI_OK);
    ASSERT_EQ(highPerformance, ANI_TRUE);
}

TEST_F(ObjectSetPropertyByNameBooleanTest, set_setter_property)
{
    ani_object car = NewCar();

    ani_boolean ecoFriendly;
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "ecoFriendly", &ecoFriendly), ANI_OK);
    ASSERT_EQ(ecoFriendly, ANI_FALSE);

    ASSERT_EQ(env_->Object_SetPropertyByName_Boolean(car, "ecoFriendly", ANI_TRUE), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(car, "ecoFriendly", &ecoFriendly), ANI_OK);
    ASSERT_EQ(ecoFriendly, ANI_TRUE);
}

TEST_F(ObjectSetPropertyByNameBooleanTest, invalid_argument)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_SetPropertyByName_Boolean(nullptr, "highPerformance", ANI_TRUE), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_SetPropertyByName_Boolean(car, nullptr, ANI_TRUE), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetPropertyByNameBooleanTest, set_field_property_invalid_type)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_SetPropertyByName_Boolean(car, "manufacturer", ANI_TRUE), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetPropertyByNameBooleanTest, set_setter_property_invalid_type)
{
    ani_object car = NewCar();

    ASSERT_EQ(env_->Object_SetPropertyByName_Boolean(car, "model", ANI_TRUE), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetPropertyByNameBooleanTest, set_interface_field)
{
    ani_object c1 = NewC1();

    ani_boolean prop;
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(c1, "prop", &prop), ANI_OK);
    ASSERT_EQ(prop, ANI_FALSE);

    ASSERT_EQ(env_->Object_SetPropertyByName_Boolean(c1, "prop", ANI_TRUE), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(c1, "prop", &prop), ANI_OK);
    ASSERT_EQ(prop, ANI_TRUE);
}

TEST_F(ObjectSetPropertyByNameBooleanTest, set_interface_property)
{
    ani_object c2 = NewC2();

    ani_boolean prop;
    ASSERT_EQ(env_->Object_SetPropertyByName_Boolean(c2, "prop", ANI_FALSE), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(c2, "prop", &prop), ANI_OK);
    ASSERT_EQ(prop, ANI_FALSE);

    ASSERT_EQ(env_->Object_SetPropertyByName_Boolean(c2, "prop", ANI_TRUE), ANI_OK);
    ASSERT_EQ(env_->Object_GetPropertyByName_Boolean(c2, "prop", &prop), ANI_OK);
    ASSERT_EQ(prop, ANI_TRUE);
}

}  // namespace ark::ets::ani::testing
