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

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

#include "include/mem/panda_containers.h"
#include "include/method.h"
#include "include/runtime.h"
#include "jit/libprofile/aot_profiling_data.h"
#include "jit/libprofile/pgo_class_context_utils.h"
#include "jit/profiling_loader.h"
#include "jit/profiling_saver.h"
#include "libarkbase/macros.h"
#include "panda_runner.h"
#include "runtime/jit/profiling_data.h"
#include "unit_test.h"

namespace ark::test {

static constexpr auto SOURCE = R"(
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

Class *FindClass(const std::string &name)
{
    PandaString descriptor;
    auto *thread = MTManagedThread::GetCurrent();
    thread->ManagedCodeBegin();
    auto cls = Runtime::GetCurrent()
                   ->GetClassLinker()
                   ->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                   ->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8(name.data()), &descriptor));
    thread->ManagedCodeEnd();
    return cls;
}

auto ToTuple(const BranchData &data)
{
    return std::make_tuple(data.GetPc(), data.GetTakenCounter(), data.GetNotTakenCounter());
}

auto ToTuple(const ThrowData &data)
{
    return std::make_tuple(data.GetPc(), data.GetTakenCounter());
}

auto ToTuple(const CallSiteInlineCache &data)
{
    auto classIdsVec = data.GetClassesCopy();
    std::set<ark::Class *> classIds(classIdsVec.begin(), classIdsVec.end());
    return std::make_tuple(data.GetBytecodePc(), classIds);
}

bool Equals(const BranchData &lhs, const BranchData &rhs)
{
    return ToTuple(lhs) == ToTuple(rhs);
}

bool Equals(const ThrowData &lhs, const ThrowData &rhs)
{
    return ToTuple(lhs) == ToTuple(rhs);
}

bool Equals(const CallSiteInlineCache &lhs, const CallSiteInlineCache &rhs)
{
    return ToTuple(lhs) == ToTuple(rhs);
}

template <typename T>
bool Equals(const Span<T> &lhs, const Span<T> &rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                      [](const T &l, const T &r) { return Equals(l, r); });
}

bool Equals(const ProfilingData &lhs, const ProfilingData &rhs)
{
    return Equals(lhs.GetBranchData(), rhs.GetBranchData()) && Equals(lhs.GetThrowData(), rhs.GetThrowData()) &&
           Equals(lhs.GetInlineCaches(), rhs.GetInlineCaches());
}

using MethodCountersSnapshot = std::map<uint32_t, std::pair<std::vector<std::tuple<uint32_t, uint64_t, uint64_t>>,
                                                            std::vector<std::tuple<uint32_t, uint64_t>>>>;

MethodCountersSnapshot SnapshotMethodCounters(const pgo::AotProfilingData::MethodsMap &methods)
{
    MethodCountersSnapshot snapshot;
    for (const auto &[methodId, methodProfile] : methods) {
        std::vector<std::tuple<uint32_t, uint64_t, uint64_t>> branchRows;
        for (const auto &br : methodProfile.GetBranchData()) {
            branchRows.emplace_back(br.pc, br.taken, br.notTaken);
        }
        std::vector<std::tuple<uint32_t, uint64_t>> throwRows;
        for (const auto &th : methodProfile.GetThrowData()) {
            throwRows.emplace_back(th.pc, th.taken);
        }
        snapshot.emplace(methodId, std::make_pair(std::move(branchRows), std::move(throwRows)));
    }
    return snapshot;
}

std::string BuildChecksumMismatchedClassCtx(std::string classCtx)
{
    size_t starPos = classCtx.find('*');
    if (starPos == std::string::npos) {
        return classCtx;
    }
    size_t checksumBegin = starPos + 1;
    size_t checksumEnd = classCtx.find(':', checksumBegin);
    if (checksumEnd == std::string::npos) {
        checksumEnd = classCtx.size();
    }
    if (checksumBegin < checksumEnd) {
        classCtx[checksumBegin] = (classCtx[checksumBegin] == '0') ? '1' : '0';
    }
    return classCtx;
}

