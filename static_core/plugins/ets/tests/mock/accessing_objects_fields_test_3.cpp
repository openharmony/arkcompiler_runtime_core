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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)

namespace ark::ets::test {

static const char *g_testBinFileName = "AccessingObjectsFieldsTest.abc";

class AccessingObjectsFieldsTestGeneral : public AccessingObjectsFieldsTestBase {
public:
    AccessingObjectsFieldsTestGeneral() : AccessingObjectsFieldsTestBase(g_testBinFileName) {}
};

class AccessingObjectsFieldsTest : public AccessingObjectsFieldsTestGeneral {};
class AccessingObjectsFieldsTestDeath : public AccessingObjectsFieldsTestGeneral {};

TEST_F(AccessingObjectsFieldsTestDeath, GetFieldIDDeathTests)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        EXPECT_DEATH(env_->Getp_field(nullptr, "some text", "some text"), "");
        EXPECT_DEATH(env_->Getp_field(nullptr, nullptr, "some text"), "");
        EXPECT_DEATH(env_->Getp_field(nullptr, "some text", nullptr), "");
        EXPECT_DEATH(env_->Getp_field(nullptr, nullptr, nullptr), "");
    }

    {
        ets_class cls = env_->FindClass("A");
        EXPECT_DEATH(env_->Getp_field(cls, nullptr, "some text"), "");
        EXPECT_DEATH(env_->Getp_field(cls, "some text", nullptr), "");
    }
}

TEST_F(AccessingObjectsFieldsTest, GetFieldID2)
{
    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);

    ets_field memberId = env_->Getp_field(cls, "member4", "I");
    ASSERT_NE(memberId, nullptr);
}

// NOTE(m.morozov): enable this test when inheritance will be implemented
TEST_F(AccessingObjectsFieldsTest, DISABLED_GetBaseFieldID2)
{
    ets_class cls = env_->FindClass("F_sub");
    ASSERT_NE(cls, nullptr);

    ets_field memberId = env_->Getp_field(cls, "member4", "I");
    ASSERT_NE(memberId, nullptr);
}

TEST_F(AccessingObjectsFieldsTest, GetTypeField)
{
    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);
    ets_method ctor = env_->Getp_method(cls, "<ctor>", ":V");
    ASSERT_NE(ctor, nullptr);
    ets_object obj = env_->NewObject(cls, ctor);
    ASSERT_NE(obj, nullptr);

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

    EXPECT_EQ(env_->GetBooleanField(obj, member0Id), static_cast<ets_boolean>(1));
    EXPECT_EQ(env_->GetByteField(obj, member1Id), static_cast<ets_byte>(2_I));
    EXPECT_EQ(env_->GetCharField(obj, member2Id), static_cast<ets_char>(3U));
    EXPECT_EQ(env_->GetShortField(obj, member3Id), static_cast<ets_short>(4_I));
    EXPECT_EQ(env_->GetIntField(obj, member4Id), static_cast<ets_int>(5_I));
    EXPECT_EQ(env_->GetLongField(obj, member5Id), static_cast<ets_long>(6_I));
    EXPECT_FLOAT_EQ(env_->GetFloatField(obj, member6Id), static_cast<ets_float>(7.0F));
    EXPECT_DOUBLE_EQ(env_->GetDoubleField(obj, member7Id), static_cast<ets_double>(8.0F));

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);
    ets_field aMemberId = env_->Getp_field(aCls, "member", "I");
    ASSERT_NE(aMemberId, nullptr);
    ets_object aObj = env_->GetObjectField(obj, member8Id);
    ASSERT_NE(aObj, nullptr);
    EXPECT_EQ(env_->GetIntField(aObj, aMemberId), static_cast<ets_int>(1));
}

struct ClassMembers {
    ets_field member0;
    ets_field member1;
    ets_field member2;
    ets_field member3;
    ets_field member4;
    ets_field member5;
    ets_field member6;
    ets_field member7;
};

