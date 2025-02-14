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

class ObjectSetFieldByNameFloatTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }
};

constexpr uint32_t CMP_VALUE = 2.0;
constexpr uint32_t SET_VALUE = 21.0;

TEST_F(ObjectSetFieldByNameFloatTest, set_field)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkObjectField", animal, ani_float(CMP_VALUE)), ANI_TRUE);

    ASSERT_EQ(env_->Object_SetFieldByName_Float(animal, "value", ani_float(SET_VALUE)), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkObjectField", animal, ani_float(SET_VALUE)), ANI_TRUE);
}

TEST_F(ObjectSetFieldByNameFloatTest, not_found_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Float(animal, "x", ani_float(SET_VALUE)), ANI_NOT_FOUND);
}

TEST_F(ObjectSetFieldByNameFloatTest, invalid_type)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Float(animal, "name", ani_float(SET_VALUE)), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldByNameFloatTest, invalid_object)
{
    ASSERT_EQ(env_->Object_SetFieldByName_Float(nullptr, "x", ani_float(SET_VALUE)), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameFloatTest, invalid_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Float(animal, nullptr, ani_float(SET_VALUE)), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