void AssertMergedCounters(const pgo::AotProfilingData::AotMethodProfilingData &mergedProfile,
                          const ProfilingData *runtimeProfile,
                          const pgo::AotProfilingData::AotMethodProfilingData *oldProfile)
{
    std::unordered_map<uint32_t, std::pair<uint64_t, uint64_t>> expectedBranches;
    if (runtimeProfile != nullptr) {
        for (const auto &br : runtimeProfile->GetBranchData()) {
            expectedBranches[static_cast<uint32_t>(br.GetPc())] = {static_cast<uint64_t>(br.GetTakenCounter()),
                                                                   static_cast<uint64_t>(br.GetNotTakenCounter())};
        }
    }
    if (oldProfile != nullptr) {
        for (const auto &br : oldProfile->GetBranchData()) {
            auto &entry = expectedBranches[br.pc];
            entry.first += br.taken;
            entry.second += br.notTaken;
        }
    }

    std::unordered_map<uint32_t, std::pair<uint64_t, uint64_t>> mergedBranches;
    for (const auto &br : mergedProfile.GetBranchData()) {
        mergedBranches[br.pc] = {br.taken, br.notTaken};
    }
    ASSERT_EQ(mergedBranches.size(), expectedBranches.size());
    for (const auto &[pc, expected] : expectedBranches) {
        auto it = mergedBranches.find(pc);
        ASSERT_NE(it, mergedBranches.end()) << "missing merged branch entry at pc " << pc;
        EXPECT_EQ(it->second.first, expected.first);
        EXPECT_EQ(it->second.second, expected.second);
    }

    std::unordered_map<uint32_t, uint64_t> expectedThrows;
    if (runtimeProfile != nullptr) {
        for (const auto &thr : runtimeProfile->GetThrowData()) {
            expectedThrows[static_cast<uint32_t>(thr.GetPc())] = static_cast<uint64_t>(thr.GetTakenCounter());
        }
    }
    if (oldProfile != nullptr) {
        for (const auto &thr : oldProfile->GetThrowData()) {
            expectedThrows[thr.pc] += thr.taken;
        }
    }

    std::unordered_map<uint32_t, uint64_t> mergedThrows;
    for (const auto &thr : mergedProfile.GetThrowData()) {
        mergedThrows[thr.pc] = thr.taken;
    }
    ASSERT_EQ(mergedThrows.size(), expectedThrows.size());
    for (const auto &[pc, expectedTaken] : expectedThrows) {
        auto it = mergedThrows.find(pc);
        ASSERT_NE(it, mergedThrows.end()) << "missing merged throw entry at pc " << pc;
        EXPECT_EQ(it->second, expectedTaken);
    }
}

class ProfilingRuntimeSaveMergeTest : public ::testing::Test {
public:
    void SetUp() override
    {
        std::error_code ec;
        auto tmpDir = std::filesystem::temp_directory_path(ec);
        ASSERT_FALSE(ec);
        ASSERT_FALSE(tmpDir.empty());

        auto *testInfo = ::testing::UnitTest::GetInstance()->current_test_info();
        ASSERT_NE(testInfo, nullptr);
        artifactsDir_ = tmpDir / "ark_profiling_runtime_save_merge";
        artifactsDir_.append(testInfo->test_suite_name());
        artifactsDir_.append(testInfo->name());

        std::filesystem::remove_all(artifactsDir_, ec);
        ASSERT_FALSE(ec);
        std::filesystem::create_directories(artifactsDir_, ec);
        ASSERT_FALSE(ec);
        ASSERT_TRUE(std::filesystem::exists(artifactsDir_));
    }

    void TearDown() override
    {
        if (artifactsDir_.empty()) {
            return;
        }
        std::error_code ec;
        std::filesystem::remove_all(artifactsDir_, ec);
        artifactsDir_.clear();
    }

    std::string GetRuntimeClassCtx() const
    {
        auto *runtime = Runtime::GetCurrent();
        auto bootCtx = runtime->GetClassLinker()->GetAotManager()->GetBootClassContext();
        auto appCtx = runtime->GetClassLinker()->GetAotManager()->GetAppClassContext();
        std::string out(bootCtx.c_str(), bootCtx.size());
        out.push_back(':');
        out.append(appCtx.c_str(), appCtx.size());
        return out;
    }

    void CollectProfile()
    {
        runner_.GetRuntimeOptions().SetIncrementalProfilesaverEnabled(false);
        auto runtime = runner_.CreateRuntime();
        runner_.Run(runtime, SOURCE, std::vector<std::string> {});
    }

    void SaveProfile()
    {
        auto *runtime = Runtime::GetCurrent();
        if (!runtime->GetClassLinker()->GetAotManager()->HasProfiledMethods()) {
            return;
        }
        classCtxStr_ = GetRuntimeClassCtx();
        SaveProfileWithClassCtx(classCtxStr_);
    }

