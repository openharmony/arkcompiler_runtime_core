/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg)

namespace ark::ets::test {

static const char *g_testBinFileName = "AccessingObjectsFieldsTest.abc";

class AccessingObjectsFieldsTestGeneral : public AccessingObjectsFieldsTestBase {
public:
    AccessingObjectsFieldsTestGeneral() : AccessingObjectsFieldsTestBase(g_testBinFileName) {}
};

class AccessingObjectsFieldsTest : public AccessingObjectsFieldsTestGeneral {};
class AccessingObjectsFieldsTestDeath : public AccessingObjectsFieldsTestGeneral {};

TEST_F(AccessingObjectsFieldsTestDeath, DISABLED_SetTypeFieldDeathTests1)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    EXPECT_DEATH(env_->SetObjectField(nullptr, nullptr, nullptr), "");

    ets_class aCls = env_->FindClass("AccessingObjectsFieldsTest/A");
    ASSERT_NE(aCls, nullptr);
    ets_method aCtor = env_->Getp_method(aCls, "<ctor>", ":V");
    ASSERT_NE(aCtor, nullptr);
    ets_object aObj = env_->NewObject(aCls, aCtor);
    ASSERT_NE(aObj, nullptr);

    EXPECT_DEATH(env_->SetObjectField(nullptr, nullptr, aObj), "");
    EXPECT_DEATH(env_->SetBooleanField(nullptr, nullptr, static_cast<ets_boolean>(0)), "");
    EXPECT_DEATH(env_->SetByteField(nullptr, nullptr, static_cast<ets_byte>(0)), "");
    EXPECT_DEATH(env_->SetCharField(nullptr, nullptr, static_cast<ets_char>(0)), "");
    EXPECT_DEATH(env_->SetShortField(nullptr, nullptr, static_cast<ets_short>(0)), "");
    EXPECT_DEATH(env_->SetIntField(nullptr, nullptr, static_cast<ets_int>(0)), "");
    EXPECT_DEATH(env_->SetLongField(nullptr, nullptr, static_cast<ets_long>(0)), "");
    EXPECT_DEATH(env_->SetFloatField(nullptr, nullptr, static_cast<ets_float>(0.0F)), "");
    EXPECT_DEATH(env_->SetDoubleField(nullptr, nullptr, static_cast<ets_double>(0.0)), "");
}

TEST_F(AccessingObjectsFieldsTestDeath, DISABLED_SetTypeFieldDeathTests2)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("AccessingObjectsFieldsTest/F");
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
    ets_field member8Id = env_->Getp_field(cls, "member8", "LAccessingObjectsFieldsTest/A;");
    ASSERT_NE(member8Id, nullptr);

    EXPECT_DEATH(env_->SetObjectField(nullptr, member8Id, nullptr), "");

    ets_class aCls = env_->FindClass("AccessingObjectsFieldsTest/A");
    ASSERT_NE(aCls, nullptr);
    ets_method aCtor = env_->Getp_method(aCls, "<ctor>", ":V");
    ASSERT_NE(aCtor, nullptr);
    ets_object aObj = env_->NewObject(aCls, aCtor);
    ASSERT_NE(aObj, nullptr);

    EXPECT_DEATH(env_->SetObjectField(nullptr, member8Id, aObj), "");
    EXPECT_DEATH(env_->SetBooleanField(nullptr, member0Id, static_cast<ets_boolean>(0)), "");
    EXPECT_DEATH(env_->SetByteField(nullptr, member1Id, static_cast<ets_byte>(0)), "");
    EXPECT_DEATH(env_->SetCharField(nullptr, member2Id, static_cast<ets_char>(0)), "");
    EXPECT_DEATH(env_->SetShortField(nullptr, member3Id, static_cast<ets_short>(0)), "");
    EXPECT_DEATH(env_->SetIntField(nullptr, member4Id, static_cast<ets_int>(0)), "");
    EXPECT_DEATH(env_->SetLongField(nullptr, member5Id, static_cast<ets_long>(0)), "");
    EXPECT_DEATH(env_->SetFloatField(nullptr, member6Id, static_cast<ets_float>(0.0F)), "");
    EXPECT_DEATH(env_->SetDoubleField(nullptr, member7Id, static_cast<ets_double>(0.0)), "");
}

TEST_F(AccessingObjectsFieldsTestDeath, DISABLED_SetTypeFieldDeathTests3)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class cls = env_->FindClass("AccessingObjectsFieldsTest/F");
    ASSERT_NE(cls, nullptr);
    ets_method ctor = env_->Getp_method(cls, "<ctor>", ":V");
    ASSERT_NE(ctor, nullptr);
    ets_object obj = env_->NewObject(cls, ctor);
    ASSERT_NE(obj, nullptr);

    EXPECT_DEATH(env_->SetObjectField(obj, nullptr, nullptr), "");

    ets_class aCls = env_->FindClass("AccessingObjectsFieldsTest/F");
    ASSERT_NE(aCls, nullptr);
    ets_method aCtor = env_->Getp_method(aCls, "<ctor>", ":V");
    ASSERT_NE(aCtor, nullptr);
    ets_object aObj = env_->NewObject(aCls, aCtor);
    ASSERT_NE(aObj, nullptr);

    EXPECT_DEATH(env_->SetObjectField(obj, nullptr, aObj), "");
    EXPECT_DEATH(env_->SetBooleanField(obj, nullptr, static_cast<ets_boolean>(0)), "");
    EXPECT_DEATH(env_->SetByteField(obj, nullptr, static_cast<ets_byte>(0)), "");
    EXPECT_DEATH(env_->SetCharField(obj, nullptr, static_cast<ets_char>(0)), "");
    EXPECT_DEATH(env_->SetShortField(obj, nullptr, static_cast<ets_short>(0)), "");
    EXPECT_DEATH(env_->SetIntField(obj, nullptr, static_cast<ets_int>(0)), "");
    EXPECT_DEATH(env_->SetLongField(obj, nullptr, static_cast<ets_long>(0)), "");
    EXPECT_DEATH(env_->SetFloatField(obj, nullptr, static_cast<ets_float>(0.0F)), "");
    EXPECT_DEATH(env_->SetDoubleField(obj, nullptr, static_cast<ets_double>(0.0)), "");
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg)
