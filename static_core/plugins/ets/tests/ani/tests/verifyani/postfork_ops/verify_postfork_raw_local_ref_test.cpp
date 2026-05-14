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

void EnableVerifyHooks(ani_env *env, void *testInstance, void (*hook)(void *data, const std::string_view message))
{
    auto *pandaEnv = PandaAniEnv::FromAniEnv(env);
    auto *verifier = pandaEnv->GetEtsVM()->GetANIVerifier();
    ASSERT_NE(verifier, nullptr);
    verifier->SetAbortHook(hook, testInstance);
    verifier->SetErrorHook(hook, testInstance);
    pandaEnv->GetEnvANIVerifier()->PushNativeFrame(pandaEnv);
}

void DisableVerifyHooks(ani_env *env)
{
    if (PandaEtsVM::GetCurrent() == nullptr || PandaEtsVM::GetCurrent()->GetANIVerifier() == nullptr) {
        return;
    }
    auto *pandaEnv = PandaAniEnv::FromAniEnv(env);
    auto popErr = pandaEnv->GetEnvANIVerifier()->PopNativeFrame();
    ASSERT_FALSE(popErr.has_value()) << "Leaked scope detected in test: " << popErr.value();
    PandaEtsVM::GetCurrent()->GetANIVerifier()->SetAbortHook(nullptr, nullptr);
    PandaEtsVM::GetCurrent()->GetANIVerifier()->SetErrorHook(nullptr, nullptr);
}

}  // namespace

