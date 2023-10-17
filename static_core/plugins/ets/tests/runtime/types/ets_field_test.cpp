/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "get_test_class.h"
#include "ets_coroutine.h"

#include "types/ets_field.h"
#include "types/ets_string.h"
#include "types/ets_class.h"

namespace panda::ets::test {

class EtsFieldTest : public testing::Test {
public:
    EtsFieldTest()
    {
        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetCompilerEnableJit(false);
        options_.SetGcType("epsilon");
        options_.SetLoadRuntimes({"ets"});

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options_.SetBootPandaFiles({stdlib});

        Runtime::Create(options_);
    }

    ~EtsFieldTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EtsFieldTest);
    NO_MOVE_SEMANTIC(EtsFieldTest);

    void SetUp() override
    {
        coroutine_ = EtsCoroutine::GetCurrent();
        coroutine_->ManagedCodeBegin();
    }

    void TearDown() override
    {
        coroutine_->ManagedCodeEnd();
    }

private:
    RuntimeOptions options_;
    EtsCoroutine *coroutine_ = nullptr;
};

static void TestFieldMethods(const char *source, const char *class_name, const char *field_name,
                             const char *field_type_descriptor_name, bool is_static_field)
{
    EtsClass *klass = GetTestClass(source, class_name);
    ASSERT_NE(klass, nullptr);

    EtsString *field_name_string = EtsString::CreateFromMUtf8(field_name);
    EtsField *field1 = nullptr;
    EtsField *field2 = nullptr;

    if (is_static_field) {
        field1 = klass->GetStaticFieldIDByName(field_name);
        field2 = klass->GetStaticFieldIDByOffset(field1->GetOffset());
    } else {
        field1 = klass->GetDeclaredFieldIDByName(field_name);
        field2 = klass->GetFieldIDByOffset(field1->GetOffset());
    }

    ASSERT_EQ(field1, field2);
    ASSERT_TRUE(field_name_string->StringsAreEqual(reinterpret_cast<EtsObject *>(field1->GetNameString())));
    ASSERT_TRUE(!strcmp(field1->GetName(), field_name));
    ASSERT_TRUE(!strcmp(field1->GetTypeDescriptor(), field_type_descriptor_name));
    ASSERT_EQ(field1->IsStatic(), is_static_field);
    ASSERT_EQ(klass, field1->GetDeclaringClass());
}

TEST_F(EtsFieldTest, AllMethods)
{
    const char *source = R"(
        .language eTS
        .record Ball {
	        f32 BALL_RADIUS <static>
	        i32 BALL_SPEED <static>
            f32 vx
            i32 vy
        }
    )";

    TestFieldMethods(source, "LBall;", "BALL_RADIUS", "F", true);
    TestFieldMethods(source, "LBall;", "BALL_SPEED", "I", true);
    TestFieldMethods(source, "LBall;", "vx", "F", false);
    TestFieldMethods(source, "LBall;", "vy", "I", false);

    source = R"(
        .language eTS
        .record Drawer {
            i32 frameWidth
            i32 frameHeight
            i64 startTime <static>
            i32 fps
            any drawCounter
        }
    )";

    TestFieldMethods(source, "LDrawer;", "frameWidth", "I", false);
    TestFieldMethods(source, "LDrawer;", "frameHeight", "I", false);
    TestFieldMethods(source, "LDrawer;", "startTime", "J", true);
    TestFieldMethods(source, "LDrawer;", "fps", "I", false);
    TestFieldMethods(source, "LDrawer;", "drawCounter", "A", false);
}

}  // namespace panda::ets::test