    void SaveProfileWithClassCtx(const std::string &classCtx)
    {
        auto *runtime = Runtime::GetCurrent();
        if (!runtime->GetClassLinker()->GetAotManager()->HasProfiledMethods()) {
            return;
        }
        classCtxStr_ = classCtx;
        ProfilingSaver profileSaver;
        auto &writtenMethods = runtime->GetClassLinker()->GetAotManager()->GetProfiledMethods();
        auto writtenMethodsFinal = runtime->GetClassLinker()->GetAotManager()->GetProfiledMethodsFinal();
        auto profiledPandaFiles = runtime->GetClassLinker()->GetAotManager()->GetProfiledPandaFiles();
        profileSaver.SaveProfile(PandaString(pgoFilePath_), PandaString(classCtx), writtenMethods, writtenMethodsFinal,
                                 profiledPandaFiles, profiledPandaFiles);
    }

    void LoadProfile(ProfilingLoader &profilingLoader)
    {
        auto profileCtxOrError = profilingLoader.LoadProfile(PandaString(pgoFilePath_));
        if (!profileCtxOrError) {
            std::cerr << profileCtxOrError.Error();
        }
        ASSERT_TRUE(profileCtxOrError.HasValue());

        auto buildCtxMap = [](std::string_view ctx, std::map<std::string, std::string> &ctxMap, std::string &error) {
            return pgo::PgoClassContextUtils::Parse(ctx, ctxMap, &error);
        };

        std::map<std::string, std::string> runtimeCtxMap;
        std::map<std::string, std::string> fileCtxMap;
        std::string parseError;
        ASSERT_TRUE(buildCtxMap(classCtxStr_, runtimeCtxMap, parseError)) << parseError;
        ASSERT_TRUE(buildCtxMap(*profileCtxOrError, fileCtxMap, parseError)) << parseError;
        ASSERT_EQ(fileCtxMap, runtimeCtxMap);
    }

    ark::Class *ClassResolver([[maybe_unused]] uint32_t classIdx, [[maybe_unused]] size_t fileIdx)
    {
        return FindClass("_GLOBAL");
    }

    void InitPGOFilePath(const std::string &path)
    {
        ASSERT_FALSE(artifactsDir_.empty());
        auto filePath = artifactsDir_;
        filePath.append(path);
        pgoFilePath_ = filePath.string();
        std::error_code ec;
        std::filesystem::remove(filePath, ec);
        ASSERT_FALSE(ec);
        ASSERT_FALSE(pgoFilePath_.empty());
    }

    void DestroyRuntime()
    {
        Runtime::Destroy();
    }

    void SetCompilerProfilingThreshold(uint32_t value)
    {
        runner_.GetRuntimeOptions().SetCompilerProfilingThreshold(value);
    }

private:
    PandaRunner runner_;
    std::map<std::string, ark::Class *> classMap_;
    std::string pgoFilePath_;
    std::string classCtxStr_;
    std::filesystem::path artifactsDir_;
};

TEST_F(ProfilingRuntimeSaveMergeTest, ProfileAddAndMergeTest)
{
    InitPGOFilePath("profileaddandoverwrite.ap");

    // first run
    const uint32_t highThreshold = 10U;
    SetCompilerProfilingThreshold(highThreshold);
    CollectProfile();
    SaveProfile();
    DestroyRuntime();

    // second run
    SetCompilerProfilingThreshold(0U);
    CollectProfile();
    {
        ProfilingLoader profilingLoader0;
        LoadProfile(profilingLoader0);
        auto &methodsProfile0 = profilingLoader0.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile0.size(), 1U);
        SaveProfile();
        ProfilingLoader profilingLoader1;
        LoadProfile(profilingLoader1);
        auto &methodsProfile1 = profilingLoader1.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile1.size(), 2U);

        // add profile data of main (new profiled)
        // overwrite profile data of foo
        auto globalCls = FindClass("_GLOBAL");
        auto mainMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("main"));
        auto fooMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("foo"));
        EXPECT_FALSE(mainMtd->GetProfilingData()->IsUpdateSinceLastSave());
        EXPECT_FALSE(fooMtd->GetProfilingData()->IsUpdateSinceLastSave());

        for (auto &[methodId, methodProfile] : methodsProfile1) {
            EXPECT_EQ(methodId, methodProfile.GetMethodIdx());
            if (methodId == mainMtd->GetFileId().GetOffset()) {
                // check data of main, same as merged(old + second profiled) on counters
                AssertMergedCounters(methodProfile, mainMtd->GetProfilingData(), nullptr);
            } else if (methodId == fooMtd->GetFileId().GetOffset()) {
                // check data of foo, same as merged(old + second profiled) on counters
                auto methodProfile0 = methodsProfile0.begin()->second;
                AssertMergedCounters(methodProfile, fooMtd->GetProfilingData(), &methodProfile0);
            }
        }
    }
    DestroyRuntime();
}

