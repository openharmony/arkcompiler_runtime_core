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
    CheckGetSuperClass<true>("LA;");
}

TEST_F(GetSuperClassTest, get_super_class_B)
{
    CheckGetSuperClass<true>("LB;");
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

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
