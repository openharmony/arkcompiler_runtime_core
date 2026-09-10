/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <array>
#include <cstdint>

#include <gtest/gtest.h>

#include <ani.h>

#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

namespace ark::ets::interop::js::testing {

namespace {

enum class StackInfoProbeFailure : uint32_t {
    NONE = 0U,
    NO_INTEROP_CONTEXT = 1U,
    GET_STACK_INFO_FAILED = 2U,
    STACK_INFO_CHANGED = 4U,
};

class StackInfoProbe {
public:
    void Reset()
    {
        bStackInfo_ = {};
        captured_ = false;
        checked_ = false;
        failures_ = static_cast<uint32_t>(StackInfoProbeFailure::NONE);
    }

    void Capture()
    {
        if (!ReadCurrentStackInfo(&bStackInfo_)) {
            return;
        }
        captured_ = true;
    }

    void AssertCurrentStackMatchesCapture()
    {
        NapiStackInfo currentStackInfo {};
        if (!ReadCurrentStackInfo(&currentStackInfo)) {
            return;
        }
        checked_ = true;
        if (!captured_ || currentStackInfo.stackStart != bStackInfo_.stackStart ||
            currentStackInfo.stackSize != bStackInfo_.stackSize) {
            failures_ |= static_cast<uint32_t>(StackInfoProbeFailure::STACK_INFO_CHANGED);
        }
    }

    bool Passed() const
    {
        return captured_ && checked_ && failures_ == static_cast<uint32_t>(StackInfoProbeFailure::NONE);
    }

    uint32_t FailureMask() const
    {
        return failures_;
    }

private:
    bool ReadCurrentStackInfo(NapiStackInfo *stackInfo)
    {
        auto *executionCtx = EtsExecutionContext::GetCurrent();
        auto *interopCtx = InteropCtx::Current(executionCtx);
        if (interopCtx == nullptr) {
            failures_ |= static_cast<uint32_t>(StackInfoProbeFailure::NO_INTEROP_CONTEXT);
            return false;
        }
        if (napi_get_stackinfo(interopCtx->GetJSEnv(), stackInfo) != napi_ok) {
            failures_ |= static_cast<uint32_t>(StackInfoProbeFailure::GET_STACK_INFO_FAILED);
            return false;
        }
        return true;
    }

    NapiStackInfo bStackInfo_ {};
    bool captured_ {false};
    bool checked_ {false};
    uint32_t failures_ {static_cast<uint32_t>(StackInfoProbeFailure::NONE)};
};

StackInfoProbe g_stackInfoProbe;

void CaptureActiveInteropStack([[maybe_unused]] ani_env *env)
{
    g_stackInfoProbe.Capture();
}

void AssertActiveInteropStack([[maybe_unused]] ani_env *env)
{
    g_stackInfoProbe.AssertCurrentStackMatchesCapture();
}

}  // namespace

class InteropStackInfoTest : public EtsInteropTest {
public:
    static bool GetAniEnv(ani_env **env)
    {
        ani_vm *aniVm {};
        ani_size vmCount {};
        auto status = ANI_GetCreatedVMs(&aniVm, 1U, &vmCount);
        if (status != ANI_OK || vmCount == 0) {
            return false;
        }
        status = aniVm->GetEnv(ANI_VERSION_1, env);
        return status == ANI_OK && *env != nullptr;
    }

    ani_status RegisterNativeFunctions(ani_env *env)
    {
        ani_ref classRef = GetClassRefObject(env, "interop_stack_info_test.ETSGLOBAL");
        if (classRef == nullptr) {
            return ANI_ERROR;
        }
        std::array methods = {
            ani_native_function {"captureActiveInteropStack", nullptr,
                                 reinterpret_cast<void *>(CaptureActiveInteropStack)},
            ani_native_function {"assertActiveInteropStack", nullptr,
                                 reinterpret_cast<void *>(AssertActiveInteropStack)},
        };
        return env->Module_BindNativeFunctions(static_cast<ani_module>(classRef), methods.data(), methods.size());
    }
};

TEST_F(InteropStackInfoTest, RestoresBoundaryForOverlappingCoroutineScopes)
{
    g_stackInfoProbe.Reset();
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));
    ASSERT_EQ(RegisterNativeFunctions(aniEnv), ANI_OK);
    ASSERT_TRUE(RunJsTestSuite("interop_stack_info_test.ts"));
    ASSERT_TRUE(g_stackInfoProbe.Passed()) << "failure mask: " << g_stackInfoProbe.FailureMask();
}

}  // namespace ark::ets::interop::js::testing
