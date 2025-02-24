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

class ObjectSetFieldByNameShortTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }
};

constexpr char CMP_VALUE = 2;
constexpr char SET_VALUE = 15;

TEST_F(ObjectSetFieldByNameShortTest, set_field)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkObjectField", animal, ani_short(CMP_VALUE)), ANI_TRUE);

    ASSERT_EQ(env_->Object_SetFieldByName_Short(animal, "value", ani_short(SET_VALUE)), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkObjectField", animal, ani_short(SET_VALUE)), ANI_TRUE);
}

TEST_F(ObjectSetFieldByNameShortTest, not_found_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Short(animal, "x", ani_short(SET_VALUE)), ANI_NOT_FOUND);
}

TEST_F(ObjectSetFieldByNameShortTest, invalid_type)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Short(animal, "name", ani_short(SET_VALUE)), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldByNameShortTest, invalid_object)
{
    ASSERT_EQ(env_->Object_SetFieldByName_Short(nullptr, "x", ani_short(SET_VALUE)), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameShortTest, invalid_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Short(animal, nullptr, ani_short(SET_VALUE)), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