static void SetterHelper(EtsEnv *env, ets_object obj, ClassMembers &members)
{
    env->SetBooleanField(obj, members.member0, static_cast<ets_boolean>(1));
    env->SetByteField(obj, members.member1, static_cast<ets_byte>(1));
    env->SetCharField(obj, members.member2, static_cast<ets_char>(1));
    env->SetShortField(obj, members.member3, static_cast<ets_short>(1));
    env->SetIntField(obj, members.member4, static_cast<ets_int>(1));
    env->SetLongField(obj, members.member5, static_cast<ets_long>(1));
    env->SetFloatField(obj, members.member6, static_cast<ets_float>(1.0F));
    env->SetDoubleField(obj, members.member7, static_cast<ets_double>(1.0));
}

TEST_F(AccessingObjectsFieldsTest, SetTypeField)
{
    ets_class cls = env_->FindClass("F");
    ASSERT_NE(cls, nullptr);
    ets_method ctor = env_->Getp_method(cls, "<ctor>", ":V");
    ASSERT_NE(ctor, nullptr);
    ets_object obj = env_->NewObject(cls, ctor);
    ASSERT_NE(obj, nullptr);

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

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);
    ets_method aCtor = env_->Getp_method(aCls, "<ctor>", ":V");
    ASSERT_NE(aCtor, nullptr);
    ets_object aObj = env_->NewObject(aCls, aCtor);
    ASSERT_NE(aObj, nullptr);
    ets_field aMemberId = env_->Getp_field(aCls, "member", "I");
    ASSERT_NE(aMemberId, nullptr);

    ClassMembers members {member0Id, member1Id, member2Id, member3Id, member4Id, member5Id, member6Id, member7Id};
    SetterHelper(env_, obj, members);

    env_->SetIntField(aObj, aMemberId, static_cast<ets_int>(5_I));
    env_->SetObjectField(obj, member8Id, aObj);

    EXPECT_EQ(env_->GetBooleanField(obj, member0Id), static_cast<ets_boolean>(1));
    EXPECT_EQ(env_->GetByteField(obj, member1Id), static_cast<ets_byte>(1));
    EXPECT_EQ(env_->GetCharField(obj, member2Id), static_cast<ets_char>(1));
    EXPECT_EQ(env_->GetShortField(obj, member3Id), static_cast<ets_short>(1));
    EXPECT_EQ(env_->GetIntField(obj, member4Id), static_cast<ets_int>(1));
    EXPECT_EQ(env_->GetLongField(obj, member5Id), static_cast<ets_long>(1));
    EXPECT_FLOAT_EQ(env_->GetFloatField(obj, member6Id), static_cast<ets_float>(1.0F));
    EXPECT_DOUBLE_EQ(env_->GetDoubleField(obj, member7Id), static_cast<ets_double>(1.0));

    ets_object setAObj = env_->GetObjectField(obj, member8Id);
    ASSERT_NE(setAObj, nullptr);
    EXPECT_EQ(env_->GetIntField(setAObj, aMemberId), static_cast<ets_int>(5_I));
}

TEST_F(AccessingObjectsFieldsTestDeath, GetStaticFieldIDDeathTests)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        EXPECT_DEATH(env_->GetStaticp_field(nullptr, "some text", "some text"), "");
        EXPECT_DEATH(env_->GetStaticp_field(nullptr, nullptr, "some text"), "");
        EXPECT_DEATH(env_->GetStaticp_field(nullptr, "some text", nullptr), "");
        EXPECT_DEATH(env_->GetStaticp_field(nullptr, nullptr, nullptr), "");
    }

    {
        ets_class cls = env_->FindClass("A");
        EXPECT_DEATH(env_->GetStaticp_field(cls, nullptr, "some text"), "");
        EXPECT_DEATH(env_->GetStaticp_field(cls, "some text", nullptr), "");
    }
}

TEST_F(AccessingObjectsFieldsTest, GetStaticFieldID2)
{
    ets_class cls = env_->FindClass("F_static");
    ASSERT_NE(cls, nullptr);

    ets_field memberId = env_->GetStaticp_field(cls, "member4", "I");
    ASSERT_NE(memberId, nullptr);
}

