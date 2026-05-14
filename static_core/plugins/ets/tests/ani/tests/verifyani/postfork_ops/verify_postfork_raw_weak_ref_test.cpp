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

#include "plugins/ets/runtime/ani/verify/ani_verifier.h"
#include "plugins/ets/runtime/ets_ani_env.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets::ani::verify::testing {

namespace {

void LateInitializeVerifyAni(ani_env *env)
{
    auto *pandaEnv = PandaAniEnv::FromAniEnv(env);
    auto *vm = pandaEnv->GetEtsVM();
    ASSERT_FALSE(vm->IsVerifyANI());
    vm->CreateANIVerifier(true);
    pandaEnv->CreateEnvANIVerifier();
    ASSERT_TRUE(vm->IsVerifyANI());
}

}  // namespace

class PostforkRawWeakRefTest : public ani::testing::AniTest {
public:
    void SetUp() override
    {
        AniTest::SetUp();

        auto *pandaEnv = PandaAniEnv::FromAniEnv(env_);
        ASSERT_FALSE(pandaEnv->GetEtsVM()->IsVerifyANI());

        ani_string str {};
        ASSERT_EQ(env_->String_NewUTF8("x", 1, &str), ANI_OK);
        auto localRef = reinterpret_cast<ani_ref>(str);
        ASSERT_EQ(env_->GlobalReference_Create(localRef, &objectRef_), ANI_OK);
        ASSERT_EQ(env_->Reference_Delete(localRef), ANI_OK);

        ASSERT_EQ(env_->WeakReference_Create(objectRef_, &rawWeakRef_), ANI_OK);
        ASSERT_NE(rawWeakRef_, nullptr);

        LateInitializeVerifyAni(env_);

        ani_env *verifyEnv = nullptr;
        ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &verifyEnv), ANI_OK);
        env_ = verifyEnv;

        auto *verifier = pandaEnv->GetEtsVM()->GetANIVerifier();
        ASSERT_NE(verifier, nullptr);
        verifier->SetAbortHook(AbortHook, this);
        verifier->SetErrorHook(AbortHook, this);

        pandaEnv->GetEnvANIVerifier()->PushNativeFrame(pandaEnv);
        env_ = pandaEnv->GetEnvANIVerifier()->GetEnv();
    }

    void TearDown() override
    {
        if (PandaEtsVM::GetCurrent() != nullptr && PandaEtsVM::GetCurrent()->GetANIVerifier() != nullptr) {
            auto *pandaEnv = PandaAniEnv::FromAniEnv(env_);
            auto popErr = pandaEnv->GetEnvANIVerifier()->PopNativeFrame();
            ASSERT_FALSE(popErr.has_value()) << "Leaked scope detected in test: " << popErr.value();
            env_ = pandaEnv->GetEnvANIVerifier()->GetEnv();

            PandaEtsVM::GetCurrent()->GetANIVerifier()->SetAbortHook(nullptr, nullptr);
            PandaEtsVM::GetCurrent()->GetANIVerifier()->SetErrorHook(nullptr, nullptr);
        }
        if (rawWeakRef_ != nullptr) {
            ASSERT_EQ(env_->WeakReference_Delete(rawWeakRef_), ANI_OK);
            rawWeakRef_ = nullptr;
        }
        if (objectRef_ != nullptr) {
            ASSERT_EQ(env_->GlobalReference_Delete(objectRef_), ANI_OK);
            objectRef_ = nullptr;
        }
        AniTest::TearDown();
        ASSERT_NO_ABORT_MESSAGE();
        ss_.str("");
    }

    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {};
    }

    std::string GetAndClearAbortMessage()
    {
        std::string msg = ss_.str();
        ss_.str("");
        return msg;
    }

    [[nodiscard]] ani_ref ObjectRef() const
    {
        return objectRef_;
    }

    [[nodiscard]] ani_wref RawWeakRef() const
    {
        return rawWeakRef_;
    }

private:
    static void AbortHook(void *data, const std::string_view message)
    {
        auto *self = reinterpret_cast<PostforkRawWeakRefTest *>(data);
        self->ss_ << message;
    }

    std::stringstream ss_;
    ani_ref objectRef_ {nullptr};
    ani_wref rawWeakRef_ {nullptr};
};

TEST_F(PostforkRawWeakRefTest, weak_reference_get_reference_with_raw_weak_after_late_init)
{
    ani_boolean wasReleased = ANI_TRUE;
    ani_ref outRef {};
    ASSERT_EQ(env_->WeakReference_GetReference(RawWeakRef(), &wasReleased, &outRef), ANI_OK);
    ASSERT_EQ(wasReleased, ANI_FALSE);

    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(ObjectRef(), outRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);

    ASSERT_EQ(env_->Reference_Delete(outRef), ANI_OK);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawWeakRefTest, weak_reference_create_from_object_after_late_init)
{
    ani_wref wref2 {};
    ASSERT_EQ(env_->WeakReference_Create(ObjectRef(), &wref2), ANI_OK);
    ASSERT_EQ(env_->WeakReference_Delete(wref2), ANI_OK);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawWeakRefTest, weak_reference_delete_null_returns_invalid_args)
{
    ASSERT_EQ(env_->c_api->WeakReference_Delete(env_, nullptr), ANI_INVALID_ARGS);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawWeakRefTest, reference_equals_raw_weak_object_with_global)
{
    ani_boolean wasReleased = ANI_FALSE;
    ani_ref outRef {};
    ASSERT_EQ(env_->WeakReference_GetReference(RawWeakRef(), &wasReleased, &outRef), ANI_OK);

    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(ObjectRef(), outRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);

    ASSERT_EQ(env_->Reference_Delete(outRef), ANI_OK);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

}  // namespace ark::ets::ani::verify::testing
