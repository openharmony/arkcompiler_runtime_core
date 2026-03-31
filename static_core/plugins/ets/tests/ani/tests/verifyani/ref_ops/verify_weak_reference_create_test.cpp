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

class WeakReferenceCreateTest : public VerifyAniTest {};

TEST_F(WeakReferenceCreateTest, wrong_ref)
{
    ani_wref wref {};
    ASSERT_EQ(env_->c_api->WeakReference_Create(env_, nullptr, &wref), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref", "wrong reference"},
        {"result", "ani_wref *"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("WeakReference_Create", testLines);
}

TEST_F(WeakReferenceCreateTest, wrong_result)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    ASSERT_EQ(env_->c_api->WeakReference_Create(env_, ref, nullptr), ANI_INVALID_ARGS);
    std::vector<TestLineInfo> testLines {
        {"env", "ani_env *"},
        {"ref", "ani_ref"},
        {"result", "ani_wref *", "wrong pointer for storing 'ani_wref'"},
    };
    ASSERT_ERROR_ANI_ARGS_MSG("WeakReference_Create", testLines);

    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