class PostforkRawLocalRefTest : public ani::testing::AniTest {
public:
    void SetUp() override
    {
        AniTest::SetUp();

        auto *pandaEnv = PandaAniEnv::FromAniEnv(env_);
        ASSERT_FALSE(pandaEnv->GetEtsVM()->IsVerifyANI());

        ani_string str {};
        ASSERT_EQ(env_->String_NewUTF8(PREFORK_LOCAL_STRING.data(), PREFORK_LOCAL_STRING.size(), &str), ANI_OK);
        rawLocalRef_ = reinterpret_cast<ani_ref>(str);

        ASSERT_EQ(env_->GlobalReference_Create(rawLocalRef_, &globalRef_), ANI_OK);

        nonVerifyEnv_ = env_;

        LateInitializeVerifyAni(env_);

        ani_env *verifyEnv = nullptr;
        ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &verifyEnv), ANI_OK);
        env_ = verifyEnv;

        EnableVerifyHooks(env_, this, AbortHook);
        env_ = PandaAniEnv::FromAniEnv(env_)->GetEnvANIVerifier()->GetEnv();
    }

    void TearDown() override
    {
        DisableVerifyHooks(env_);
        // Prefork refs were created before VerifyANI; clean them up via the original env.
        if (rawLocalRef_ != nullptr) {
            ASSERT_EQ(nonVerifyEnv_->Reference_Delete(rawLocalRef_), ANI_OK);
            rawLocalRef_ = nullptr;
        }
        if (globalRef_ != nullptr) {
            ASSERT_EQ(nonVerifyEnv_->GlobalReference_Delete(globalRef_), ANI_OK);
            globalRef_ = nullptr;
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

    [[nodiscard]] ani_ref RawLocalRef() const
    {
        return rawLocalRef_;
    }

    [[nodiscard]] ani_ref GlobalRef() const
    {
        return globalRef_;
    }

    void ClearGlobalRef()
    {
        globalRef_ = nullptr;
    }

    void ClearRawLocalRef()
    {
        rawLocalRef_ = nullptr;
    }

protected:
    static constexpr std::string_view PREFORK_LOCAL_STRING = "local";
    static constexpr std::string_view POSTFORK_VERIFY_STRING = "post";
    static constexpr ani_size ESCAPE_SCOPE_CAPACITY = 2;
    static constexpr ani_size LOCAL_SCOPE_CAPACITY = 4;

private:
    static void AbortHook(void *data, const std::string_view message)
    {
        auto *self = reinterpret_cast<PostforkRawLocalRefTest *>(data);
        self->ss_ << message;
    }

    std::stringstream ss_;
    ani_ref rawLocalRef_ {nullptr};
    ani_ref globalRef_ {nullptr};
    ani_env *nonVerifyEnv_ {nullptr};
};

TEST_F(PostforkRawLocalRefTest, reference_is_null_with_prefork_raw_local)
{
    ani_boolean isNull = ANI_TRUE;
    ASSERT_EQ(env_->Reference_IsNull(RawLocalRef(), &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_FALSE);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, reference_is_undefined_with_prefork_raw_local)
{
    ani_boolean isUndefined = ANI_TRUE;
    ASSERT_EQ(env_->Reference_IsUndefined(RawLocalRef(), &isUndefined), ANI_OK);
    ASSERT_EQ(isUndefined, ANI_FALSE);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, reference_is_nullish_value_with_prefork_raw_local)
{
    ani_boolean isNullish = ANI_TRUE;
    ASSERT_EQ(env_->Reference_IsNullishValue(RawLocalRef(), &isNullish), ANI_OK);
    ASSERT_EQ(isNullish, ANI_FALSE);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, reference_equals_raw_local_with_self)
{
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(RawLocalRef(), RawLocalRef(), &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, reference_equals_raw_local_with_global)
{
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(RawLocalRef(), GlobalRef(), &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, reference_strict_equals_raw_local_with_self)
{
    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_StrictEquals(RawLocalRef(), RawLocalRef(), &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, reference_strict_equals_raw_local_with_verify_local)
{
    ani_string verifyStr {};
    ASSERT_EQ(env_->String_NewUTF8(POSTFORK_VERIFY_STRING.data(), POSTFORK_VERIFY_STRING.size(), &verifyStr), ANI_OK);

    ani_boolean isEquals = ANI_TRUE;
    ASSERT_EQ(env_->Reference_StrictEquals(RawLocalRef(), reinterpret_cast<ani_ref>(verifyStr), &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_FALSE);

    ASSERT_EQ(env_->Reference_Delete(reinterpret_cast<ani_ref>(verifyStr)), ANI_OK);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, global_reference_delete_returns_incorrect_ref_for_raw_local)
{
    ASSERT_EQ(env_->GlobalReference_Delete(RawLocalRef()), ANI_INCORRECT_REF);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, global_reference_create_from_prefork_raw_local_after_late_init)
{
    ani_ref gref2 {};
    ASSERT_EQ(env_->GlobalReference_Create(RawLocalRef(), &gref2), ANI_OK);
    ASSERT_EQ(env_->GlobalReference_Delete(gref2), ANI_OK);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, global_reference_delete_prefork_global_after_late_init)
{
    ASSERT_EQ(env_->GlobalReference_Delete(GlobalRef()), ANI_OK);
    ClearGlobalRef();
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, weak_reference_create_from_prefork_raw_local_after_late_init)
{
    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(RawLocalRef(), &wref), ANI_OK);
    ASSERT_EQ(env_->WeakReference_Delete(wref), ANI_OK);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, weak_reference_get_reference_from_prefork_raw_local)
{
    ani_wref wref {};
    ASSERT_EQ(env_->WeakReference_Create(RawLocalRef(), &wref), ANI_OK);

    ani_boolean wasReleased = ANI_TRUE;
    ani_ref outRef {};
    ASSERT_EQ(env_->WeakReference_GetReference(wref, &wasReleased, &outRef), ANI_OK);
    ASSERT_EQ(wasReleased, ANI_FALSE);

    ani_boolean isEquals = ANI_FALSE;
    ASSERT_EQ(env_->Reference_Equals(RawLocalRef(), outRef, &isEquals), ANI_OK);
    ASSERT_EQ(isEquals, ANI_TRUE);

    ASSERT_EQ(env_->Reference_Delete(outRef), ANI_OK);
    ASSERT_EQ(env_->WeakReference_Delete(wref), ANI_OK);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, weak_reference_delete_rejects_raw_local_as_wref)
{
    ASSERT_EQ(env_->WeakReference_Delete(reinterpret_cast<ani_wref>(RawLocalRef())), ANI_INCORRECT_REF);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, reference_delete_global_ref_returns_incorrect_ref)
{
    ASSERT_EQ(env_->Reference_Delete(GlobalRef()), ANI_INCORRECT_REF);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, reference_delete_null_returns_invalid_args)
{
    ASSERT_EQ(env_->c_api->Reference_Delete(env_, nullptr), ANI_INVALID_ARGS);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, string_get_utf8_size_with_prefork_raw_local)
{
    ani_size size = 0;
    ASSERT_EQ(env_->String_GetUTF8Size(reinterpret_cast<ani_string>(RawLocalRef()), &size), ANI_OK);
    ASSERT_EQ(size, PREFORK_LOCAL_STRING.size());
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, string_get_utf8_with_prefork_raw_local)
{
    ani_size size = 0;
    ASSERT_EQ(env_->String_GetUTF8Size(reinterpret_cast<ani_string>(RawLocalRef()), &size), ANI_OK);

    std::vector<char> buffer(size + 1);
    ani_size written = 0;
    ASSERT_EQ(env_->String_GetUTF8(reinterpret_cast<ani_string>(RawLocalRef()), buffer.data(), buffer.size(), &written),
              ANI_OK);
    ASSERT_EQ(written, PREFORK_LOCAL_STRING.size());
    ASSERT_STREQ(buffer.data(), PREFORK_LOCAL_STRING.data());
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, object_get_type_with_prefork_raw_local)
{
    ani_type type {};
    ASSERT_EQ(env_->Object_GetType(reinterpret_cast<ani_object>(RawLocalRef()), &type), ANI_OK);
    ASSERT_NE(type, nullptr);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, object_instance_of_string_with_prefork_raw_local)
{
    ani_class stringClass {};
    ASSERT_EQ(env_->FindClass("std.core.String", &stringClass), ANI_OK);

    ani_boolean isInstance = ANI_FALSE;
    ASSERT_EQ(env_->Object_InstanceOf(reinterpret_cast<ani_object>(RawLocalRef()), stringClass, &isInstance), ANI_OK);
    ASSERT_EQ(isInstance, ANI_TRUE);

    ASSERT_EQ(env_->Reference_Delete(stringClass), ANI_OK);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, destroy_escape_local_scope_rejects_prefork_raw_local)
{
    ASSERT_EQ(env_->CreateEscapeLocalScope(ESCAPE_SCOPE_CAPACITY), ANI_OK);

    ani_ref outRef {};
    ASSERT_EQ(env_->DestroyEscapeLocalScope(RawLocalRef(), &outRef), ANI_INCORRECT_REF);

    ani_ref validRef {};
    ASSERT_EQ(env_->GetUndefined(&validRef), ANI_OK);
    ASSERT_EQ(env_->DestroyEscapeLocalScope(validRef, &outRef), ANI_OK);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, create_local_scope_prefork_raw_local_still_valid)
{
    ASSERT_EQ(env_->CreateLocalScope(LOCAL_SCOPE_CAPACITY), ANI_OK);

    ani_boolean isNull = ANI_TRUE;
    ASSERT_EQ(env_->Reference_IsNull(RawLocalRef(), &isNull), ANI_OK);
    ASSERT_EQ(isNull, ANI_FALSE);

    ASSERT_EQ(env_->DestroyLocalScope(), ANI_OK);
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

TEST_F(PostforkRawLocalRefTest, reference_delete_with_prefork_raw_local)
{
    ASSERT_EQ(env_->Reference_Delete(RawLocalRef()), ANI_OK);
    ClearRawLocalRef();
    (void)GetAndClearAbortMessage();
    ASSERT_NO_ABORT_MESSAGE();
}

}  // namespace ark::ets::ani::verify::testing
