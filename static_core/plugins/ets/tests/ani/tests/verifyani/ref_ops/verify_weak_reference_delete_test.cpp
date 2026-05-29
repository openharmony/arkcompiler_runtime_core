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

class WeakReferenceDeleteTest : public VerifyAniTest {};

TEST_F(WeakReferenceDeleteTest, wrong_wref)
{
    ASSERT_EQ(env_->c_api->WeakReference_Delete(env_, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"wref", "ani_wref", "reference is null, expected: weak reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("WeakReference_Delete", testLines);
}

TEST_F(WeakReferenceDeleteTest, wrong_wref_kind)
{
    ani_string str {};
    ASSERT_EQ(env_->String_NewUTF8("x", 1, &str), ANI_OK);
    auto ref = reinterpret_cast<ani_ref>(str);

    ASSERT_EQ(env_->c_api->WeakReference_Delete(env_, reinterpret_cast<ani_wref>(ref)), ANI_INCORRECT_REF);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"wref", "ani_wref", "wrong weak reference"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("WeakReference_Delete", testLines);

    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

TEST_F(WeakReferenceDeleteTest, success_with_pending_error)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(ref, &wref), ANI_OK);
    ThrowError();
    ASSERT_EQ(env_->c_api->WeakReference_Delete(env_, wref), ANI_OK);
    ASSERT_EQ(env_->ResetError(), ANI_OK);
    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
