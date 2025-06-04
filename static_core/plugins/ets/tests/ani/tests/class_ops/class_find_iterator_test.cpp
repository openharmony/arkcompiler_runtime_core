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

class ClassFindIteratorTest : public AniTest {
public:
    void GetTestData(ani_class *clsResult, ani_method *ctorResult)
    {
        ani_class cls;
        ASSERT_EQ(env_->FindClass("Lclass_find_iterator_test/Singleton;", &cls), ANI_OK);

        ani_method ctor;
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":V", &ctor), ANI_OK);

        *clsResult = cls;
        *ctorResult = ctor;
    }
};

TEST_F(ClassFindIteratorTest, find_iterator)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_find_iterator_test/Singleton;", &cls), ANI_OK);

    ani_method result;
    ASSERT_EQ(env_->Class_FindIterator(cls, &result), ANI_OK);

    ASSERT_NE(result, nullptr);
}

TEST_F(ClassFindIteratorTest, find_iterator_in_namespace)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_find_iterator_test/ops/Singleton;", &cls), ANI_OK);

    ani_method result;
    ASSERT_EQ(env_->Class_FindIterator(cls, &result), ANI_OK);

    ASSERT_NE(result, nullptr);
}

TEST_F(ClassFindIteratorTest, invalid_argument1)
{
    ani_class cls;
    ASSERT_EQ(env_->FindClass("Lclass_find_iterator_test/ops/Singleton;", &cls), ANI_OK);
    ani_method result;
    ASSERT_EQ(env_->Class_FindIterator(nullptr, &result), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->Class_FindIterator(cls, nullptr), ANI_INVALID_ARGS);
}

}  // namespace ark::ets::ani::testing