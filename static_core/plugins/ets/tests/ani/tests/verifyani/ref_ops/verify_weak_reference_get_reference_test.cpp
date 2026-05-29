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

class WeakReferenceGetReferenceTest : public VerifyAniTest {};

TEST_F(WeakReferenceGetReferenceTest, wrong_wref)
{
    ani_boolean wasReleased = ANI_FALSE;
    ani_ref ref {};

    ASSERT_EQ(env_->c_api->WeakReference_GetReference(env_, nullptr, &wasReleased, &ref), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"wref", "ani_wref", "wrong weak reference"},
        {"was_released_result", "ani_boolean *"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("WeakReference_GetReference", testLines);
}

TEST_F(WeakReferenceGetReferenceTest, wrong_wref_kind)
{
    ani_string str {};
    ASSERT_EQ(env_->String_NewUTF8("x", 1, &str), ANI_OK);
    auto ref = reinterpret_cast<ani_ref>(str);

    ani_boolean wasReleased = ANI_FALSE;
    ani_ref outRef {};
    ASSERT_EQ(env_->c_api->WeakReference_GetReference(env_, reinterpret_cast<ani_wref>(ref), &wasReleased, &outRef),
              ANI_INCORRECT_REF);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"wref", "ani_wref", "wrong weak reference"},
        {"was_released_result", "ani_boolean *"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("WeakReference_GetReference", testLines);

    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

TEST_F(WeakReferenceGetReferenceTest, wrong_was_released_result)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(ref, &wref), ANI_OK);

    ani_ref outRef {};
    ASSERT_EQ(env_->c_api->WeakReference_GetReference(env_, wref, nullptr, &outRef), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"wref", "ani_wref"},
        {"was_released_result", "ani_boolean *", "nullptr for storing 'ani_boolean'"},
        {"result", "ani_ref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("WeakReference_GetReference", testLines);

    ASSERT_EQ(env_->WeakReference_Delete(wref), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

TEST_F(WeakReferenceGetReferenceTest, wrong_result)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(ref, &wref), ANI_OK);

    ani_boolean wasReleased = ANI_FALSE;
    ASSERT_EQ(env_->c_api->WeakReference_GetReference(env_, wref, &wasReleased, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"wref", "ani_wref"},
        {"was_released_result", "ani_boolean *"},
        {"result", "ani_ref *", "nullptr for storing 'ani_ref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("WeakReference_GetReference", testLines);

    ASSERT_EQ(env_->WeakReference_Delete(wref), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

TEST_F(WeakReferenceGetReferenceTest, success)
{
    ani_string str {};
    ASSERT_EQ(env_->String_NewUTF8("x", 1, &str), ANI_OK);
    auto ref = reinterpret_cast<ani_ref>(str);

    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(ref, &wref), ANI_OK);

    ani_boolean wasReleased = ANI_TRUE;
    ani_ref outRef {};
    ASSERT_EQ(env_->c_api->WeakReference_GetReference(env_, wref, &wasReleased, &outRef), ANI_OK);
    ASSERT_EQ(wasReleased, ANI_FALSE);

    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(ref, outRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);

    ASSERT_EQ(env_->Reference_Delete(outRef), ANI_OK);
    ASSERT_EQ(env_->WeakReference_Delete(wref), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
