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

class ObjectGetPropertyByNameFloatTest : public AniTest {
public:
    ani_object NewCar()
    {
        auto carRef = CallEtsFunction<ani_ref>("object_get_property_by_name_float_test", "newCarObject");
        return static_cast<ani_object>(carRef);
    }
};

TEST_F(ObjectGetPropertyByNameFloatTest, get_field_property)
{
    ani_object car = NewCar();

    ani_float length;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "length", &length), ANI_OK);
    ASSERT_EQ(length, 4275.0F);
}

TEST_F(ObjectGetPropertyByNameFloatTest, get_getter_property)
{
    ani_object car = NewCar();

    ani_float year;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "year", &year), ANI_OK);
    ASSERT_EQ(year, 1989.0F);
}

TEST_F(ObjectGetPropertyByNameFloatTest, invalid_argument)
{
    ani_object car = NewCar();

    ani_float length;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(nullptr, "length", &length), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, nullptr, &length), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "length", nullptr), ANI_INVALID_ARGS);
}

TEST_F(ObjectGetPropertyByNameFloatTest, get_field_property_invalid_type)
{
    ani_object car = NewCar();

    ani_float manufacturer;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "manufacturer", &manufacturer), ANI_INVALID_TYPE);
}

TEST_F(ObjectGetPropertyByNameFloatTest, get_getter_property_invalid_type)
{
    ani_object car = NewCar();

    ani_float model;
    ASSERT_EQ(env_->Object_GetPropertyByName_Float(car, "model", &model), ANI_INVALID_TYPE);
}

}  // namespace ark::ets::ani::testing
