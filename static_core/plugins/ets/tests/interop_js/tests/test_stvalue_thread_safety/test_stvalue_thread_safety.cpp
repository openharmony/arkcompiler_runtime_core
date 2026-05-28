/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/interop_js/st_value/ets_vm_STValue.h"

namespace ark::ets::interop::js::testing {

class STValueThreadSafetyTest : public EtsInteropTest {
protected:
    bool TestIdempotency()
    {
        napi_env env = GetJsEnv();
        napi_value result1 = GetSTValueClass(env);
        if (result1 == nullptr) {
            return false;
        }

        napi_value result2 = GetSTValueClass(env);
        if (result2 == nullptr) {
            return false;
        }

        bool isSame = false;
        napi_status status = napi_strict_equals(env, result1, result2, &isSame);
        if (status != napi_ok) {
            return false;
        }
        if (isSame) {
            return false;
        }

        return true;
    }

    bool TestEdgeCases()
    {
        napi_env env = GetJsEnv();

        auto result1 = GetSTValueClass(env);
        auto result2 = GetSTValueClass(env);
        auto result3 = GetSTValueClass(env);

        bool isSame12 = false;
        bool isSame23 = false;
        bool isSame13 = false;
        auto status1 = napi_strict_equals(env, result1, result2, &isSame12);
        auto status2 = napi_strict_equals(env, result2, result3, &isSame23);
        auto status3 = napi_strict_equals(env, result1, result3, &isSame13);
        if (status1 != napi_ok || status2 != napi_ok || status3 != napi_ok || isSame12 || isSame23 || isSame13) {
            return false;
        }

        napi_value result4 = GetSTValueClass(env);
        bool isSame14 = false;
        auto status4 = napi_strict_equals(env, result1, result4, &isSame14);
        return status4 == napi_ok && !isSame14;
    }
};

TEST_F(STValueThreadSafetyTest, IdempotencyTest)
{
    bool success = TestIdempotency();
    EXPECT_TRUE(success) << "Idempotency test failed!";
}

TEST_F(STValueThreadSafetyTest, EdgeCasesTest)
{
    bool success = TestEdgeCases();
    EXPECT_TRUE(success) << "Edge cases test failed!";
}

}  // namespace ark::ets::interop::js::testing
