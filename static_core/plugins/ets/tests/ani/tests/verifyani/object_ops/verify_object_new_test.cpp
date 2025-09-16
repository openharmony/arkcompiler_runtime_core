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

#include "verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

class ObjectNewTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        const ani_byte bVal = 0x3f;
        const ani_short sVal = 0x1f4f;
        const ani_int iVal = 0x323fff23;
        const ani_long lVal = 0xff3229884222;
        const ani_float fVal = 0.234;
        const ani_double dVal = 1.423;

        ASSERT_EQ(env_->FindClass("verify_object_new_test.CheckCtorTypes", &cls_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(cls_, "<ctor>", "zcbsilfdC{std.core.String}:", &ctor_), ANI_OK);
        z_ = ANI_TRUE;
        c_ = 'c';
        b_ = bVal;
        s_ = sVal;
        i_ = iVal;
        l_ = lVal;
        f_ = fVal;
        d_ = dVal;
        std::string_view str("tra-ta-ta");
        ASSERT_EQ(env_->String_NewUTF8(str.data(), str.size(), &r_), ANI_OK);
    }

protected:
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    ani_class cls_ {};
    ani_method ctor_ {};

    ani_boolean z_ {};
    ani_char c_ {};
    ani_byte b_ {};
    ani_short s_ {};
    ani_int i_ {};
    ani_long l_ {};
    ani_float f_ {};
    ani_double d_ {};
    ani_string r_ {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

TEST_F(ObjectNewTest, wrong_env)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(nullptr, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_cls_0)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, nullptr, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class", "wrong reference"},
        {"ctor", "ani_method", "wrong class"},
        {"result", "ani_object *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_cls_1)
{
    ani_class cls;
    ASSERT_EQ(env_->GetNull(reinterpret_cast<ani_ref *>(&cls)), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class", "wrong reference type: ani_ref"},
        {"ctor", "ani_method", "wrong class"},
        {"result", "ani_object *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_ctor_null)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, nullptr, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "wrong ctor"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_ctor_fake)
{
    const auto fakeCtor = reinterpret_cast<ani_method>(static_cast<long>(0x0ff0f0f));  // NOLINT(google-runtime-int)
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, fakeCtor, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "wrong ctor"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

// NOTE: Enable when #25617 will be resolved
TEST_F(ObjectNewTest, DISABLED_wrong_ctor_invalidated)
{
    // Invalidate ctor
    ASSERT_EQ(env_->Reference_Delete(cls_), ANI_OK);
    ASSERT_EQ(env_->FindClass("verify_object_new_test.CheckCtorTypes", &cls_), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method", "bla-bla-bla"},
        {"result", "ani_object *"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_result)
{
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, nullptr, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *", "wrong pointer for storing 'ani_object'"},
        {"...", "       "},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_boolean)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, 0x2, c_, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean", "wrong value"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, valid_arg_boolean)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, ANI_FALSE, c_, b_, s_, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, ANI_TRUE, c_, b_, s_, i_, l_, f_, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, wrong_arg_char)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, 0xffff32, b_, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char", "wrong value"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, valid_arg_char)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, 0xffdf, b_, s_, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, 0x001f, b_, s_, i_, l_, f_, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, wrong_arg_byte_0)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, 0x1ff, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte", "wrong value"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, DISABLED_wrong_arg_byte_1)
{
    const ani_int b = -129;
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte", "wrong value"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_byte_2)
{
    const ani_int b = 128;
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b, s_, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte", "wrong value"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, valid_arg_byte)
{
    constexpr ani_byte MIN_BYTE = std::numeric_limits<ani_byte>::min();
    constexpr ani_byte MAX_BYTE = std::numeric_limits<ani_byte>::max();
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, MIN_BYTE, s_, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, 0, s_, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, MAX_BYTE, s_, i_, l_, f_, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, wrong_arg_short)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, 0x3ff64, i_, l_, f_, d_, r_), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short", "wrong value"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, valid_arg_short)
{
    constexpr ani_short MIN_SHORT = std::numeric_limits<ani_short>::min();
    constexpr ani_short MAX_SHORT = std::numeric_limits<ani_short>::max();
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, MIN_SHORT, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, 0, i_, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, MAX_SHORT, i_, l_, f_, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, valid_arg_int)
{
    constexpr ani_int MIN_INT = std::numeric_limits<ani_int>::min();
    constexpr ani_int MAX_INT = std::numeric_limits<ani_int>::max();
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, MIN_INT, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, 0, l_, f_, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, MAX_INT, l_, f_, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, valid_arg_float)
{
    constexpr ani_float MIN_FLOAT = std::numeric_limits<ani_float>::min();
    constexpr ani_float MAX_FLOAT = std::numeric_limits<ani_float>::max();
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, MIN_FLOAT, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, -0.0F, d_, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, MAX_FLOAT, d_, r_), ANI_OK);
}

TEST_F(ObjectNewTest, valid_arg_double)
{
    constexpr ani_double MIN_DOUBLE = std::numeric_limits<ani_double>::min();
    constexpr ani_double MAX_DOUBLE = std::numeric_limits<ani_double>::max();
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, MIN_DOUBLE, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, -0.0, r_), ANI_OK);
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, MAX_DOUBLE, r_), ANI_OK);
}

TEST_F(ObjectNewTest, wrong_arg_ref_null)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref", "wrong value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_arg_ref_fake)
{
    const auto fakeRef = reinterpret_cast<ani_ref>(static_cast<long>(0x0ff0f1f));  // NOLINT(google-runtime-int)
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, fakeRef), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref", "wrong value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, DISABLED_wrong_arg_ref_invalid_type)
{
    ani_object argObj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &argObj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_OK);

    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, argObj), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"cls", "ani_class"},
        {"ctor", "ani_method"},
        {"result", "ani_object *"},
        {"...", "       ", "wrong method arguments"},
        {"[0]", "ani_boolean"},
        {"[1]", "ani_char"},
        {"[2]", "ani_byte"},
        {"[3]", "ani_short"},
        {"[4]", "ani_int"},
        {"[5]", "ani_long"},
        {"[6]", "ani_float"},
        {"[7]", "ani_double"},
        {"[8]", "ani_ref", "wrong value"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, wrong_all_args)
{
    ASSERT_EQ(env_->c_api->Object_New(nullptr, nullptr, nullptr, nullptr), ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"cls", "ani_class", "wrong reference"},
        {"ctor", "ani_method", "wrong ctor"},
        {"result", "ani_object *", "wrong pointer for storing 'ani_object'"},
        {"...", "       ", "wrong method"},
    };
    // clang-format on
    ASSERT_ERROR_ANI_ARGS_MSG("Object_New", testLines);
}

TEST_F(ObjectNewTest, success)
{
    ani_object obj {};
    ASSERT_EQ(env_->c_api->Object_New(env_, cls_, ctor_, &obj, z_, c_, b_, s_, i_, l_, f_, d_, r_), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