TEST_F(AccessingObjectsFieldsTest, GetStaticTypeField)
{
    ets_class cls = env_->FindClass("F_static");
    ASSERT_NE(cls, nullptr);

    ets_field member0Id = env_->GetStaticp_field(cls, "member0", "Z");
    ASSERT_NE(member0Id, nullptr);
    ets_field member1Id = env_->GetStaticp_field(cls, "member1", "B");
    ASSERT_NE(member1Id, nullptr);
    ets_field member2Id = env_->GetStaticp_field(cls, "member2", "C");
    ASSERT_NE(member2Id, nullptr);
    ets_field member3Id = env_->GetStaticp_field(cls, "member3", "S");
    ASSERT_NE(member3Id, nullptr);
    ets_field member4Id = env_->GetStaticp_field(cls, "member4", "I");
    ASSERT_NE(member4Id, nullptr);
    ets_field member5Id = env_->GetStaticp_field(cls, "member5", "J");
    ASSERT_NE(member5Id, nullptr);
    ets_field member6Id = env_->GetStaticp_field(cls, "member6", "F");
    ASSERT_NE(member6Id, nullptr);
    ets_field member7Id = env_->GetStaticp_field(cls, "member7", "D");
    ASSERT_NE(member7Id, nullptr);
    ets_field member8Id = env_->GetStaticp_field(cls, "member8", "LA;");
    ASSERT_NE(member8Id, nullptr);

    EXPECT_EQ(env_->GetStaticBooleanField(cls, member0Id), static_cast<ets_boolean>(1));
    EXPECT_EQ(env_->GetStaticByteField(cls, member1Id), static_cast<ets_byte>(2_I));
    EXPECT_EQ(env_->GetStaticCharField(cls, member2Id), static_cast<ets_char>(3U));
    EXPECT_EQ(env_->GetStaticShortField(cls, member3Id), static_cast<ets_short>(4_I));
    EXPECT_EQ(env_->GetStaticIntField(cls, member4Id), static_cast<ets_int>(5_I));
    EXPECT_EQ(env_->GetStaticLongField(cls, member5Id), static_cast<ets_long>(6_I));
    EXPECT_FLOAT_EQ(env_->GetStaticFloatField(cls, member6Id), static_cast<ets_float>(7.0F));
    EXPECT_DOUBLE_EQ(env_->GetStaticDoubleField(cls, member7Id), static_cast<ets_double>(8.0F));

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);
    ets_field aMemberId = env_->Getp_field(aCls, "member", "I");
    ASSERT_NE(aMemberId, nullptr);
    ets_object aObj = env_->GetStaticObjectField(cls, member8Id);
    ASSERT_NE(aObj, nullptr);
    EXPECT_EQ(env_->GetIntField(aObj, aMemberId), static_cast<ets_int>(1));
}

TEST_F(AccessingObjectsFieldsTest, SetStaticField)
{
    ets_class cls = env_->FindClass("F_static");
    ASSERT_NE(cls, nullptr);

    ets_field member0Id = env_->GetStaticp_field(cls, "member0", "Z");
    ASSERT_NE(member0Id, nullptr);
    ets_field member1Id = env_->GetStaticp_field(cls, "member1", "B");
    ASSERT_NE(member1Id, nullptr);
    ets_field member2Id = env_->GetStaticp_field(cls, "member2", "C");
    ASSERT_NE(member2Id, nullptr);
    ets_field member3Id = env_->GetStaticp_field(cls, "member3", "S");
    ASSERT_NE(member3Id, nullptr);
    ets_field member4Id = env_->GetStaticp_field(cls, "member4", "I");
    ASSERT_NE(member4Id, nullptr);
    ets_field member5Id = env_->GetStaticp_field(cls, "member5", "J");
    ASSERT_NE(member5Id, nullptr);
    ets_field member6Id = env_->GetStaticp_field(cls, "member6", "F");
    ASSERT_NE(member6Id, nullptr);
    ets_field member7Id = env_->GetStaticp_field(cls, "member7", "D");
    ASSERT_NE(member7Id, nullptr);
    ets_field member8Id = env_->GetStaticp_field(cls, "member8", "LA;");
    ASSERT_NE(member8Id, nullptr);

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);
    ets_field aMemberId = env_->Getp_field(aCls, "member", "I");
    ASSERT_NE(aMemberId, nullptr);
    ets_object aObj = env_->GetStaticObjectField(cls, member8Id);
    ASSERT_NE(aObj, nullptr);

    env_->SetIntField(aObj, aMemberId, static_cast<ets_int>(5_I));

    env_->SetStaticBooleanField(cls, member0Id, static_cast<ets_boolean>(1));
    env_->SetStaticByteField(cls, member1Id, static_cast<ets_byte>(10_I));
    env_->SetStaticCharField(cls, member2Id, static_cast<ets_char>(20U));
    env_->SetStaticShortField(cls, member3Id, static_cast<ets_short>(30_I));
    env_->SetStaticIntField(cls, member4Id, static_cast<ets_int>(40_I));
    env_->SetStaticLongField(cls, member5Id, static_cast<ets_long>(50_I));
    env_->SetStaticFloatField(cls, member6Id, static_cast<ets_float>(60.0F));
    env_->SetStaticDoubleField(cls, member7Id, static_cast<ets_double>(70.0F));
    env_->SetStaticObjectField(cls, member8Id, aObj);

    EXPECT_EQ(env_->GetStaticBooleanField(cls, member0Id), static_cast<ets_boolean>(1));
    EXPECT_EQ(env_->GetStaticByteField(cls, member1Id), static_cast<ets_byte>(10_I));
    EXPECT_EQ(env_->GetStaticCharField(cls, member2Id), static_cast<ets_char>(20U));
    EXPECT_EQ(env_->GetStaticShortField(cls, member3Id), static_cast<ets_short>(30_I));
    EXPECT_EQ(env_->GetStaticIntField(cls, member4Id), static_cast<ets_int>(40_I));
    EXPECT_EQ(env_->GetStaticLongField(cls, member5Id), static_cast<ets_long>(50_I));
    EXPECT_FLOAT_EQ(env_->GetStaticFloatField(cls, member6Id), static_cast<ets_float>(60.0F));
    EXPECT_DOUBLE_EQ(env_->GetStaticDoubleField(cls, member7Id), static_cast<ets_double>(70.0F));

    ets_object setAObj = env_->GetStaticObjectField(cls, member8Id);
    ASSERT_NE(setAObj, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(setAObj, aCls), ETS_TRUE);
    EXPECT_EQ(env_->GetIntField(setAObj, aMemberId), static_cast<ets_int>(5_I));
}

