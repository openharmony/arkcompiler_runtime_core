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

#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/interop_js/tooling/internal_api.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"

#include <ani.h>
#include <gtest/gtest.h>

namespace ark::ets::interop::js::testing {

constexpr int SINGLE_ETS_SEGMENT_ON_JS = 2;
constexpr int MIN_INTERLEAVED_SEGMENTS = 2;

static bool CaptureUnionStack(std::vector<bool> &frames)
{
    frames.clear();
    bool isEmpty = true;
    if (!UnionStackIsEmpty(&isEmpty) || isEmpty) {
        return false;
    }
    bool res = ForEachFrameInUnionStack(
        [&frames]([[maybe_unused]] const void *frame, bool isStaticFrame) { frames.push_back(isStaticFrame); });
    return res && !frames.empty();
}

static int CountSegments(const std::vector<bool> &frames)
{
    if (frames.empty()) {
        return 0;
    }
    int segments = 1;
    for (size_t i = 1; i < frames.size(); i++) {
        if (frames[i] != frames[i - 1]) {
            segments++;
        }
    }
    return segments;
}

static bool HasStaticAndDynamic(const std::vector<bool> &frames)
{
    bool hasStatic = false;
    bool hasDynamic = false;
    for (bool isStatic : frames) {
        if (isStatic) {
            hasStatic = true;
        } else {
            hasDynamic = true;
        }
    }
    return hasStatic && hasDynamic;
}

static bool TestPropertyAccessNoExtraRecords()
{
    std::vector<bool> frames;
    if (!CaptureUnionStack(frames)) {
        return false;
    }

    if (!HasStaticAndDynamic(frames) || !frames.front()) {
        return false;
    }

    return CountSegments(frames) == SINGLE_ETS_SEGMENT_ON_JS;
}

static bool TestFunctionCallInterleaving()
{
    std::vector<bool> frames;
    if (!CaptureUnionStack(frames)) {
        return false;
    }

    if (!HasStaticAndDynamic(frames) || !frames.front()) {
        return false;
    }

    return CountSegments(frames) >= MIN_INTERLEAVED_SEGMENTS;
}

class HybridStackEtsToTsTest : public EtsInteropTest {
public:
    static bool GetAniEnv(ani_env **env)
    {
        ani_vm *aniVm;
        ani_size res;

        auto status = ANI_GetCreatedVMs(&aniVm, 1U, &res);
        if (status != ANI_OK || res == 0U) {
            return false;
        }

        status = aniVm->GetEnv(ANI_VERSION_1, env);
        return status == ANI_OK && *env != nullptr;
    }

    bool RegisterETSFunction(ani_env *env)
    {
        ani_ref classRef = GetClassRefObject(env, "hybrid_stack_ets_to_ts.ETSGLOBAL");
        if (classRef == nullptr) {
            return false;
        }

        std::array methods = {
            ani_native_function {"testPropertyAccessNative", nullptr,
                                 reinterpret_cast<void *>(TestPropertyAccessNoExtraRecords)},
            ani_native_function {"testFunctionCallNative", nullptr,
                                 reinterpret_cast<void *>(TestFunctionCallInterleaving)},
        };
        return env->Module_BindNativeFunctions(static_cast<ani_module>(classRef), methods.data(), methods.size()) ==
               ANI_OK;
    }
};

TEST_F(HybridStackEtsToTsTest, check_ets_to_ts_hybridstack)
{
    ani_env *aniEnv {};
    ASSERT_TRUE(GetAniEnv(&aniEnv));

    ASSERT_TRUE(RegisterETSFunction(aniEnv));
    ASSERT_TRUE(RunJsTestSuite("entry_stack.ts"));
}

}  // namespace ark::ets::interop::js::testing