TEST_F(ProfilingRuntimeSaveMergeTest, ProfileNoDiskFastPathTest)
{
    InitPGOFilePath("profile_nodisk_fastpath.ap");

    // Single run with no existing profile file: save path should take the no-disk fast path.
    SetCompilerProfilingThreshold(0U);
    CollectProfile();
    SaveProfile();

    {
        ProfilingLoader profilingLoader;
        LoadProfile(profilingLoader);
        auto &methodsProfile = profilingLoader.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile.size(), 2U);

        auto globalCls = FindClass("_GLOBAL");
        auto mainMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("main"));
        auto fooMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("foo"));
        ASSERT_NE(mainMtd, nullptr);
        ASSERT_NE(fooMtd, nullptr);
        ASSERT_NE(mainMtd->GetProfilingData(), nullptr);
        ASSERT_NE(fooMtd->GetProfilingData(), nullptr);

        for (auto &[methodId, methodProfile] : methodsProfile) {
            EXPECT_EQ(methodId, methodProfile.GetMethodIdx());
            if (methodId == mainMtd->GetFileId().GetOffset()) {
                AssertMergedCounters(methodProfile, mainMtd->GetProfilingData(), nullptr);
            } else if (methodId == fooMtd->GetFileId().GetOffset()) {
                AssertMergedCounters(methodProfile, fooMtd->GetProfilingData(), nullptr);
            }
        }
    }
    DestroyRuntime();
}

TEST_F(ProfilingRuntimeSaveMergeTest, ProfileKeepAndMergeTest)
{
    InitPGOFilePath("profilekeepandoverwrite.ap");

    // first run
    SetCompilerProfilingThreshold(0U);
    CollectProfile();
    SaveProfile();
    DestroyRuntime();

    // second run
    const uint32_t highThreshold = 10U;
    SetCompilerProfilingThreshold(highThreshold);
    CollectProfile();
    {
        ProfilingLoader profilingLoader0;
        LoadProfile(profilingLoader0);
        auto &methodsProfile0 = profilingLoader0.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile0.size(), 2U);
        SaveProfile();
        ProfilingLoader profilingLoader1;
        LoadProfile(profilingLoader1);
        auto &methodsProfile1 = profilingLoader1.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile1.size(), 2U);

        // keep profile data of main (old profield)
        // overwrite profile data of foo
        auto globalCls = FindClass("_GLOBAL");
        auto mainMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("main"));
        auto fooMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("foo"));
        EXPECT_FALSE(fooMtd->GetProfilingData()->IsUpdateSinceLastSave());

        for (auto &[methodId, methodProfile] : methodsProfile1) {
            EXPECT_EQ(methodId, methodProfile.GetMethodIdx());
            if (methodId == mainMtd->GetFileId().GetOffset()) {
                // check data of main, same as merged(old + second profiled) on counters
                auto methodProfile0 = methodsProfile0.begin()->first == mainMtd->GetFileId().GetOffset()
                                          ? methodsProfile0.begin()->second
                                          : methodsProfile0.rbegin()->second;
                AssertMergedCounters(methodProfile, mainMtd->GetProfilingData(), &methodProfile0);
            } else if (methodId == fooMtd->GetFileId().GetOffset()) {
                // check data of foo, same as merged(old + second profiled) on counters
                auto oldFooIt = methodsProfile0.find(fooMtd->GetFileId().GetOffset());
                ASSERT_NE(oldFooIt, methodsProfile0.end());
                AssertMergedCounters(methodProfile, fooMtd->GetProfilingData(), &oldFooIt->second);
            }
        }
    }
    DestroyRuntime();
}

