/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class StringGetUtf16SubstrTest : public VerifyAniTest {
public:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        uint16_t input[3];  // NOLINT(modernize-avoid-c-arrays)
        auto status = env_->String_NewUTF16(input, 3U, &string_);
        ASSERT_EQ(status, ANI_OK);
        substrOffset_ = 0;
        substrSize_ = 2U;
        bufSize_ = 5U;
        ASSERT_EQ(env_->GetUndefined(&undef_), ANI_OK);
        ASSERT_EQ(env_->GetNull(&null_), ANI_OK);

        ani_class cls {};
        ani_method ctor {};
        ASSERT_EQ(env_->FindClass("std.core.Object", &cls), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(cls, "<ctor>", ":", &ctor), ANI_OK);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Object_New(cls, ctor, &obj_), ANI_OK);

        ASSERT_EQ(env_->FindClass("escompat.Error", &cls_), ANI_OK);
        ASSERT_EQ(env_->Class_FindMethod(cls_, "<ctor>", ":", &ctor), ANI_OK);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(env_->Object_New(cls_, ctor, &err_), ANI_OK);
    }

protected:
    ani_string string_;      // NOLINT(misc-non-private-member-variables-in-classes)
    ani_size substrOffset_;  // NOLINT(misc-non-private-member-variables-in-classes)
    ani_size substrSize_;    // NOLINT(misc-non-private-member-variables-in-classes)
    uint16_t buf_[5];        // NOLINT(misc-non-private-member-variables-in-classes,modernize-avoid-c-arrays)
    ani_size bufSize_;       // NOLINT(misc-non-private-member-variables-in-classes)
    ani_size res_;           // NOLINT(misc-non-private-member-variables-in-classes)
    ani_object obj_ {};      // NOLINT(misc-non-private-member-variables-in-classes)
    ani_ref undef_ {};       // NOLINT(misc-non-private-member-variables-in-classes)
    ani_object err_ {};      // NOLINT(misc-non-private-member-variables-in-classes)
    ani_ref null_ {};        // NOLINT(misc-non-private-member-variables-in-classes)
    ani_class cls_ {};       // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(StringGetUtf16SubstrTest, wrong_env)
{
    ASSERT_EQ(
        env_->c_api->String_GetUTF16SubString(nullptr, string_, substrOffset_, substrSize_, buf_, bufSize_, &res_),
        ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"string", "ani_string"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);
}

TEST_F(StringGetUtf16SubstrTest, wrong_input_string)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16SubString(env_, nullptr, substrOffset_, substrSize_, buf_, bufSize_, &res_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *"},
        {"result", "ani_size *"},
    };
    // clang-format on

    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);
}

TEST_F(StringGetUtf16SubstrTest, null_input_string)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16SubString(env_, static_cast<ani_string>(null_), substrOffset_, substrSize_,
                                                    buf_, bufSize_, &res_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference type: null"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *"},
        {"result", "ani_size *"},
    };
    // clang-format on

    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);
}

TEST_F(StringGetUtf16SubstrTest, undef_input_string)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16SubString(env_, static_cast<ani_string>(undef_), substrOffset_, substrSize_,
                                                    buf_, bufSize_, &res_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference type: undefined"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *"},
        {"result", "ani_size *"},
    };
    // clang-format on

    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);
}

TEST_F(StringGetUtf16SubstrTest, err_input_string)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16SubString(env_, static_cast<ani_string>(err_), substrOffset_, substrSize_,
                                                    buf_, bufSize_, &res_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference type: ani_error"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *"},
        {"result", "ani_size *"},
    };
    // clang-format on

    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);
}

TEST_F(StringGetUtf16SubstrTest, obj_input_string)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16SubString(env_, static_cast<ani_string>(obj_), substrOffset_, substrSize_,
                                                    buf_, bufSize_, &res_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference type: ani_object"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *"},
        {"result", "ani_size *"},
    };
    // clang-format on

    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);
}

TEST_F(StringGetUtf16SubstrTest, cls_input_string)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16SubString(env_, reinterpret_cast<ani_string>(cls_), substrOffset_,
                                                    substrSize_, buf_, bufSize_, &res_),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string", "wrong reference type: ani_class"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *"},
        {"result", "ani_size *"},
    };
    // clang-format on

    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);
}

TEST_F(StringGetUtf16SubstrTest, wrong_res)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16SubString(env_, string_, substrOffset_, substrSize_, buf_, bufSize_, nullptr),
              ANI_ERROR);
    // clang-format off
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *"},
        {"result", "ani_size *", "wrong pointer for storing 'ani_size'"},
    };
    // clang-format on

    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);
}

TEST_F(StringGetUtf16SubstrTest, wrong_buf)
{
    ASSERT_EQ(
        env_->c_api->String_GetUTF16SubString(env_, string_, substrOffset_, substrSize_, nullptr, bufSize_, &res_),
        ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"string", "ani_string"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *", "wrong pointer to use as argument in 'uint16_t *'"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);
}

TEST_F(StringGetUtf16SubstrTest, wrong_all_args)
{
    ASSERT_EQ(
        env_->c_api->String_GetUTF16SubString(nullptr, nullptr, substrOffset_, substrSize_, nullptr, bufSize_, nullptr),
        ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "called from incorrect the native scope"},
        {"string", "ani_string", "wrong reference"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *", "wrong pointer to use as argument in 'uint16_t *'"},
        {"result", "ani_size *", "wrong pointer for storing 'ani_size'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);
}

TEST_F(StringGetUtf16SubstrTest, throw_error)
{
    ThrowError();
    ASSERT_EQ(env_->c_api->String_GetUTF16SubString(env_, string_, substrOffset_, substrSize_, buf_, bufSize_, &res_),
              ANI_ERROR);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *", "has unhandled an error"},
        {"string", "ani_string"},
        {"substrOffset", "ani_size"},
        {"substrSize", "ani_size"},
        {"utf16Buffer", "uint16_t *"},
        {"result", "ani_size *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("String_GetUTF16SubString", testLines);

    ASSERT_EQ(env_->ResetError(), ANI_OK);
}

TEST_F(StringGetUtf16SubstrTest, success)
{
    ASSERT_EQ(env_->c_api->String_GetUTF16SubString(env_, string_, substrOffset_, substrSize_, buf_, bufSize_, &res_),
              ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
