/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

#include "ets_vm.h"
#include "include/runtime.h"
#include "include/runtime_options.h"
#include "libarkbase/test_utilities.h"
#include "libarkbase/os/exec.h"
#include "jit/libprofile/aot_profiling_data.h"
#include "jit/profiling_loader.h"
#include "jit/profiling_saver.h"
#include "libarkbase/os/filesystem.h"

namespace ark::test {
class ProfileSaverTest : public testing::Test {
public:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetLoadRuntimes({"ets"});
        options.SetProfilesaverSleepingTimeMs(0U);
        options.SetCompilerProfilingThreshold(0U);
        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib, "profile_saver_test.abc"});
        options_ = std::move(options);
    }

    void SetProfilesaverSleepingTimeMs(uint32_t value)
    {
        options_.SetProfilesaverSleepingTimeMs(value);
    }

    void SetWorkersType(const std::string &value)
    {
        options_.SetWorkersType(value);
    }

    void SetIncrementalProfilesaverEnabled(bool value)
    {
        options_.SetIncrementalProfilesaverEnabled(value);
    }

    void CreateRuntime()
    {
        bool success = Runtime::Create(options_);
        ASSERT_TRUE(success) << "Cannot create Runtime";
    }

    void RunPandafile()
    {
        const std::string mainFunc = "profile_saver_test.ETSGLOBAL::main";
        const std::string fileName = "profile_saver_test.abc";
        auto res = Runtime::GetCurrent()->ExecutePandaFile(fileName.c_str(), mainFunc.c_str(), {});
        ASSERT_TRUE(res);
    }

    void InitPGOFilePath(const std::string &pgoFilePath)
    {
        options_.SetProfileOutput(pgoFilePath);
        if (std::filesystem::exists(pgoFilePath)) {
            std::filesystem::remove(pgoFilePath);
        }
    }

    void CheckLoadProfile()
    {
        ProfilingLoader loader;
        const std::string &pgoFilePath = options_.GetProfileOutput();
        auto profileCtxOrError = loader.LoadProfile(PandaString(pgoFilePath));
        auto classCtxStr = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->GetBootClassContext() + ":" +
                           Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->GetAppClassContext();
        ASSERT_TRUE(profileCtxOrError.HasValue());
        ASSERT_EQ(*profileCtxOrError, PandaString(classCtxStr));
    }

    void CheckLoadProfileFail()
    {
        ProfilingLoader loader;
        const std::string &pgoFilePath = options_.GetProfileOutput();
        auto profileCtxOrError = loader.LoadProfile(PandaString(pgoFilePath));
        ASSERT_FALSE(profileCtxOrError.HasValue());
    }

    void DestroyRuntime()
    {
        bool success = Runtime::Destroy();
        ASSERT_TRUE(success) << "Cannot destroy Runtime";
    }

    void RunAptoolAndCheck()
    {
        auto apPath = options_.GetProfileOutput();
        auto aptoolPath = GetArkAptoolBinary();
        auto tempRoot = std::filesystem::temp_directory_path() / "aptool_integration";
        std::error_code ec;
        std::filesystem::create_directories(tempRoot, ec);
        auto yamlPath = tempRoot / "profile_saver_dump.yaml";

        auto sourceAbc = std::filesystem::absolute("profile_saver_test.abc");
        ASSERT_TRUE(std::filesystem::exists(sourceAbc))
            << "profile_saver_test.abc must exist for aptool integration test";
        auto abcDir = sourceAbc.parent_path();

        auto execResult = os::exec::Exec(aptoolPath.c_str(), "dump", "--ap-path", apPath.c_str(), "--output",
                                         yamlPath.string().c_str(), "--abc-path", sourceAbc.string().c_str(),
                                         "--abc-dir", abcDir.string().c_str());
        ASSERT_TRUE(execResult) << "ark_aptool dump failed: " << execResult.Error().ToString();
        EXPECT_EQ(execResult.Value(), 0U);

        std::ifstream yaml(yamlPath);
        ASSERT_TRUE(yaml.is_open());
        std::string content((std::istreambuf_iterator<char>(yaml)), std::istreambuf_iterator<char>());
        EXPECT_NE(content.find("ClassCtxStr:"), std::string::npos);
        EXPECT_NE(content.find("class_descriptor"), std::string::npos);
        EXPECT_NE(content.find("method_signature"), std::string::npos);
        EXPECT_NE(content.find("inline_caches"), std::string::npos);
        EXPECT_NE(content.find("branches"), std::string::npos);
        EXPECT_NE(content.find("throws"), std::string::npos);
        EXPECT_NE(content.find("bytecode:"), std::string::npos);

        auto filteredYamlPath = tempRoot / "profile_saver_filtered.yaml";
        auto filteredResult =
            os::exec::Exec(aptoolPath.c_str(), "dump", "--ap-path", apPath.c_str(), "--output",
                           filteredYamlPath.string().c_str(), "--abc-path", sourceAbc.string().c_str(), "--abc-dir",
                           abcDir.string().c_str(), "--keep-method-name", "foo");
        ASSERT_TRUE(filteredResult) << "ark_aptool dump filtered run failed: " << filteredResult.Error().ToString();
        EXPECT_EQ(filteredResult.Value(), 0U);

        std::ifstream filteredYaml(filteredYamlPath);
        ASSERT_TRUE(filteredYaml.is_open());
        std::string filteredContent((std::istreambuf_iterator<char>(filteredYaml)), std::istreambuf_iterator<char>());
        EXPECT_NE(filteredContent.find("method_signature: \"foo"), std::string::npos);
        EXPECT_EQ(filteredContent.find("method_signature: \"main"), std::string::npos);

        std::filesystem::remove_all(tempRoot, ec);
    }

    static std::string GetArkAptoolBinary()
    {
        auto execPath = std::filesystem::canonical("/proc/self/exe");
        auto outDir = execPath.parent_path().parent_path();
        auto binPath = outDir / "bin" / "ark_aptool";
        return binPath.string();
    }