TEST_F(AccessingObjectsFieldsTest, SetStaticField2)
{
    ets_class cls = env_->FindClass("F_static");
    ASSERT_NE(cls, nullptr);

    ets_field member0Id = env_->GetStaticp_field(cls, "member0", "Z");
    ASSERT_NE(member0Id, nullptr);
    ets_field member1Id = env_->GetStaticp_field(cls, "member1", "B");
    ASSERT_NE(member1Id, nullptr);
    ets_field member2Id = env_->GetStaticp_field(cls, "member2", "C");
    ASSERT_NE(member2Id, nullptr);
    ets_field member3Id = env_->GetStaticp_field(cls, "member3", "S");
    ASSERT_NE(member3Id, nullptr);
    ets_field member4Id = env_->GetStaticp_field(cls, "member4", "I");
    ASSERT_NE(member4Id, nullptr);
    ets_field member5Id = env_->GetStaticp_field(cls, "member5", "J");
    ASSERT_NE(member5Id, nullptr);
    ets_field member6Id = env_->GetStaticp_field(cls, "member6", "F");
    ASSERT_NE(member6Id, nullptr);
    ets_field member7Id = env_->GetStaticp_field(cls, "member7", "D");
    ASSERT_NE(member7Id, nullptr);
    ets_field member8Id = env_->GetStaticp_field(cls, "member8", "LA;");
    ASSERT_NE(member8Id, nullptr);

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);
    ets_field aMemberId = env_->Getp_field(aCls, "member", "I");
    ASSERT_NE(aMemberId, nullptr);
    ets_object aObj = env_->GetStaticObjectField(cls, member8Id);
    ASSERT_NE(aObj, nullptr);

    env_->SetIntField(aObj, aMemberId, static_cast<ets_int>(5_I));

    env_->SetStaticBooleanField(nullptr, member0Id, static_cast<ets_boolean>(1));
    env_->SetStaticByteField(nullptr, member1Id, static_cast<ets_byte>(10_I));
    env_->SetStaticCharField(nullptr, member2Id, static_cast<ets_char>(20U));
    env_->SetStaticShortField(nullptr, member3Id, static_cast<ets_short>(30_I));
    env_->SetStaticIntField(nullptr, member4Id, static_cast<ets_int>(40_I));
    env_->SetStaticLongField(nullptr, member5Id, static_cast<ets_long>(50_I));
    env_->SetStaticFloatField(nullptr, member6Id, static_cast<ets_float>(60.0F));
    env_->SetStaticDoubleField(nullptr, member7Id, static_cast<ets_double>(70.0F));
    env_->SetStaticObjectField(nullptr, member8Id, aObj);

    EXPECT_EQ(env_->GetStaticBooleanField(nullptr, member0Id), static_cast<ets_boolean>(1));
    EXPECT_EQ(env_->GetStaticByteField(nullptr, member1Id), static_cast<ets_byte>(10_I));
    EXPECT_EQ(env_->GetStaticCharField(nullptr, member2Id), static_cast<ets_char>(20U));
    EXPECT_EQ(env_->GetStaticShortField(nullptr, member3Id), static_cast<ets_short>(30_I));
    EXPECT_EQ(env_->GetStaticIntField(nullptr, member4Id), static_cast<ets_int>(40_I));
    EXPECT_EQ(env_->GetStaticLongField(nullptr, member5Id), static_cast<ets_long>(50_I));
    EXPECT_FLOAT_EQ(env_->GetStaticFloatField(nullptr, member6Id), static_cast<ets_float>(60.0F));
    EXPECT_DOUBLE_EQ(env_->GetStaticDoubleField(nullptr, member7Id), static_cast<ets_double>(70.0F));

    ets_object setAObj = env_->GetStaticObjectField(cls, member8Id);
    ASSERT_NE(setAObj, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(setAObj, aCls), ETS_TRUE);
    EXPECT_EQ(env_->GetIntField(setAObj, aMemberId), static_cast<ets_int>(5_I));
}

