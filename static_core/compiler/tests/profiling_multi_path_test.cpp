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
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "assembler/assembly-emitter.h"
#include "assembler/assembly-parser.h"
#include "compiler/aot/aot_manager.h"
#include "include/mem/panda_containers.h"
#include "include/runtime.h"
#include "jit/profiling_loader.h"
#include "jit/profiling_saver.h"
#include "libarkbase/macros.h"
#include "panda_runner.h"

namespace ark::test {
// NOLINTBEGIN(readability-magic-numbers)

namespace {

constexpr auto SOURCE = R"(
.function void main() <static> {
    movi v0, 0x2
    movi v1, 0x0
    jump_label_1: lda v1
    jge v0, jump_label_0
    call.short foo
    inci v1, 0x1
    jmp jump_label_1
    jump_label_0: return.void
}

.function i32 foo() <static> {
    movi v0, 0x64
    movi v1, 0x0
    mov v2, v1
    jump_label_3: lda v2
    jge v0, jump_label_0
    lda v2
    modi 0x3
    jnez jump_label_1
    lda v1
    addi 0x2
    sta v3
    mov v1, v3
    jmp jump_label_2
    jump_label_1: lda v1
    addi 0x3
    sta v3
    mov v1, v3
    jump_label_2: inci v2, 0x1
    jmp jump_label_3
    jump_label_0: lda v1
    return
}
)";

std::string CreateTmpFileName(const std::string &prefix)
{
    uint32_t tid = os::thread::GetCurrentThreadId();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::string pgoFileName = helpers::string::Format("%s_%04x.ap", prefix.c_str(), tid);
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static const std::string LOCATION = "/tmp";
    return LOCATION + "/" + pgoFileName;
}

void CleanupFile(const std::string &path)
{
    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
    }
}

size_t CountProfileMethods(const std::string &path)
{
    ProfilingLoader loader;
    auto result = loader.LoadProfile(PandaString(path));
    if (!result.HasValue()) {
        return 0;
    }
    auto &allMethods = loader.GetAotProfilingData().GetAllMethods();
    size_t total = 0;
    for (auto &[fileIdx, methods] : allMethods) {
        total += methods.size();
    }
    return total;
}

size_t CountProfilePandaFiles(const std::string &path)
{
    ProfilingLoader loader;
    auto result = loader.LoadProfile(PandaString(path));
    if (!result.HasValue()) {
        return 0;
    }
    return loader.GetAotProfilingData().GetPandaFileMap().size();
}

class ProfilingMultiPathTest : public ::testing::Test {
public:
    PandaRunner &CreateRunner()
    {
        runner_.GetRuntimeOptions().SetIncrementalProfilesaverEnabled(false);
        runner_.GetRuntimeOptions().SetShouldLoadBootPandaFiles(false);
        runner_.GetRuntimeOptions().SetShouldInitializeIntrinsics(false);
        runner_.GetRuntimeOptions().SetCompilerProfilingThreshold(0U);
        return runner_;
    }

    void CollectProfile()
    {
        auto runtime = runner_.CreateRuntime();
        runner_.Run(runtime, SOURCE, std::vector<std::string> {});
    }

    compiler::AotManager *GetAotManager()
    {
        return Runtime::GetCurrent()->GetClassLinker()->GetAotManager();
    }

private:
    PandaRunner runner_;
};

}  // namespace

