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

#include <array>
#include <string>

#include "compiler/aot/aot_manager.h"
#include "plugins/ets/runtime/integrate/ark_vm_api.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/runtime.h"

namespace ark::test {

#if defined(PANDA_WITH_ETS)
class ArkVmApiTest : public testing::Test {
public:
    ArkVmApiTest()
    {
        Runtime::Create(CreateOptions());
        runtime_ = Runtime::GetCurrent();
    }

    ~ArkVmApiTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(ArkVmApiTest);
    NO_MOVE_SEMANTIC(ArkVmApiTest);

protected:
    static RuntimeOptions CreateOptions()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType("g1-gc");
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcWorkersCount(0);
        options.SetAdaptiveTlabSize(false);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetExplicitConcurrentGcEnabled(false);
        options.SetVerifyCallStack(false);
        return options;
    }

    AotManager *GetAotManager() const
    {
        return runtime_->GetClassLinker()->GetAotManager();
    }

    Runtime *runtime_ {nullptr};  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(ArkVmApiTest, RegisterProfilePathsNullInfos)
{
    arkvm_status status = ARKVM_RegisterProfilePaths(nullptr, 1);
    EXPECT_EQ(status, ARKVM_INVALID_ARGS);

    auto snapshot = GetAotManager()->GetProfiledPandaFilesMapSnapshot();
    EXPECT_TRUE(snapshot.empty());
}

TEST_F(ArkVmApiTest, RegisterProfilePathsZeroCount)
{
    std::string abcPath = "/tmp/app.abc";
    std::string curProfilePath = "/tmp/app.ap";
    arkvm_profile_path_info info {abcPath.c_str(), curProfilePath.c_str(), nullptr};

    arkvm_status status = ARKVM_RegisterProfilePaths(&info, 0);
    EXPECT_EQ(status, ARKVM_INVALID_ARGS);

    auto snapshot = GetAotManager()->GetProfiledPandaFilesMapSnapshot();
    EXPECT_TRUE(snapshot.empty());
}

TEST_F(ArkVmApiTest, RegisterProfilePathsAllValid)
{
    std::string abcPath1 = "/tmp/app1.abc";
    std::string curProfilePath1 = "/tmp/app1.ap";
    std::string baselineProfilePath1 = "/tmp/app1.base.ap";
    std::string abcPath2 = "/tmp/app2.abc";
    std::string curProfilePath2 = "/tmp/app2.ap";

    std::array<arkvm_profile_path_info, 2> infos {{
        {abcPath1.c_str(), curProfilePath1.c_str(), baselineProfilePath1.c_str()},
        {abcPath2.c_str(), curProfilePath2.c_str(), nullptr},
    }};

    arkvm_status status = ARKVM_RegisterProfilePaths(infos.data(), infos.size());
    EXPECT_EQ(status, ARKVM_OK);

    auto snapshot = GetAotManager()->GetProfiledPandaFilesMapSnapshot();
    EXPECT_EQ(snapshot.size(), infos.size());
}

TEST_F(ArkVmApiTest, RegisterProfilePathsPartialSuccess)
{
    std::string abcPath = "/tmp/app.abc";
    std::string curProfilePath = "/tmp/app.ap";
    std::string invalidCurProfilePath = "/tmp/invalid.ap";

    std::array<arkvm_profile_path_info, 2> infos {{
        {abcPath.c_str(), curProfilePath.c_str(), nullptr},
        {nullptr, invalidCurProfilePath.c_str(), nullptr},
    }};

    arkvm_status status = ARKVM_RegisterProfilePaths(infos.data(), infos.size());
    EXPECT_EQ(status, ARKVM_PARTIAL_SUCCESS);

    auto snapshot = GetAotManager()->GetProfiledPandaFilesMapSnapshot();
    EXPECT_EQ(snapshot.size(), 1U);
}

TEST_F(ArkVmApiTest, RegisterProfilePathsAllInvalid)
{
    std::string curProfilePath = "/tmp/app.ap";
    std::string abcPath = "/tmp/app.abc";

    std::array<arkvm_profile_path_info, 2> infos {{
        {nullptr, curProfilePath.c_str(), nullptr},
        {abcPath.c_str(), nullptr, nullptr},
    }};

    arkvm_status status = ARKVM_RegisterProfilePaths(infos.data(), infos.size());
    EXPECT_EQ(status, ARKVM_INVALID_ARGS);

    auto snapshot = GetAotManager()->GetProfiledPandaFilesMapSnapshot();
    EXPECT_TRUE(snapshot.empty());
}
#else
TEST(ArkVmApiTest, Disabled)
{
    GTEST_SKIP() << "PANDA_WITH_ETS not enabled";
}
#endif

}  // namespace ark::test