TEST_F(ProfilingRuntimeSaveMergeTest, ProfileClassCtxMismatchFallbackToCurrentRuntimeData)
{
    InitPGOFilePath("profile_classctx_mismatch_fallback.ap");

    // First run writes baseline disk profile.
    SetCompilerProfilingThreshold(0U);
    CollectProfile();
    SaveProfile();
    DestroyRuntime();

    // Second run intentionally saves with classCtx checksum mismatch against existing disk profile.
    SetCompilerProfilingThreshold(0U);
    CollectProfile();
    std::string runtimeClassCtx = GetRuntimeClassCtx();
    std::string mismatchedClassCtx = BuildChecksumMismatchedClassCtx(runtimeClassCtx);
    ASSERT_NE(mismatchedClassCtx, runtimeClassCtx);

    SaveProfileWithClassCtx(mismatchedClassCtx);
    {
        ProfilingLoader profilingLoader;
        LoadProfile(profilingLoader);
        auto &methodsProfile = profilingLoader.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methodsProfile.size(), 2U);

        // Fallback semantics: saved profile should equal current runtime counters (not old+new merge).
        auto globalCls = FindClass("_GLOBAL");
        auto mainMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("main"));
        auto fooMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("foo"));
        ASSERT_NE(mainMtd, nullptr);
        ASSERT_NE(fooMtd, nullptr);
        ASSERT_NE(mainMtd->GetProfilingData(), nullptr);
        ASSERT_NE(fooMtd->GetProfilingData(), nullptr);

        for (auto &[methodId, methodProfile] : methodsProfile) {
            EXPECT_EQ(methodId, methodProfile.GetMethodIdx());
            if (methodId == mainMtd->GetFileId().GetOffset()) {
                AssertMergedCounters(methodProfile, mainMtd->GetProfilingData(), nullptr);
            } else if (methodId == fooMtd->GetFileId().GetOffset()) {
                AssertMergedCounters(methodProfile, fooMtd->GetProfilingData(), nullptr);
            }
        }
    }
    DestroyRuntime();
}

TEST_F(ProfilingRuntimeSaveMergeTest, ProfileSaveCommitsRuntimeState)
{
    InitPGOFilePath("profile_save_commit_state.ap");

    SetCompilerProfilingThreshold(0U);
    CollectProfile();
    SaveProfile();

    auto globalCls = FindClass("_GLOBAL");
    auto mainMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("main"));
    auto fooMtd = globalCls->GetClassMethod(utf::CStringAsMutf8("foo"));
    ASSERT_NE(mainMtd, nullptr);
    ASSERT_NE(fooMtd, nullptr);
    ASSERT_NE(mainMtd->GetProfilingData(), nullptr);
    ASSERT_NE(fooMtd->GetProfilingData(), nullptr);

    // Save success should commit runtime profiling state.
    EXPECT_FALSE(mainMtd->GetProfilingData()->IsUpdateSinceLastSave());
    EXPECT_FALSE(fooMtd->GetProfilingData()->IsUpdateSinceLastSave());
    EXPECT_TRUE(mainMtd->GetProfilingData()->HasLastSaved());
    EXPECT_TRUE(fooMtd->GetProfilingData()->HasLastSaved());

    MethodCountersSnapshot afterFirstSave;
    {
        ProfilingLoader profilingLoader;
        LoadProfile(profilingLoader);
        auto &methods = profilingLoader.GetAotProfilingData().GetAllMethods().begin()->second;
        EXPECT_EQ(methods.size(), 2U);
        afterFirstSave = SnapshotMethodCounters(methods);
    }

    // No extra runtime execution: second save should be a no-op in persisted counters.
    SaveProfile();
    EXPECT_FALSE(mainMtd->GetProfilingData()->IsUpdateSinceLastSave());
    EXPECT_FALSE(fooMtd->GetProfilingData()->IsUpdateSinceLastSave());
    EXPECT_TRUE(mainMtd->GetProfilingData()->HasLastSaved());
    EXPECT_TRUE(fooMtd->GetProfilingData()->HasLastSaved());

    {
        ProfilingLoader profilingLoader;
        LoadProfile(profilingLoader);
        auto &methods = profilingLoader.GetAotProfilingData().GetAllMethods().begin()->second;
        auto afterSecondSave = SnapshotMethodCounters(methods);
        EXPECT_EQ(afterSecondSave, afterFirstSave);
    }
    DestroyRuntime();
}
}  // namespace ark::test