TEST_F(ProfilingMultiPathTest, RegisterAndRetrieveProfilePaths)
{
    CreateRunner();
    CollectProfile();

    {
        auto *aotManager = GetAotManager();

        // Snapshot should be empty before registration
        ASSERT_TRUE(aotManager->GetProfiledPandaFilesMapSnapshot().empty());

        // Register profile paths
        PandaVector<compiler::AotManager::ProfilePathEntry> entries;
        entries.emplace_back(PandaString("entry/modules.abc"),
                             compiler::ProfilePathInfo {PandaString("/data/profile/entry.ap"), PandaString("")});
        entries.emplace_back(PandaString("feature/modules.abc"),
                             compiler::ProfilePathInfo {PandaString("/data/profile/feature.ap"), PandaString("")});
        aotManager->RegisterProfilePaths(std::move(entries));

        // Snapshot should contain registered entries — copy to std containers for verification
        auto snapshot = aotManager->GetProfiledPandaFilesMapSnapshot();
        ASSERT_EQ(snapshot.size(), 2U);

        std::unordered_map<std::string, std::string> stdSnapshot;
        for (auto &[k, v] : snapshot) {
            stdSnapshot[std::string(k)] = std::string(v.curProfilePath);
        }
        snapshot.clear();

        EXPECT_EQ(stdSnapshot.size(), 2U);
        EXPECT_EQ(stdSnapshot["entry/modules.abc"], "/data/profile/entry.ap");
        EXPECT_EQ(stdSnapshot["feature/modules.abc"], "/data/profile/feature.ap");

        // Snapshot is a copy — original should still have 2 entries
        EXPECT_EQ(aotManager->GetProfiledPandaFilesMapSnapshot().size(), 2U);
    }

    Runtime::Destroy();
}

TEST_F(ProfilingMultiPathTest, RegisterEmptyEntriesNoOp)
{
    CreateRunner();
    CollectProfile();

    {
        auto *aotManager = GetAotManager();
        PandaVector<compiler::AotManager::ProfilePathEntry> entries;
        aotManager->RegisterProfilePaths(std::move(entries));
        ASSERT_TRUE(aotManager->GetProfiledPandaFilesMapSnapshot().empty());
    }

    Runtime::Destroy();
}

TEST_F(ProfilingMultiPathTest, SaveProfileWithUnmatchedFilesDoesNotMarkSaved)
{
    // This test validates the DataSaved ordering fix.
    // Before the fix: calling SaveProfile with an unmatched panda file set would
    // mark all methods as DataSaved, causing subsequent saves with the correct set to skip them.
    // After the fix: unmatched calls don't mark methods, so subsequent saves work correctly.
    auto pgoPath1 = CreateTmpFileName("multipath_unmatched1");
    auto pgoPath2 = CreateTmpFileName("multipath_unmatched2");
    CleanupFile(pgoPath1);
    CleanupFile(pgoPath2);

    CreateRunner();
    CollectProfile();

    {
        auto *aotManager = GetAotManager();
        ASSERT_TRUE(aotManager->HasProfiledMethods());

        auto classCtxStr = aotManager->GetBootClassContext() + ":" + aotManager->GetAppClassContext();
        auto &profiledMethods = aotManager->GetProfiledMethods();
        auto profiledMethodsFinal = aotManager->GetProfiledMethodsFinal();

        auto allPandaFiles = aotManager->GetProfiledPandaFiles();

        // First save: use a saveable file set that does NOT match any profiled methods
        {
            ProfilingSaver saver;
            PandaUnorderedSet<std::string> unmatchedFiles;
            unmatchedFiles.insert("nonexistent_module.abc");
            saver.SaveProfile(PandaString(pgoPath1), classCtxStr, profiledMethods, profiledMethodsFinal, allPandaFiles,
                              unmatchedFiles);
        }

        // Second save: use the actual panda file set — methods should still be saveable
        {
            ProfilingSaver saver;
            saver.SaveProfile(PandaString(pgoPath2), classCtxStr, profiledMethods, profiledMethodsFinal, allPandaFiles,
                              allPandaFiles);
        }
    }

    // Verify outside the Panda container scope using std::string paths
    EXPECT_EQ(CountProfileMethods(pgoPath1), 0U);
    EXPECT_GT(CountProfileMethods(pgoPath2), 0U);

    CleanupFile(pgoPath1);
    CleanupFile(pgoPath2);
    Runtime::Destroy();
}

