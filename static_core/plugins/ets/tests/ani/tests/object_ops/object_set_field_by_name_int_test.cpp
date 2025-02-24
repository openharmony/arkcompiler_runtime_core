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

class ObjectSetFieldByNameIntTest : public AniTest {
public:
    ani_object NewAnimal()
    {
        auto animalRef = CallEtsFunction<ani_ref>("newAnimalObject");
        return static_cast<ani_object>(animalRef);
    }
};

constexpr uint32_t CMP_VALUE = 2;
constexpr uint32_t SET_VALUE = 21;

TEST_F(ObjectSetFieldByNameIntTest, set_field)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkObjectField", animal, ani_int(CMP_VALUE)), ANI_TRUE);

    ASSERT_EQ(env_->Object_SetFieldByName_Int(animal, "value", ani_int(SET_VALUE)), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("checkObjectField", animal, ani_int(SET_VALUE)), ANI_TRUE);
}

TEST_F(ObjectSetFieldByNameIntTest, not_found_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Int(animal, "x", ani_int(SET_VALUE)), ANI_NOT_FOUND);
}

TEST_F(ObjectSetFieldByNameIntTest, invalid_type)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Int(animal, "name", ani_int(SET_VALUE)), ANI_INVALID_TYPE);
}

TEST_F(ObjectSetFieldByNameIntTest, invalid_object)
{
    ASSERT_EQ(env_->Object_SetFieldByName_Int(nullptr, "x", ani_int(SET_VALUE)), ANI_INVALID_ARGS);
}

TEST_F(ObjectSetFieldByNameIntTest, invalid_name)
{
    ani_object animal = NewAnimal();
    ASSERT_EQ(env_->Object_SetFieldByName_Int(animal, nullptr, ani_int(SET_VALUE)), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing
