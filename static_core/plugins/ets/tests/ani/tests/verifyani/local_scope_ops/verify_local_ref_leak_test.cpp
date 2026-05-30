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

#include "plugins/ets/tests/ani/ani_gtest/verify_ani_gtest.h"

namespace ark::ets::ani::verify::testing {

class LocalRefNoScopeTest : public VerifyAniTest {
protected:
    void SetUp() override
    {
        VerifyAniTest::SetUp();

        pandaEnv_ = EtsExecutionContext::GetCurrent()->GetPandaAniEnv();
        envVerifier_ = pandaEnv_->GetEnvANIVerifier();
    }

    void PopTestNativeFrame()
    {
        auto err = envVerifier_->PopNativeFrame();
        ASSERT_FALSE(err.has_value()) << err.value();
        env_ = envVerifier_->GetEnv();
    }

    void PushTestNativeFrame()
    {
        envVerifier_->PushNativeFrame(pandaEnv_);
        env_ = envVerifier_->GetEnv();
    }

    EnvANIVerifier *envVerifier_ {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
    PandaAniEnv *pandaEnv_ {nullptr};        // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(LocalRefNoScopeTest, ref_created_outside_native_scope)
{
    PopTestNativeFrame();

    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);

    std::string msg = GetAndClearAbortMessage();
    ASSERT_THAT(msg, ::testing::HasSubstr("outside of any native scope"));
    ASSERT_EQ(env_->Reference_Delete(ref), ANI_OK);

    PushTestNativeFrame();
}

TEST_F(LocalRefNoScopeTest, ref_inside_native_scope_no_error)
{
    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(LocalRefNoScopeTest, ref_inside_explicit_local_scope_no_error)
{
    constexpr ani_size SCOPE_CAPACITY = 16;
    ASSERT_EQ(env_->CreateLocalScope(SCOPE_CAPACITY), ANI_OK);

    ani_ref ref {};
    ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

class LocalScopeCapacityTest : public VerifyAniTest {};

TEST_F(LocalScopeCapacityTest, overflow_explicit_local_scope)
{
    constexpr ani_size SCOPE_CAPACITY = 2;
    ASSERT_EQ(env_->CreateLocalScope(SCOPE_CAPACITY), ANI_OK);

    ani_ref ref1 {};
    ASSERT_EQ(env_->GetUndefined(&ref1), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();

    ani_ref ref2 {};
    ASSERT_EQ(env_->GetUndefined(&ref2), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();

    ani_ref ref3 {};
    ASSERT_EQ(env_->GetUndefined(&ref3), ANI_OK);

    std::string msg = GetAndClearAbortMessage();
    ASSERT_THAT(msg, ::testing::HasSubstr("capacity " + std::to_string(SCOPE_CAPACITY)));

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(LocalScopeCapacityTest, within_capacity_no_error)
{
    constexpr ani_size SCOPE_CAPACITY = 10;
    ASSERT_EQ(env_->CreateLocalScope(SCOPE_CAPACITY), ANI_OK);

    for (ani_size i = 0; i < SCOPE_CAPACITY; ++i) {
        ani_ref ref {};
        ASSERT_EQ(env_->GetUndefined(&ref), ANI_OK);
    }
    ASSERT_NO_ABORT_MESSAGE();

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(LocalScopeCapacityTest, overflow_escape_local_scope)
{
    constexpr ani_size SCOPE_CAPACITY = 1;
    ASSERT_EQ(env_->CreateEscapeLocalScope(SCOPE_CAPACITY), ANI_OK);

    ani_ref ref1 {};
    ASSERT_EQ(env_->GetUndefined(&ref1), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();

    ani_ref ref2 {};
    ASSERT_EQ(env_->GetUndefined(&ref2), ANI_OK);

    std::string msg = GetAndClearAbortMessage();
    ASSERT_THAT(msg, ::testing::HasSubstr("capacity " + std::to_string(SCOPE_CAPACITY)));

    ASSERT_EQ(env_->DestroyEscapeLocalScope(ref1, &ref1), ANI_OK);
}

TEST_F(LocalScopeCapacityTest, delete_ref_frees_capacity)
{
    constexpr ani_size SCOPE_CAPACITY = 2;
    ASSERT_EQ(env_->CreateLocalScope(SCOPE_CAPACITY), ANI_OK);

    ani_ref ref1 {};
    ASSERT_EQ(env_->GetUndefined(&ref1), ANI_OK);

    ani_ref ref2 {};
    ASSERT_EQ(env_->GetUndefined(&ref2), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();

    ASSERT_EQ(env_->Reference_Delete(ref1), ANI_OK);

    ani_ref ref3 {};
    ASSERT_EQ(env_->GetUndefined(&ref3), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

TEST_F(LocalScopeCapacityTest, nested_scopes_independent_capacity)
{
    constexpr ani_size OUTER_CAPACITY = 2;
    constexpr ani_size INNER_CAPACITY = 1;

    ASSERT_EQ(env_->CreateLocalScope(OUTER_CAPACITY), ANI_OK);

    ani_ref outerRef {};
    ASSERT_EQ(env_->GetUndefined(&outerRef), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();

    ASSERT_EQ(env_->CreateLocalScope(INNER_CAPACITY), ANI_OK);

    ani_ref innerRef1 {};
    ASSERT_EQ(env_->GetUndefined(&innerRef1), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();

    ani_ref innerRef2 {};
    ASSERT_EQ(env_->GetUndefined(&innerRef2), ANI_OK);
    std::string msg = GetAndClearAbortMessage();
    ASSERT_THAT(msg, ::testing::HasSubstr("capacity " + std::to_string(INNER_CAPACITY)));

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);

    ani_ref outerRef2 {};
    ASSERT_EQ(env_->GetUndefined(&outerRef2), ANI_OK);
    ASSERT_NO_ABORT_MESSAGE();

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
}

}  // namespace ark::ets::ani::verify::testing