TEST_F(ProfilingMultiPathTest, MultiPathSaveToMultipleFiles)
{
    // Test end-to-end: two ABC files with profiled methods from each,
    // saved to two separate .ap files via multi-path grouping logic.

    // Pandasm source for module1: main calls foo in a loop
    constexpr auto SOURCE1 = R"(
.function void main() <static> {
    movi v0, 0x2
    movi v1, 0x0
    jump_label_1: lda v1
    jge v0, jump_label_0
    call.short foo
    inci v1, 0x1
    jmp jump_label_1
    jump_label_0: return.void
}
.function i32 foo() <static> {
    movi v0, 0x64
    movi v1, 0x0
    mov v2, v1
    jump_label_3: lda v2
    jge v0, jump_label_0
    lda v2
    modi 0x3
    jnez jump_label_1
    lda v1
    addi 0x2
    sta v3
    mov v1, v3
    jmp jump_label_2
    jump_label_1: lda v1
    addi 0x3
    sta v3
    mov v1, v3
    jump_label_2: inci v2, 0x1
    jmp jump_label_3
    jump_label_0: lda v1
    return
}
)";

    // Pandasm source for module2: uses Mod2 record to avoid _GLOBAL conflict with module1
    constexpr auto SOURCE2 = R"(
.record Mod2 {}
.function void Mod2.entry2() <static> {
    movi v0, 0x2
    movi v1, 0x0
    jump_label_1: lda v1
    jge v0, jump_label_0
    call.short Mod2.bar
    inci v1, 0x1
    jmp jump_label_1
    jump_label_0: return.void
}
.function i32 Mod2.bar() <static> {
    movi v0, 0x64
    movi v1, 0x0
    mov v2, v1
    jump_label_3: lda v2
    jge v0, jump_label_0
    lda v2
    modi 0x3
    jnez jump_label_1
    lda v1
    addi 0x2
    sta v3
    mov v1, v3
    jmp jump_label_2
    jump_label_1: lda v1
    addi 0x3
    sta v3
    mov v1, v3
    jump_label_2: inci v2, 0x1
    jmp jump_label_3
    jump_label_0: lda v1
    return
}
)";

    auto abc1 = CreateTmpFileName("module1_abc");
    auto abc2 = CreateTmpFileName("module2_abc");
    auto ap1 = CreateTmpFileName("module1_profile");
    auto ap2 = CreateTmpFileName("module2_profile");
    CleanupFile(abc1);
    CleanupFile(abc2);
    CleanupFile(ap1);
    CleanupFile(ap2);

    // Emit two ABC files to disk
    {
        pandasm::Parser parser;
        auto res = parser.Parse(SOURCE1);
        ASSERT_TRUE(res) << res.Error().message;
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(abc1, res.Value()));
    }
    {
        pandasm::Parser parser;
        auto res = parser.Parse(SOURCE2);
        ASSERT_TRUE(res) << res.Error().message;
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(abc2, res.Value()));
    }

    // Create runtime with both ABC files as panda-files so GetPandaFiles() returns both
    RuntimeOptions options;
    options.SetShouldLoadBootPandaFiles(false);
    options.SetShouldInitializeIntrinsics(false);
    options.SetCompilerProfilingThreshold(0U);
    options.SetIncrementalProfilesaverEnabled(false);
    options.SetNoAsyncJit(true);
    options.SetPandaFiles({abc1, abc2});
    ASSERT_TRUE(Runtime::Create(options));

    {
        auto *runtime = Runtime::GetCurrent();

        // ExecutePandaFile loads abc1 and creates app context from GetPandaFiles()
        // which includes both abc1 and abc2 — so both end up in profiledPandaFiles_
        auto eres1 = runtime->ExecutePandaFile(abc1, "_GLOBAL::main", {});
        ASSERT_TRUE(eres1) << static_cast<unsigned>(eres1.Error());

        // Execute module2 — Mod2 record is unique to abc2, resolved from the app context
        auto eres2 = runtime->Execute("Mod2::entry2", {});
        ASSERT_TRUE(eres2) << static_cast<unsigned>(eres2.Error());

        auto *aotManager = runtime->GetClassLinker()->GetAotManager();
        ASSERT_TRUE(aotManager->HasProfiledMethods());

        // Verify methods come from both panda files
        auto profiledPandaFiles = aotManager->GetProfiledPandaFiles();

        // Register each ABC to a different profile path
        PandaVector<compiler::AotManager::ProfilePathEntry> entries;
        for (auto &pfName : profiledPandaFiles) {
            PandaString pfStr(pfName);
            if (pfStr.find("module1_abc") != PandaString::npos) {
                entries.emplace_back(pfStr, compiler::ProfilePathInfo {PandaString(ap1), PandaString("")});
            } else if (pfStr.find("module2_abc") != PandaString::npos) {
                entries.emplace_back(pfStr, compiler::ProfilePathInfo {PandaString(ap2), PandaString("")});
            }
        }
        ASSERT_EQ(entries.size(), 2U);
        aotManager->RegisterProfilePaths(std::move(entries));

        // Multi-path save logic (same as AddTask)
        auto classCtxStr = aotManager->GetBootClassContext() + ":" + aotManager->GetAppClassContext();
        auto &profiledMethods = aotManager->GetProfiledMethods();
        auto profiledMethodsFinal = aotManager->GetProfiledMethodsFinal();
        auto pathMapSnapshot = aotManager->GetProfiledPandaFilesMapSnapshot();
        ASSERT_EQ(pathMapSnapshot.size(), 2U);

        PandaUnorderedMap<PandaString, PandaUnorderedSet<std::string>> pathToAbcs;
        for (const auto &[abc, pathInfo] : pathMapSnapshot) {
            pathToAbcs[pathInfo.curProfilePath].insert(std::string(abc));
        }
        ASSERT_EQ(pathToAbcs.size(), 2U);

        auto allPandaFiles = aotManager->GetProfiledPandaFiles();
        ProfilingSaver profileSaver;
        for (auto &[profilePath, abcSet] : pathToAbcs) {
            profileSaver.SaveProfile(profilePath, classCtxStr, profiledMethods, profiledMethodsFinal, allPandaFiles,
                                     abcSet);
        }
    }

    // Verify: both .ap files should have profiled methods
    EXPECT_GT(CountProfileMethods(ap1), 0U);
    EXPECT_GT(CountProfileMethods(ap2), 0U);

    CleanupFile(abc1);
    CleanupFile(abc2);
    CleanupFile(ap1);
    CleanupFile(ap2);
    Runtime::Destroy();
}

TEST_F(ProfilingMultiPathTest, FallbackToDefaultPathWhenNoRegistration)
{
    // When no profile paths are registered, profile should be saved to the default path.
    auto defaultPath = CreateTmpFileName("multipath_fallback");
    CleanupFile(defaultPath);

    CreateRunner();
    CollectProfile();

    {
        auto *aotManager = GetAotManager();
        ASSERT_TRUE(aotManager->HasProfiledMethods());

        // Do NOT register any profile paths — snapshot should be empty
        ASSERT_TRUE(aotManager->GetProfiledPandaFilesMapSnapshot().empty());

        // Simulate the fallback logic from AddTask
        auto classCtxStr = aotManager->GetBootClassContext() + ":" + aotManager->GetAppClassContext();
        auto &profiledMethods = aotManager->GetProfiledMethods();
        auto profiledMethodsFinal = aotManager->GetProfiledMethodsFinal();
        auto profiledPandaFiles = aotManager->GetProfiledPandaFiles();

        ProfilingSaver profileSaver;
        profileSaver.SaveProfile(PandaString(defaultPath), classCtxStr, profiledMethods, profiledMethodsFinal,
                                 profiledPandaFiles, profiledPandaFiles);
    }

    // Verify outside Panda container scope
    EXPECT_GT(CountProfileMethods(defaultPath), 0U);

    CleanupFile(defaultPath);
    Runtime::Destroy();
}

TEST_F(ProfilingMultiPathTest, MultiPathSaveContainsAllPandaFiles)
{
    // Verifies that in multi-path mode, each .ap file's PandaFiles section contains ALL panda files
    // (not just the saveable subset), so inline cache entries referencing classes from other files
    // get valid pfIdx values instead of -1.

    auto abc1 = CreateTmpFileName("allpf_module1");
    auto abc2 = CreateTmpFileName("allpf_module2");
    auto ap1 = CreateTmpFileName("allpf_profile1");
    auto ap2 = CreateTmpFileName("allpf_profile2");
    CleanupFile(abc1);
    CleanupFile(abc2);
    CleanupFile(ap1);
    CleanupFile(ap2);

    constexpr auto SRC1 = R"(
.function void main() <static> {
    movi v0, 0x2
    movi v1, 0x0
    jump_label_1: lda v1
    jge v0, jump_label_0
    call.short foo
    inci v1, 0x1
    jmp jump_label_1
    jump_label_0: return.void
}
.function i32 foo() <static> {
    ldai 0x1
    return
}
)";

    constexpr auto SRC2 = R"(