TEST_F(AccessingObjectsFieldsTestDeath, GetStaticTypeFieldDeathTests)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        EXPECT_DEATH(env_->GetStaticObjectField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetStaticBooleanField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetStaticByteField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetStaticCharField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetStaticShortField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetStaticIntField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetStaticLongField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetStaticFloatField(nullptr, nullptr), "");
        EXPECT_DEATH(env_->GetStaticDoubleField(nullptr, nullptr), "");
    }

    {
        ets_class cls = env_->FindClass("F_static");
        ASSERT_NE(cls, nullptr);

        EXPECT_DEATH(env_->GetStaticObjectField(cls, nullptr), "");
        EXPECT_DEATH(env_->GetStaticBooleanField(cls, nullptr), "");
        EXPECT_DEATH(env_->GetStaticByteField(cls, nullptr), "");
        EXPECT_DEATH(env_->GetStaticCharField(cls, nullptr), "");
        EXPECT_DEATH(env_->GetStaticShortField(cls, nullptr), "");
        EXPECT_DEATH(env_->GetStaticIntField(cls, nullptr), "");
        EXPECT_DEATH(env_->GetStaticLongField(cls, nullptr), "");
        EXPECT_DEATH(env_->GetStaticFloatField(cls, nullptr), "");
        EXPECT_DEATH(env_->GetStaticDoubleField(cls, nullptr), "");
    }
}

