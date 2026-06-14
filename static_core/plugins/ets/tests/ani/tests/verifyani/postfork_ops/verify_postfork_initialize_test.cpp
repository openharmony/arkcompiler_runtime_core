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

#include "plugins/ets/runtime/ets_ani_env.h"

namespace ark::ets::ani::verify::testing {

namespace {

// NOLINTNEXTLINE(readability-identifier-naming)
constexpr const char *POSTFORK_MODULE = "verify_postfork_initialize_test";
// NOLINTNEXTLINE(readability-identifier-naming)
constexpr const char *POSTFORK_MODULE_NATIVE = "postforkModuleNative";
// NOLINTNEXTLINE(readability-identifier-naming)
constexpr const char *POSTFORK_MODULE_NATIVE_SIGNATURE = ":i";
constexpr ani_int POSTFORK_NATIVE_RESULT = 73;
constexpr ani_int POSTFORK_MANAGED_RESULT = 17;
constexpr ani_int POSTFORK_PROXY_RESULT = 42;

void LateInitializeVerifyAni(ani_env *env)
{
    auto *pandaEnv = PandaAniEnv::FromAniEnv(env);
    auto *vm = pandaEnv->GetEtsVM();
    ASSERT_FALSE(vm->IsVerifyANI());
    vm->CreateANIVerifier(true);
    pandaEnv->CreateEnvANIVerifier();
    ASSERT_TRUE(vm->IsVerifyANI());
}

ani_int PostforkModuleNative([[maybe_unused]] ani_env *env)
{
    return POSTFORK_NATIVE_RESULT;
}

}  // namespace

class PostforkInitializeTest : public ani::testing::AniTest {
public:
    void TearDown() override
    {
        if (enteredVerifyFrame_) {
            auto *pandaEnv = PandaAniEnv::FromAniEnv(env_);
            auto popErr = pandaEnv->GetEnvANIVerifier()->PopNativeFrame();
            ASSERT_FALSE(popErr.has_value()) << "Leaked scope detected in test: " << popErr.value();
            PandaEtsVM::GetCurrent()->GetANIVerifier()->SetAbortHook(nullptr, nullptr);
            PandaEtsVM::GetCurrent()->GetANIVerifier()->SetErrorHook(nullptr, nullptr);
        }
        AniTest::TearDown();
        ASSERT_NO_ABORT_MESSAGE();
        ss_.str("");
    }

    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {};
    }

    void LateInitializeAndEnterNativeFrame()
    {
        LateInitializeVerifyAni(env_);

        ani_env *verifyEnv = nullptr;
        ASSERT_EQ(vm_->GetEnv(ANI_VERSION_1, &verifyEnv), ANI_OK);
        env_ = verifyEnv;

        auto *pandaEnv = PandaAniEnv::FromAniEnv(env_);
        auto *verifier = pandaEnv->GetEtsVM()->GetANIVerifier();
        ASSERT_NE(verifier, nullptr);
        verifier->SetAbortHook(AbortHook, this);
        verifier->SetErrorHook(AbortHook, this);

        pandaEnv->GetEnvANIVerifier()->PushNativeFrame(pandaEnv);
        env_ = pandaEnv->GetEnvANIVerifier()->GetEnv();
        enteredVerifyFrame_ = true;
    }

    std::string GetAndClearAbortMessage()
    {
        std::string msg = ss_.str();
        ss_.str("");
        return msg;
    }

    void BindPostforkModuleNative(ani_function *fn)
    {
        ani_module mod {};
        ASSERT_EQ(env_->FindModule(POSTFORK_MODULE, &mod), ANI_OK);
        ASSERT_EQ(env_->Module_FindFunction(mod, POSTFORK_MODULE_NATIVE, POSTFORK_MODULE_NATIVE_SIGNATURE, fn), ANI_OK);

        ani_native_function nativeFn {
            POSTFORK_MODULE_NATIVE,
            POSTFORK_MODULE_NATIVE_SIGNATURE,
            reinterpret_cast<void *>(PostforkModuleNative),
        };
        ASSERT_EQ(env_->Module_BindNativeFunctions(mod, &nativeFn, 1), ANI_OK);
    }

    void CallPostforkModuleNative(ani_function fn)
    {
        ani_int result {};
        ASSERT_EQ(env_->Function_Call_Int(fn, &result), ANI_OK);  // NOLINT(cppcoreguidelines-pro-type-vararg)
        ASSERT_EQ(result, POSTFORK_NATIVE_RESULT);
    }

private:
    static void AbortHook(void *data, const std::string_view message)
    {
        auto *self = reinterpret_cast<PostforkInitializeTest *>(data);
        self->ss_ << message;
    }

    std::stringstream ss_;
    bool enteredVerifyFrame_ {false};
};

TEST_F(PostforkInitializeTest, verify_api_available_after_late_init)
{
    LateInitializeAndEnterNativeFrame();
    uint32_t version {};
    ASSERT_EQ(env_->c_api->GetVersion(env_, &version), ANI_OK);
    ASSERT_EQ(version, ANI_VERSION_1);
}

TEST_F(PostforkInitializeTest, prebound_module_native_function_callable_after_late_init)
{
    ani_function fn {};
    BindPostforkModuleNative(&fn);
    CallPostforkModuleNative(fn);

    LateInitializeAndEnterNativeFrame();

    CallPostforkModuleNative(fn);
}

TEST_F(PostforkInitializeTest, managed_function_callable_after_late_init)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(POSTFORK_MODULE, "postforkManagedValue"), POSTFORK_MANAGED_RESULT);

    LateInitializeAndEnterNativeFrame();

    ASSERT_EQ(CallEtsFunction<ani_int>(POSTFORK_MODULE, "postforkManagedValue"), POSTFORK_MANAGED_RESULT);
}

TEST_F(PostforkInitializeTest, reflect_proxy_call_works_after_late_init)
{
    ASSERT_EQ(CallEtsFunction<ani_int>(POSTFORK_MODULE, "callPostforkReflectProxy"), POSTFORK_PROXY_RESULT);

    LateInitializeAndEnterNativeFrame();

    ASSERT_EQ(CallEtsFunction<ani_int>(POSTFORK_MODULE, "callPostforkReflectProxy"), POSTFORK_PROXY_RESULT);
}

}  // namespace ark::ets::ani::verify::testing