.record Mod2 {}
.function i32 Mod2.bar() <static> {
    ldai 0x2
    return
}
)";

    {
        pandasm::Parser parser;
        auto res = parser.Parse(SRC1);
        ASSERT_TRUE(res) << res.Error().message;
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(abc1, res.Value()));
    }
    {
        pandasm::Parser parser;
        auto res = parser.Parse(SRC2);
        ASSERT_TRUE(res) << res.Error().message;
        ASSERT_TRUE(pandasm::AsmEmitter::Emit(abc2, res.Value()));
    }

    RuntimeOptions options;
    options.SetShouldLoadBootPandaFiles(false);
    options.SetShouldInitializeIntrinsics(false);
    options.SetCompilerProfilingThreshold(0U);
    options.SetIncrementalProfilesaverEnabled(false);
    options.SetNoAsyncJit(true);
    options.SetPandaFiles({abc1, abc2});
    ASSERT_TRUE(Runtime::Create(options));

    {
        auto *runtime = Runtime::GetCurrent();
        auto eres = runtime->ExecutePandaFile(abc1, "_GLOBAL::main", {});
        ASSERT_TRUE(eres) << static_cast<unsigned>(eres.Error());

        auto *aotManager = runtime->GetClassLinker()->GetAotManager();
        ASSERT_TRUE(aotManager->HasProfiledMethods());

        auto allPandaFiles = aotManager->GetProfiledPandaFiles();
        auto allFileCount = allPandaFiles.size();
        ASSERT_GE(allFileCount, 2U);

        // Register only abc1 to ap1, abc2 to ap2
        PandaVector<compiler::AotManager::ProfilePathEntry> entries;
        for (auto &pfName : allPandaFiles) {
            PandaString pfStr(pfName);
            if (pfStr.find("allpf_module1") != PandaString::npos) {
                entries.emplace_back(pfStr, compiler::ProfilePathInfo {PandaString(ap1), PandaString("")});
            } else if (pfStr.find("allpf_module2") != PandaString::npos) {
                entries.emplace_back(pfStr, compiler::ProfilePathInfo {PandaString(ap2), PandaString("")});
            }
        }
        aotManager->RegisterProfilePaths(std::move(entries));

        auto classCtxStr = aotManager->GetAppClassContext();
        auto &profiledMethods = aotManager->GetProfiledMethods();
        auto profiledMethodsFinal = aotManager->GetProfiledMethodsFinal();
        auto pathMapSnapshot = aotManager->GetProfiledPandaFilesMapSnapshot();

        PandaUnorderedMap<PandaString, PandaUnorderedSet<std::string>> pathToAbcs;
        for (const auto &[abc, pathInfo] : pathMapSnapshot) {
            pathToAbcs[pathInfo.curProfilePath].insert(std::string(abc));
        }

        ProfilingSaver profileSaver;
        for (auto &[profilePath, abcSet] : pathToAbcs) {
            profileSaver.SaveProfile(profilePath, classCtxStr, profiledMethods, profiledMethodsFinal, allPandaFiles,
                                     abcSet);
        }
    }

    // Verify: ap1 should have profiled methods from abc1 execution
    EXPECT_GT(CountProfileMethods(ap1), 0U);

    // Key assertion: ap1's PandaFiles section should contain ALL panda files (including abc2),
    // not just the ones whose methods were saved, so inline cache cross-file pfIdx is valid
    EXPECT_GE(CountProfilePandaFiles(ap1), 2U);

    CleanupFile(abc1);
    CleanupFile(abc2);
    CleanupFile(ap1);
    CleanupFile(ap2);
    Runtime::Destroy();
}

// NOLINTEND(readability-magic-numbers)
}  // namespace ark::test
