/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "plugins/ets/tests/mock/accessing_objects_fields_test_helper.h"

namespace ark::ets::test {

static const char *g_testBinFileName = "AccessingObjectsFieldsTest.abc";

class AccessingObjectsFieldsTestGeneral : public AccessingObjectsFieldsTestBase {
public:
    AccessingObjectsFieldsTestGeneral() : AccessingObjectsFieldsTestBase(g_testBinFileName) {}
};

class AccessingObjectsFieldsTest : public AccessingObjectsFieldsTestGeneral {};
class AccessingObjectsFieldsTestDeath : public AccessingObjectsFieldsTestGeneral {};

TEST_F(AccessingObjectsFieldsTestDeath, GetTypeFieldDeathTest1)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        EXPECT_DEATH(env_->GetObjectField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetBooleanField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetByteField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetCharField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetShortField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetIntField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetLongField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetFloatField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetDoubleField(nullptr, nullptr), "");
    }

    {
        ets_class cls = env_->FindClass("F");
        ASSERT_NE(cls, nullptr);
        ets_field member0Id = env_->Getp_field(cls, "member0", "Z");
        ASSERT_NE(member0Id, nullptr);
        ets_field member1Id = env_->Getp_field(cls, "member1", "B");
        ASSERT_NE(member1Id, nullptr);
        ets_field member2Id = env_->Getp_field(cls, "member2", "C");
        ASSERT_NE(member2Id, nullptr);
        ets_field member3Id = env_->Getp_field(cls, "member3", "S");
        ASSERT_NE(member3Id, nullptr);
        ets_field member4Id = env_->Getp_field(cls, "member4", "I");
        ASSERT_NE(member4Id, nullptr);
        ets_field member5Id = env_->Getp_field(cls, "member5", "J");
        ASSERT_NE(member5Id, nullptr);
        ets_field member6Id = env_->Getp_field(cls, "member6", "F");
        ASSERT_NE(member6Id, nullptr);
        ets_field member7Id = env_->Getp_field(cls, "member7", "D");
        ASSERT_NE(member7Id, nullptr);
        ets_field member8Id = env_->Getp_field(cls, "member8", "LA;");
        ASSERT_NE(member8Id, nullptr);

        EXPECT_DEATH(env_->GetObjectField(nullptr, member8Id), "");
        EXPECT_DEATH(env_->GetBooleanField(nullptr, member0Id), "");
        EXPECT_DEATH(env_->GetByteField(nullptr, member1Id), "");
        EXPECT_DEATH(env_->GetCharField(nullptr, member2Id), "");
        EXPECT_DEATH(env_->GetShortField(nullptr, member3Id), "");
        EXPECT_DEATH(env_->GetIntField(nullptr, member4Id), "");
        EXPECT_DEATH(env_->GetLongField(nullptr, member5Id), "");
        EXPECT_DEATH(env_->GetFloatField(nullptr, member6Id), "");
        EXPECT_DEATH(env_->GetDoubleField(nullptr, member7Id), "");
    }
}

TEST_F(AccessingObjectsFieldsTestDeath, GetTypeFieldDeathTest2)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);
    ets_method ctor = env_->Getp_method(cls, "<ctor>", ":V");
    ASSERT_NE(ctor, nullptr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ets_object obj = env_->NewObject(cls, ctor);
    ASSERT_NE(obj, nullptr);

    EXPECT_DEATH(env_->GetObjectField(obj, nullptr), "");
    EXPECT_DEATH(env_->GetBooleanField(obj, nullptr), "");
    EXPECT_DEATH(env_->GetByteField(obj, nullptr), "");
    EXPECT_DEATH(env_->GetCharField(obj, nullptr), "");
    EXPECT_DEATH(env_->GetShortField(obj, nullptr), "");
    EXPECT_DEATH(env_->GetIntField(obj, nullptr), "");
    EXPECT_DEATH(env_->GetLongField(obj, nullptr), "");
    EXPECT_DEATH(env_->GetFloatField(obj, nullptr), "");
    EXPECT_DEATH(env_->GetDoubleField(obj, nullptr), "");
}

}  // namespace ark::ets::test