private:
    RuntimeOptions options_;
};

TEST_F(ProfileSaverTest, ProfileSaverWorkerTest)
{
    InitPGOFilePath("profile_saver_worker_test.ap");
    SetIncrementalProfilesaverEnabled(true);
    CreateRuntime();
    RunPandafile();
    EXPECT_TRUE(Runtime::GetCurrent()->IncrementalSaveProfileInfo());
    auto saverWorker = Runtime::GetCurrent()->GetPandaVM()->GetProfileSaverWorker();
    saverWorker->WaitForFinishSaverTask();
    ASSERT_TRUE(saverWorker->TryAddTask());
    saverWorker->WaitForFinishSaverTask();
    CheckLoadProfile();
    RunAptoolAndCheck();
    DestroyRuntime();
}

TEST_F(ProfileSaverTest, ProfilingSaverAddFailOfTimeTest)
{
    InitPGOFilePath("profiling_saver_add_fail_of_time_test.ap");
    const uint32_t largeSleepingTime = 10000000;
    SetProfilesaverSleepingTimeMs(largeSleepingTime);
    SetIncrementalProfilesaverEnabled(true);
    CreateRuntime();
    RunPandafile();
    EXPECT_TRUE(Runtime::GetCurrent()->IncrementalSaveProfileInfo());
    auto saver = Runtime::GetCurrent()->GetPandaVM()->GetProfileSaverWorker();
    EXPECT_NE(saver, nullptr);
    EXPECT_FALSE(saver->TryAddTask());
    CheckLoadProfileFail();
    DestroyRuntime();
}

TEST_F(ProfileSaverTest, ProfilingSaverDisabledWithOption)
{
    InitPGOFilePath("profiling_saver_disabled_with_option.ap");
    SetIncrementalProfilesaverEnabled(false);
    CreateRuntime();
    RunPandafile();
    EXPECT_FALSE(Runtime::GetCurrent()->IncrementalSaveProfileInfo());
    auto saver = Runtime::GetCurrent()->GetPandaVM()->GetProfileSaverWorker();
    EXPECT_EQ(saver, nullptr);
    CheckLoadProfileFail();
    DestroyRuntime();
}

TEST_F(ProfileSaverTest, ProfilingSaverDisabledWithWorkerType)
{
    InitPGOFilePath("profiling_saver_disabled_with_worker_type.ap");
    SetWorkersType("threadpool");
    SetIncrementalProfilesaverEnabled(true);
    CreateRuntime();
    RunPandafile();
    EXPECT_TRUE(Runtime::GetCurrent()->IncrementalSaveProfileInfo());
    auto saver = Runtime::GetCurrent()->GetPandaVM()->GetProfileSaverWorker();
    EXPECT_EQ(saver, nullptr);
    CheckLoadProfileFail();
    DestroyRuntime();
}
}  // namespace ark::test