// NOTE(m.morozov): enable this test when inheritance will be implemented
TEST_F(AccessingObjectsFieldsTest, DISABLED_GetStaticTypeFieldBase)
{
    ets_class cls = env_->FindClass("F_static_sub");
    ASSERT_NE(cls, nullptr);

    ets_field member0Id = env_->GetStaticp_field(cls, "member0", "Z");
    ASSERT_NE(member0Id, nullptr);
    ets_field member1Id = env_->GetStaticp_field(cls, "member1", "B");
    ASSERT_NE(member1Id, nullptr);
    ets_field member2Id = env_->GetStaticp_field(cls, "member2", "C");
    ASSERT_NE(member2Id, nullptr);
    ets_field member3Id = env_->GetStaticp_field(cls, "member3", "S");
    ASSERT_NE(member3Id, nullptr);
    ets_field member4Id = env_->GetStaticp_field(cls, "member4", "I");
    ASSERT_NE(member4Id, nullptr);
    ets_field member5Id = env_->GetStaticp_field(cls, "member5", "J");
    ASSERT_NE(member5Id, nullptr);
    ets_field member6Id = env_->GetStaticp_field(cls, "member6", "F");
    ASSERT_NE(member6Id, nullptr);
    ets_field member7Id = env_->GetStaticp_field(cls, "member7", "D");
    ASSERT_NE(member7Id, nullptr);
    ets_field member8Id = env_->GetStaticp_field(cls, "member8", "LA;");
    ASSERT_NE(member8Id, nullptr);

    EXPECT_EQ(env_->GetStaticBooleanField(cls, member0Id), static_cast<ets_boolean>(1));
    EXPECT_EQ(env_->GetStaticByteField(cls, member1Id), static_cast<ets_byte>(2_I));
    EXPECT_EQ(env_->GetStaticCharField(cls, member2Id), static_cast<ets_char>(3U));
    EXPECT_EQ(env_->GetStaticShortField(cls, member3Id), static_cast<ets_short>(4_I));
    EXPECT_EQ(env_->GetStaticIntField(cls, member4Id), static_cast<ets_int>(5_I));
    EXPECT_EQ(env_->GetStaticLongField(cls, member5Id), static_cast<ets_long>(6L));
    EXPECT_FLOAT_EQ(env_->GetStaticFloatField(cls, member6Id), static_cast<ets_float>(7.0F));
    EXPECT_DOUBLE_EQ(env_->GetStaticDoubleField(cls, member7Id), static_cast<ets_double>(8.0_D));

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);
    ets_field aMemberId = env_->Getp_field(aCls, "member", "I");
    ASSERT_NE(aMemberId, nullptr);
    ets_object aObj = env_->GetStaticObjectField(cls, member8Id);
    ASSERT_NE(aObj, nullptr);
    EXPECT_EQ(env_->GetIntField(aObj, aMemberId), static_cast<ets_int>(1));
}

TEST_F(AccessingObjectsFieldsTestDeath, SetStaticTypeFieldDeathTests)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);
    ets_object aObj = env_->AllocObject(aCls);
    ASSERT_NE(aObj, nullptr);

    {
        EXPECT_DEATH(env_->SetStaticObjectField(nullptr, nullptr, nullptr), "");
        EXPECT_DEATH(env_->SetStaticObjectField(nullptr, nullptr, aObj), "");

        EXPECT_DEATH(env_->SetStaticBooleanField(nullptr, nullptr, static_cast<ets_boolean>(0)), "");
        EXPECT_DEATH(env_->SetStaticByteField(nullptr, nullptr, static_cast<ets_byte>(0)), "");
        EXPECT_DEATH(env_->SetStaticCharField(nullptr, nullptr, static_cast<ets_char>(0)), "");
        EXPECT_DEATH(env_->SetStaticShortField(nullptr, nullptr, static_cast<ets_short>(0)), "");
        EXPECT_DEATH(env_->SetStaticIntField(nullptr, nullptr, static_cast<ets_int>(0)), "");
        EXPECT_DEATH(env_->SetStaticLongField(nullptr, nullptr, static_cast<ets_long>(0)), "");
        EXPECT_DEATH(env_->SetStaticFloatField(nullptr, nullptr, static_cast<ets_float>(0.0F)), "");
        EXPECT_DEATH(env_->SetStaticDoubleField(nullptr, nullptr, static_cast<ets_double>(0.0)), "");
    }

    {
        ets_class cls = env_->FindClass("F_static");
        ASSERT_NE(cls, nullptr);

        EXPECT_DEATH(env_->SetStaticObjectField(cls, nullptr, nullptr), "");
        EXPECT_DEATH(env_->SetStaticObjectField(cls, nullptr, aObj), "");

        EXPECT_DEATH(env_->SetStaticBooleanField(cls, nullptr, static_cast<ets_boolean>(0)), "");
        EXPECT_DEATH(env_->SetStaticByteField(cls, nullptr, static_cast<ets_byte>(0)), "");
        EXPECT_DEATH(env_->SetStaticCharField(cls, nullptr, static_cast<ets_char>(0)), "");
        EXPECT_DEATH(env_->SetStaticShortField(cls, nullptr, static_cast<ets_short>(0)), "");
        EXPECT_DEATH(env_->SetStaticIntField(cls, nullptr, static_cast<ets_int>(0)), "");
        EXPECT_DEATH(env_->SetStaticLongField(cls, nullptr, static_cast<ets_long>(0)), "");
        EXPECT_DEATH(env_->SetStaticFloatField(cls, nullptr, static_cast<ets_float>(0.0F)), "");
        EXPECT_DEATH(env_->SetStaticDoubleField(cls, nullptr, static_cast<ets_double>(0.0)), "");
    }
}

