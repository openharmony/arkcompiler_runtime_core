/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ani_gtest.h"

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class FunctionCallBooleanTest : public AniTest {
public:
    void GetMethod(ani_namespace *nsResult, ani_function *fnResult1, ani_function *fnResult2, ani_function *fnResult3)
    {
        ani_namespace ns {};
        ASSERT_EQ(env_->FindNamespace("Lops;", &ns), ANI_OK);

        ani_function fn1 {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "callLaunch", ":V", &fn1), ANI_OK);

        ani_function fn2 {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "checkCallbackDone", ":Z", &fn2), ANI_OK);

        ani_function fn3 {};
        ASSERT_EQ(env_->Namespace_FindFunction(ns, "doHeavyJob", ":V", &fn3), ANI_OK);

        *nsResult = ns;
        *fnResult1 = fn1;
        *fnResult2 = fn2;
        *fnResult3 = fn3;
    }

protected:
    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {ani_option {"--ext:coroutine-enable-features:ani-drain-queue", nullptr}};
    }
};

TEST_F(FunctionCallBooleanTest, function_create_promise)
{
    ani_namespace ns {};
    ani_function fn1 {};
    ani_function fn2 {};
    ani_function fn3 {};
    GetMethod(&ns, &fn1, &fn2, &fn3);

    ani_boolean result;
    ASSERT_EQ(env_->c_api->Function_Call_Void(env_, fn1), ANI_OK);
    ASSERT_EQ(env_->c_api->Function_Call_Void(env_, fn3), ANI_OK);
    ASSERT_EQ(env_->c_api->Function_Call_Boolean(env_, fn2, &result, ANI_TRUE, ANI_FALSE), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}
}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
