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

class GlobalReferenceDeleteTest : public VerifyAniTest {};

TEST_F(GlobalReferenceDeleteTest, undefined_ref)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    ASSERT_EQ(env_->c_api->GlobalReference_Delete(env_, ref), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();

    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

TEST_F(GlobalReferenceDeleteTest, wrong_ref_kind)
{
    ani_string str {};
    ASSERT_EQ(env_->String_NewUTF8("x", 1, &str), ANI_OK);
    auto ref = reinterpret_cast<ani_ref>(str);

    ASSERT_EQ(env_->c_api->GlobalReference_Delete(env_, ref), ANI_INCORRECT_REF);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"gref", "ani_ref", "wrong reference type: local reference, expected: global reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GlobalReference_Delete", testLines);

    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

TEST_F(GlobalReferenceDeleteTest, wrong_null_ref)
{
    ASSERT_EQ(env_->c_api->GlobalReference_Delete(env_, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"gref", "ani_ref", "reference is null, expected: global reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("GlobalReference_Delete", testLines);
}

TEST_F(GlobalReferenceDeleteTest, success_with_pending_error)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    ani_ref gref {};
    ASSERT_EQ(env_->GlobalReference_Create(ref, &gref), ANI_OK);
    ThrowError();
    ASSERT_EQ(env_->c_api->GlobalReference_Delete(env_, gref), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