// NOTE(m.morozov): enable this test when inheritance will be implemented
TEST_F(AccessingObjectsFieldsTest, DISABLED_SetStaticFieldBase)
{
    ets_class cls = env_->FindClass("F_static_sub");
    ASSERT_NE(cls, nullptr);

    ets_field member0Id = env_->GetStaticp_field(cls, "member0", "Z");
    ASSERT_NE(member0Id, nullptr);
    ets_field member1Id = env_->GetStaticp_field(cls, "member1", "B");
    ASSERT_NE(member1Id, nullptr);
    ets_field member2Id = env_->GetStaticp_field(cls, "member2", "C");
    ASSERT_NE(member2Id, nullptr);
    ets_field member3Id = env_->GetStaticp_field(cls, "member3", "S");
    ASSERT_NE(member3Id, nullptr);
    ets_field member4Id = env_->GetStaticp_field(cls, "member4", "I");
    ASSERT_NE(member4Id, nullptr);
    ets_field member5Id = env_->GetStaticp_field(cls, "member5", "J");
    ASSERT_NE(member5Id, nullptr);
    ets_field member6Id = env_->GetStaticp_field(cls, "member6", "F");
    ASSERT_NE(member6Id, nullptr);
    ets_field member7Id = env_->GetStaticp_field(cls, "member7", "D");
    ASSERT_NE(member7Id, nullptr);
    ets_field member8Id = env_->GetStaticp_field(cls, "member8", "LA;");
    ASSERT_NE(member8Id, nullptr);

    ets_class aCls = env_->FindClass("A");
    ASSERT_NE(aCls, nullptr);
    ets_field aMemberId = env_->Getp_field(aCls, "member", "I");
    ASSERT_NE(aMemberId, nullptr);
    ets_object aObj = env_->GetStaticObjectField(cls, member8Id);
    ASSERT_NE(aObj, nullptr);

    env_->SetIntField(aObj, aMemberId, static_cast<ets_int>(5_I));

    env_->SetStaticBooleanField(cls, member0Id, static_cast<ets_boolean>(1));
    env_->SetStaticByteField(cls, member1Id, static_cast<ets_byte>(10_I));
    env_->SetStaticCharField(cls, member2Id, static_cast<ets_char>(20U));
    env_->SetStaticShortField(cls, member3Id, static_cast<ets_short>(30_I));
    env_->SetStaticIntField(cls, member4Id, static_cast<ets_int>(40_I));
    env_->SetStaticLongField(cls, member5Id, static_cast<ets_long>(50L));
    env_->SetStaticFloatField(cls, member6Id, static_cast<ets_float>(60.0F));
    env_->SetStaticDoubleField(cls, member7Id, static_cast<ets_double>(70.0F));
    env_->SetStaticObjectField(cls, member8Id, aObj);

    EXPECT_EQ(env_->GetStaticBooleanField(cls, member0Id), static_cast<ets_boolean>(1));
    EXPECT_EQ(env_->GetStaticByteField(cls, member1Id), static_cast<ets_byte>(10_I));
    EXPECT_EQ(env_->GetStaticCharField(cls, member2Id), static_cast<ets_char>(20U));
    EXPECT_EQ(env_->GetStaticShortField(cls, member3Id), static_cast<ets_short>(30_I));
    EXPECT_EQ(env_->GetStaticIntField(cls, member4Id), static_cast<ets_int>(40_I));
    EXPECT_EQ(env_->GetStaticLongField(cls, member5Id), static_cast<ets_long>(50L));
    EXPECT_FLOAT_EQ(env_->GetStaticFloatField(cls, member6Id), static_cast<ets_float>(60.0F));
    EXPECT_DOUBLE_EQ(env_->GetStaticDoubleField(cls, member7Id), static_cast<ets_double>(70.0F));

    ets_object setAObj = env_->GetStaticObjectField(cls, member8Id);
    ASSERT_NE(setAObj, nullptr);
    EXPECT_EQ(env_->IsInstanceOf(setAObj, aCls), ETS_TRUE);
    EXPECT_EQ(env_->GetIntField(setAObj, aMemberId), static_cast<ets_int>(5_I));
}

}  // namespace ark::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers)
