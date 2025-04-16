/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"

namespace ark::ets::test {
static int64_t GetCoroId()
{
    return reinterpret_cast<int64_t>(EtsCoroutine::GetCurrent());
}

class JsModeLaunchTest : public testing::Test {
protected:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(false);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});
        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib, "js_mode_launch.abc"});
        options.SetCoroutineImpl("stackful");

        Runtime::Create(options);

        EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
        ScopedManagedCodeThread scope(coroutine);
        EtsClassLinker *etsClassLinker = coroutine->GetPandaVM()->GetClassLinker();
        EtsClass *global = etsClassLinker->GetClass("Ljs_mode_launch/JSCoroutine;");
        ASSERT_NE(nullptr, global);
        EtsMethod *getCoroIdMethod = global->GetStaticMethod("getCoroutineId", nullptr);
        ASSERT_NE(nullptr, getCoroIdMethod);
        ASSERT_TRUE(getCoroIdMethod->IsNative());
        getCoroIdMethod->RegisterNativeDeprecated(reinterpret_cast<void *>(GetCoroId));
    }

    void TearDown() override
    {
        Runtime::Destroy();
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    const std::string abcFile_ = "js_mode_launch.abc";
};

TEST_F(JsModeLaunchTest, Call)
{
    const std::string mainFunc = "js_mode_launch.ETSGLOBAL::main";
    auto res = Runtime::GetCurrent()->ExecutePandaFile(abcFile_.c_str(), mainFunc.c_str(), {});
    ASSERT_EQ(0, res.Value());
}
}  // namespace ark::ets::test
