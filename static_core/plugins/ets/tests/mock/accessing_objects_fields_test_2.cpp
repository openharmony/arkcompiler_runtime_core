/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

namespace panda::ets::test {

static const char *TEST_BIN_FILE_NAME = "AccessingObjectsFieldsTest.abc";

class AccessingObjectsFieldsTestGeneral : public AccessingObjectsFieldsTestBase {
public:
    AccessingObjectsFieldsTestGeneral() : AccessingObjectsFieldsTestBase(TEST_BIN_FILE_NAME) {}
};

class AccessingObjectsFieldsTest : public AccessingObjectsFieldsTestGeneral {};
class AccessingObjectsFieldsTestDeath : public AccessingObjectsFieldsTestGeneral {};

TEST_F(AccessingObjectsFieldsTestDeath, SetTypeFieldDeathTests)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    {
        EXPECT_DEATH(env_->SetObjectField(nullptr, nullptr, nullptr), "");

        ets_class a_cls = env_->FindClass("A");
        ASSERT_NE(a_cls, nullptr);
        ets_method a_ctor = env_->Getp_method(a_cls, "<ctor>", ":V");
        ASSERT_NE(a_ctor, nullptr);
        ets_object a_obj = env_->NewObject(a_cls, a_ctor);
        ASSERT_NE(a_obj, nullptr);

        EXPECT_DEATH(env_->SetObjectField(nullptr, nullptr, a_obj), "");
        EXPECT_DEATH(env_->SetBooleanField(nullptr, nullptr, static_cast<ets_boolean>(0)), "");
        EXPECT_DEATH(env_->SetByteField(nullptr, nullptr, static_cast<ets_byte>(0)), "");
        EXPECT_DEATH(env_->SetCharField(nullptr, nullptr, static_cast<ets_char>(0)), "");
        EXPECT_DEATH(env_->SetShortField(nullptr, nullptr, static_cast<ets_short>(0)), "");
        EXPECT_DEATH(env_->SetIntField(nullptr, nullptr, static_cast<ets_int>(0)), "");
        EXPECT_DEATH(env_->SetLongField(nullptr, nullptr, static_cast<ets_long>(0)), "");
        EXPECT_DEATH(env_->SetFloatField(nullptr, nullptr, static_cast<ets_float>(0.0F)), "");
        EXPECT_DEATH(env_->SetDoubleField(nullptr, nullptr, static_cast<ets_double>(0.0)), "");
    }

    {
        ets_class cls = env_->FindClass("F");
        ASSERT_NE(cls, nullptr);
        ets_field member0_id = env_->Getp_field(cls, "member0", "Z");
        ASSERT_NE(member0_id, nullptr);
        ets_field member1_id = env_->Getp_field(cls, "member1", "B");
        ASSERT_NE(member1_id, nullptr);
        ets_field member2_id = env_->Getp_field(cls, "member2", "C");
        ASSERT_NE(member2_id, nullptr);
        ets_field member3_id = env_->Getp_field(cls, "member3", "S");
        ASSERT_NE(member3_id, nullptr);
        ets_field member4_id = env_->Getp_field(cls, "member4", "I");
        ASSERT_NE(member4_id, nullptr);
        ets_field member5_id = env_->Getp_field(cls, "member5", "J");
        ASSERT_NE(member5_id, nullptr);
        ets_field member6_id = env_->Getp_field(cls, "member6", "F");
        ASSERT_NE(member6_id, nullptr);
        ets_field member7_id = env_->Getp_field(cls, "member7", "D");
        ASSERT_NE(member7_id, nullptr);
        ets_field member8_id = env_->Getp_field(cls, "member8", "LA;");
        ASSERT_NE(member8_id, nullptr);

        EXPECT_DEATH(env_->SetObjectField(nullptr, member8_id, nullptr), "");

        ets_class a_cls = env_->FindClass("A");
        ASSERT_NE(a_cls, nullptr);
        ets_method a_ctor = env_->Getp_method(a_cls, "<ctor>", ":V");
        ASSERT_NE(a_ctor, nullptr);
        ets_object a_obj = env_->NewObject(a_cls, a_ctor);
        ASSERT_NE(a_obj, nullptr);

        EXPECT_DEATH(env_->SetObjectField(nullptr, member8_id, a_obj), "");
        EXPECT_DEATH(env_->SetBooleanField(nullptr, member0_id, static_cast<ets_boolean>(0)), "");
        EXPECT_DEATH(env_->SetByteField(nullptr, member1_id, static_cast<ets_byte>(0)), "");
        EXPECT_DEATH(env_->SetCharField(nullptr, member2_id, static_cast<ets_char>(0)), "");
        EXPECT_DEATH(env_->SetShortField(nullptr, member3_id, static_cast<ets_short>(0)), "");
        EXPECT_DEATH(env_->SetIntField(nullptr, member4_id, static_cast<ets_int>(0)), "");
        EXPECT_DEATH(env_->SetLongField(nullptr, member5_id, static_cast<ets_long>(0)), "");
        EXPECT_DEATH(env_->SetFloatField(nullptr, member6_id, static_cast<ets_float>(0.0F)), "");
        EXPECT_DEATH(env_->SetDoubleField(nullptr, member7_id, static_cast<ets_double>(0.0)), "");
    }

    {
        ets_class cls = env_->FindClass("F");
        ASSERT_NE(cls, nullptr);
        ets_method ctor = env_->Getp_method(cls, "<ctor>", ":V");
        ASSERT_NE(ctor, nullptr);
        ets_object obj = env_->NewObject(cls, ctor);
        ASSERT_NE(obj, nullptr);

        EXPECT_DEATH(env_->SetObjectField(obj, nullptr, nullptr), "");

        ets_class a_cls = env_->FindClass("F");
        ASSERT_NE(a_cls, nullptr);
        ets_method a_ctor = env_->Getp_method(a_cls, "<ctor>", ":V");
        ASSERT_NE(a_ctor, nullptr);
        ets_object a_obj = env_->NewObject(a_cls, a_ctor);
        ASSERT_NE(a_obj, nullptr);

        EXPECT_DEATH(env_->SetObjectField(obj, nullptr, a_obj), "");
        EXPECT_DEATH(env_->SetBooleanField(obj, nullptr, static_cast<ets_boolean>(0)), "");
        EXPECT_DEATH(env_->SetByteField(obj, nullptr, static_cast<ets_byte>(0)), "");
        EXPECT_DEATH(env_->SetCharField(obj, nullptr, static_cast<ets_char>(0)), "");
        EXPECT_DEATH(env_->SetShortField(obj, nullptr, static_cast<ets_short>(0)), "");
        EXPECT_DEATH(env_->SetIntField(obj, nullptr, static_cast<ets_int>(0)), "");
        EXPECT_DEATH(env_->SetLongField(obj, nullptr, static_cast<ets_long>(0)), "");
        EXPECT_DEATH(env_->SetFloatField(obj, nullptr, static_cast<ets_float>(0.0F)), "");
        EXPECT_DEATH(env_->SetDoubleField(obj, nullptr, static_cast<ets_double>(0.0)), "");
    }
}

}  // namespace panda::ets::test

// NOLINTEND(cppcoreguidelines-pro-type-vararg)