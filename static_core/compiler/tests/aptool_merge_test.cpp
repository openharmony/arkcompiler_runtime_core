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

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#if !defined(PANDA_TARGET_WINDOWS)
#include <sys/wait.h>
#endif

#include "compiler/tools/aptool/merge/merge_result.h"
#include "libarkbase/os/file.h"
#include "runtime/jit/libprofile/aot_profiling_data.h"
#include "runtime/jit/libprofile/pgo_file_builder.h"
#include "unit_test.h"

namespace ark::compiler {

namespace {

using BranchData = ark::pgo::AotProfilingData::AotBranchData;
using ThrowData = ark::pgo::AotProfilingData::AotThrowData;
using InlineCache = ark::pgo::AotProfilingData::AotCallSiteInlineCache;

struct MethodSpec {
    ark::pgo::PandaFileIdxType pfIdx {0};
    uint32_t methodId {0};
    uint32_t classIdx {7U};
    PandaVector<InlineCache> inlineCaches;
    PandaVector<BranchData> branches;
    PandaVector<ThrowData> throwsData;

    MethodSpec() = default;

    MethodSpec(uint32_t id, uint64_t branchTaken, uint64_t branchNotTaken, uint64_t throwTaken) : methodId(id)
    {
        branches.push_back({0x10U, branchTaken, branchNotTaken});  // NOLINT
        throwsData.push_back({0x20U, throwTaken});                 // NOLINT
    }
};

std::string QuoteShellArg(const std::string &value)
{
#if defined(PANDA_TARGET_WINDOWS)
    std::string result;
    result.reserve(value.size() + 2U);
    result.push_back('"');
    for (char ch : value) {
        if (ch == '"') {
            result += "\\\"";
        } else {
            result.push_back(ch);
        }
    }
    result.push_back('"');
    return result;
#else
    std::string result;
    result.reserve(value.size() + 2U);
    result.push_back('\'');
    for (char ch : value) {
        if (ch == '\'') {
            result += "'\"'\"'";
        } else {
            result.push_back(ch);
        }
    }
    result.push_back('\'');
    return result;
#endif
}

std::string QuoteShellArg(const std::filesystem::path &path)
{
    return QuoteShellArg(path.string());
}

std::string FindArkAptoolBinary()
{
    auto execPathOrError = os::file::File::GetExecutablePath();
    if (!execPathOrError) {
        return {};
    }
    std::filesystem::path current(execPathOrError.Value());
    current = current.parent_path();
    constexpr size_t maxParentDepth = 5;
    for (size_t i = 0; i < maxParentDepth && !current.empty(); i++) {
        auto candidate = current / "bin" / "ark_aptool";
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
        current = current.parent_path();
    }
    return {};
}

int DecodeProcessExitCode(int status)
{
    if (status == -1) {
        return -1;
    }
#if defined(PANDA_TARGET_WINDOWS)
    return status;
#else
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    return status;
#endif
}

int MergeExitCodeValue(ark::aptool::merge::MergeExitCode code)
{
    return static_cast<int>(code);
}

void CreateProfile(const std::filesystem::path &path, std::string_view classCtx,
                   const PandaVector<PandaString> &pandaFiles, PandaVector<MethodSpec> methods)
{
    ark::pgo::AotProfilingData profilingData;
    profilingData.AddPandaFiles(pandaFiles);
    for (auto &method : methods) {
        ark::pgo::AotProfilingData::AotMethodProfilingData methodProfile(
            method.methodId, method.classIdx, std::move(method.inlineCaches), std::move(method.branches),
            std::move(method.throwsData));
        profilingData.AddMethod(method.pfIdx, method.methodId, std::move(methodProfile));
    }

    ark::pgo::AotPgoFile writer;
    auto written = writer.Save(PandaString(path.string().c_str()), &profilingData, PandaString(classCtx.data()));
    ASSERT_NE(written, 0U);
}

void CreateProfile(const std::filesystem::path &path, std::string_view classCtx, const std::vector<MethodSpec> &methods)
{
    PandaVector<PandaString> pandaFiles;
    pandaFiles.emplace_back("app.abc");

    PandaVector<MethodSpec> methodSpecs;
    methodSpecs.reserve(methods.size());
    for (const auto &method : methods) {
        methodSpecs.emplace_back(method);
    }
    CreateProfile(path, classCtx, pandaFiles, std::move(methodSpecs));
}

InlineCache MakeInlineCache(uint32_t pc, std::initializer_list<std::pair<uint32_t, ark::pgo::PandaFileIdxType>> classes)
{
    InlineCache inlineCache {};
    inlineCache.pc = pc;
    InlineCache::ClearClasses(inlineCache.classes);
    size_t idx = 0;
    for (const auto &cls : classes) {
        if (idx >= InlineCache::CLASSES_COUNT) {
            break;
        }
        inlineCache.classes[idx++] = cls;
    }
    return inlineCache;
}

const ark::pgo::AotProfilingData::AotMethodProfilingData *FindMethod(ark::pgo::AotProfilingData &profile,
                                                                     uint32_t methodId)
{
    const auto &allMethods = profile.GetAllMethods();
    auto pfIt = allMethods.find(0);
    if (pfIt == allMethods.end()) {
        return nullptr;
    }
    auto methodIt = pfIt->second.find(methodId);
    if (methodIt == pfIt->second.end()) {
        return nullptr;
    }
    return &methodIt->second;
}

const ark::pgo::AotProfilingData::AotMethodProfilingData *FindMethod(ark::pgo::AotProfilingData &profile,
                                                                     std::string_view pandaFile, uint32_t methodId)
{
    auto pfIdx = profile.GetPandaFileIdxByName(pandaFile);
    if (pfIdx < 0) {
        return nullptr;
    }
    const auto &allMethods = profile.GetAllMethods();
    auto pfIt = allMethods.find(pfIdx);
    if (pfIt == allMethods.end()) {
        return nullptr;
    }
    auto methodIt = pfIt->second.find(methodId);
    if (methodIt == pfIt->second.end()) {
        return nullptr;
    }
    return &methodIt->second;
}

}  // namespace

class AptoolMergeTest : public PandaRuntimeTest {
public:
    AptoolMergeTest() = default;
    ~AptoolMergeTest() override = default;

protected:
    void TearDown() override
    {
        PandaRuntimeTest::TearDown();
        auto keep = std::getenv("APMERGE_KEEP_ARTIFACTS");
        if (keep != nullptr && std::string_view(keep) == "1") {
            return;
        }
        if (!artifactsRoot_.empty()) {
            std::error_code ec;
            std::filesystem::remove_all(artifactsRoot_, ec);
            artifactsRoot_.clear();
        }
    }

    std::filesystem::path PrepareArtifactsDir(const std::string &name)
    {
        auto base = GetArtifactsRoot();
        if (base.empty() || name.empty()) {
            return {};
        }
        auto dir = base;
        dir.append(name);
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
        if (ec) {
            ADD_FAILURE() << "Failed to clear artifacts dir '" << dir.string() << "': " << ec.message();
            return {};
        }
        std::filesystem::create_directories(dir, ec);
        if (ec) {
            ADD_FAILURE() << "Failed to create artifacts dir '" << dir.string() << "': " << ec.message();
            return {};
        }
        if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
            ADD_FAILURE() << "Artifacts dir is not ready: '" << dir.string() << "'";
            return {};
        }
        return dir;
    }

    int RunMerge(const std::vector<std::string> &args, const std::filesystem::path *stderrPath = nullptr) const
    {
        auto toolPath = FindArkAptoolBinary();
        if (toolPath.empty()) {
            ADD_FAILURE() << "Unable to locate ark_aptool binary";
            return -1;
        }
        std::ostringstream cmd;
        cmd << QuoteShellArg(toolPath) << " merge";
        for (const auto &arg : args) {
            cmd << " " << arg;
        }
        if (stderrPath != nullptr) {
            cmd << " 2> " << QuoteShellArg(*stderrPath);
        }
        return DecodeProcessExitCode(std::system(cmd.str().c_str()));
    }

private:
    std::filesystem::path GetArtifactsRoot()
    {
        if (artifactsRoot_.empty()) {
            auto execPathOrError = os::file::File::GetExecutablePath();
            if (!execPathOrError) {
                ADD_FAILURE() << "Cannot determine test binary path";
                return {};
            }
            std::filesystem::path execPath(execPathOrError.Value());
            artifactsRoot_ = execPath.parent_path() / "apmerge_test_artifacts";
        }
        std::error_code ec;
        std::filesystem::create_directories(artifactsRoot_, ec);
        if (ec) {
            ADD_FAILURE() << "Failed to create artifacts root '" << artifactsRoot_.string() << "': " << ec.message();
            return {};
        }
        if (!std::filesystem::exists(artifactsRoot_) || !std::filesystem::is_directory(artifactsRoot_)) {
            ADD_FAILURE() << "Artifacts root is not ready: '" << artifactsRoot_.string() << "'";
            return {};
        }
        return artifactsRoot_;
    }

    std::filesystem::path artifactsRoot_;
};

TEST_F(AptoolMergeTest, MergeAggregatesCountersAndPreservesNewMethods)
{
    auto workspace = PrepareArtifactsDir("aggregate");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.ap";
    auto out = workspace / "merged.ap";

    CreateProfile(first, "app.abc*1111", {{100U, 1U, 2U, 3U}});
    CreateProfile(second, "app.abc*1111", {{100U, 4U, 5U, 6U}, {200U, 7U, 8U, 9U}});

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, 0) << "Expected aptool merge to succeed";

    PandaString classCtx;
    PandaVector<PandaString> pandaFiles;
    auto loaded = ark::pgo::AotPgoFile::Load(PandaString(out.string().c_str()), classCtx, pandaFiles);
    ASSERT_TRUE(loaded);
    ASSERT_EQ(pandaFiles.size(), 1U);
    EXPECT_EQ(std::string_view(pandaFiles[0].c_str(), pandaFiles[0].size()), "app.abc");

    auto *method100 = FindMethod(*loaded, 100U);
    ASSERT_NE(method100, nullptr);
    ASSERT_EQ(method100->GetBranchData().size(), 1U);
    ASSERT_EQ(method100->GetThrowData().size(), 1U);
    EXPECT_EQ(method100->GetBranchData()[0].taken, 5U);
    EXPECT_EQ(method100->GetBranchData()[0].notTaken, 7U);
    EXPECT_EQ(method100->GetThrowData()[0].taken, 9U);

    auto *method200 = FindMethod(*loaded, 200U);
    ASSERT_NE(method200, nullptr);
    ASSERT_EQ(method200->GetBranchData().size(), 1U);
    ASSERT_EQ(method200->GetThrowData().size(), 1U);
    EXPECT_EQ(method200->GetBranchData()[0].taken, 7U);
    EXPECT_EQ(method200->GetBranchData()[0].notTaken, 8U);
    EXPECT_EQ(method200->GetThrowData()[0].taken, 9U);
}

TEST_F(AptoolMergeTest, RejectsLessThanTwoDistinctInputs)
{
    auto workspace = PrepareArtifactsDir("distinct_input_required");
    ASSERT_FALSE(workspace.empty());
    auto only = workspace / "single.ap";
    auto out = workspace / "merged.ap";

    CreateProfile(only, "app.abc*1111", {{100U, 1U, 1U, 1U}});

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(only), "--ap-path " + QuoteShellArg(only), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, MergeExitCodeValue(ark::aptool::merge::MergeExitCode::USAGE_ERROR))
        << "Expected aptool merge to reject non-distinct inputs";
}

TEST_F(AptoolMergeTest, RejectsClassContextChecksumMismatch)
{
    auto workspace = PrepareArtifactsDir("class_ctx_mismatch");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "a.ap";
    auto second = workspace / "b.ap";
    auto out = workspace / "merged.ap";

    CreateProfile(first, "app.abc*1111", {{100U, 1U, 2U, 3U}});
    CreateProfile(second, "app.abc*2222", {{100U, 4U, 5U, 6U}});

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, MergeExitCodeValue(ark::aptool::merge::MergeExitCode::PROFILE_COMPATIBILITY_ERROR))
        << "Expected aptool merge to reject class context checksum mismatch";
}

TEST_F(AptoolMergeTest, MergeSaturatesCountersAtUint64Max)
{
    auto workspace = PrepareArtifactsDir("saturate_counters");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.ap";
    auto out = workspace / "merged.ap";

    constexpr uint64_t max = std::numeric_limits<uint64_t>::max();
    CreateProfile(first, "app.abc*1111", {{100U, max - 1U, max - 2U, max - 3U}});
    CreateProfile(second, "app.abc*1111", {{100U, 9U, 10U, 11U}});

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, 0) << "Expected aptool merge to succeed";

    PandaString classCtx;
    PandaVector<PandaString> pandaFiles;
    auto loaded = ark::pgo::AotPgoFile::Load(PandaString(out.string().c_str()), classCtx, pandaFiles);
    ASSERT_TRUE(loaded);

    auto *method100 = FindMethod(*loaded, 100U);
    ASSERT_NE(method100, nullptr);
    EXPECT_EQ(method100->GetBranchData()[0].taken, max);
    EXPECT_EQ(method100->GetBranchData()[0].notTaken, max);
    EXPECT_EQ(method100->GetThrowData()[0].taken, max);
}

TEST_F(AptoolMergeTest, MergeEscalatesInlineCacheToMegamorphicWhenTargetsExceedFour)
{
    auto workspace = PrepareArtifactsDir("ic_to_mega");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.ap";
    auto out = workspace / "merged.ap";

    PandaVector<PandaString> pandaFiles;
    pandaFiles.emplace_back("app.abc");

    PandaVector<MethodSpec> firstMethods;
    MethodSpec firstMethod;
    firstMethod.methodId = 100U;
    firstMethod.inlineCaches.push_back(MakeInlineCache(0x30U, {{10U, 0}, {11U, 0}, {12U, 0}, {13U, 0}}));
    firstMethods.push_back(std::move(firstMethod));

    PandaVector<MethodSpec> secondMethods;
    MethodSpec secondMethod;
    secondMethod.methodId = 100U;
    secondMethod.inlineCaches.push_back(MakeInlineCache(0x30U, {{14U, 0}}));
    secondMethods.push_back(std::move(secondMethod));

    CreateProfile(first, "app.abc*1111", pandaFiles, std::move(firstMethods));
    CreateProfile(second, "app.abc*1111", pandaFiles, std::move(secondMethods));

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, 0) << "Expected aptool merge to succeed";

    PandaString classCtx;
    PandaVector<PandaString> outPandaFiles;
    auto loaded = ark::pgo::AotPgoFile::Load(PandaString(out.string().c_str()), classCtx, outPandaFiles);
    ASSERT_TRUE(loaded);

    auto *method100 = FindMethod(*loaded, 100U);
    ASSERT_NE(method100, nullptr);
    ASSERT_EQ(method100->GetInlineCaches().size(), 1U);
    const auto &inlineCaches = method100->GetInlineCaches();
    const auto &inlineCache = inlineCaches[0];
    EXPECT_EQ(inlineCache.pc, 0x30U);
    EXPECT_EQ(inlineCache.classes[0].first, static_cast<uint32_t>(InlineCache::MEGAMORPHIC_FLAG));
    EXPECT_EQ(inlineCache.classes[0].second, -1);
}

// CC-OFFNXT(G.FUN.01-CPP) test validates cross-input pfIdx remap end to end.
TEST_F(AptoolMergeTest, MergeRemapsPandaFileIndicesInInlineCaches)
{
    auto workspace = PrepareArtifactsDir("pfidx_remap");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.ap";
    auto out = workspace / "merged.ap";

    PandaVector<PandaString> pandaFilesFirst;
    pandaFilesFirst.emplace_back("a.abc");
    pandaFilesFirst.emplace_back("b.abc");
    PandaVector<PandaString> pandaFilesSecond;
    pandaFilesSecond.emplace_back("b.abc");
    pandaFilesSecond.emplace_back("a.abc");

    PandaVector<MethodSpec> firstMethods;
    MethodSpec firstMethod;
    firstMethod.pfIdx = 0;
    firstMethod.methodId = 100U;
    firstMethod.inlineCaches.push_back(MakeInlineCache(0x40U, {{1000U, 1}}));
    firstMethods.push_back(std::move(firstMethod));

    PandaVector<MethodSpec> secondMethods;
    MethodSpec secondMethod;
    secondMethod.pfIdx = 1;
    secondMethod.methodId = 100U;
    secondMethod.inlineCaches.push_back(MakeInlineCache(0x40U, {{2000U, 0}}));
    secondMethods.push_back(std::move(secondMethod));

    CreateProfile(first, "a.abc*1111:b.abc*2222", pandaFilesFirst, std::move(firstMethods));
    CreateProfile(second, "b.abc*2222:a.abc*1111", pandaFilesSecond, std::move(secondMethods));

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, 0) << "Expected aptool merge to succeed";

    PandaString classCtx;
    PandaVector<PandaString> outPandaFiles;
    auto loaded = ark::pgo::AotPgoFile::Load(PandaString(out.string().c_str()), classCtx, outPandaFiles);
    ASSERT_TRUE(loaded);
    ASSERT_EQ(outPandaFiles.size(), 2U);
    EXPECT_EQ(std::string_view(outPandaFiles[0].c_str(), outPandaFiles[0].size()), "a.abc");
    EXPECT_EQ(std::string_view(outPandaFiles[1].c_str(), outPandaFiles[1].size()), "b.abc");

    auto pfIdxB = loaded->GetPandaFileIdxByName("b.abc");
    ASSERT_GE(pfIdxB, 0);
    auto *method100 = FindMethod(*loaded, "a.abc", 100U);
    ASSERT_NE(method100, nullptr);
    ASSERT_EQ(method100->GetInlineCaches().size(), 1U);

    bool hasTarget1000 = false;
    bool hasTarget2000 = false;
    for (const auto &cls : method100->GetInlineCaches()[0].classes) {
        if (cls.first == 1000U && cls.second == pfIdxB) {
            hasTarget1000 = true;
        }
        if (cls.first == 2000U && cls.second == pfIdxB) {
            hasTarget2000 = true;
        }
    }
    EXPECT_TRUE(hasTarget1000);
    EXPECT_TRUE(hasTarget2000);
}

TEST_F(AptoolMergeTest, RejectsMethodClassIdxMismatch)
{
    auto workspace = PrepareArtifactsDir("class_idx_mismatch");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.ap";
    auto out = workspace / "merged.ap";

    PandaVector<PandaString> pandaFiles;
    pandaFiles.emplace_back("app.abc");

    PandaVector<MethodSpec> firstMethods;
    MethodSpec firstMethod;
    firstMethod.methodId = 100U;
    firstMethod.classIdx = 7U;
    firstMethod.branches.push_back({0x10U, 1U, 1U});
    firstMethods.push_back(std::move(firstMethod));

    PandaVector<MethodSpec> secondMethods;
    MethodSpec secondMethod;
    secondMethod.methodId = 100U;
    secondMethod.classIdx = 8U;
    secondMethod.branches.push_back({0x10U, 1U, 1U});
    secondMethods.push_back(std::move(secondMethod));

    CreateProfile(first, "app.abc*1111", pandaFiles, std::move(firstMethods));
    CreateProfile(second, "app.abc*1111", pandaFiles, std::move(secondMethods));

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, MergeExitCodeValue(ark::aptool::merge::MergeExitCode::PROFILE_COMPATIBILITY_ERROR))
        << "Expected aptool merge to reject method class index mismatch";
}

TEST_F(AptoolMergeTest, RejectsUnparseableInputFile)
{
    auto workspace = PrepareArtifactsDir("invalid_input");
    ASSERT_FALSE(workspace.empty());
    auto valid = workspace / "valid.ap";
    auto invalid = workspace / "invalid.txt";
    auto out = workspace / "merged.ap";

    CreateProfile(valid, "app.abc*1111", {{100U, 1U, 1U, 1U}});
    {
        std::ofstream stream(invalid);
        stream << "not a pgo profile";
    }

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(valid), "--ap-path " + QuoteShellArg(invalid), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, MergeExitCodeValue(ark::aptool::merge::MergeExitCode::PROFILE_LOAD_ERROR))
        << "Expected aptool merge to reject unparseable input file";
}

TEST_F(AptoolMergeTest, AcceptsBapAndApInputs)
{
    auto workspace = PrepareArtifactsDir("bap_and_ap");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.bap";
    auto out = workspace / "merged.ap";

    CreateProfile(first, "app.abc*1111", {{100U, 1U, 2U, 3U}});
    CreateProfile(second, "app.abc*1111", {{100U, 4U, 5U, 6U}});

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, 0) << "Expected aptool merge to accept .bap and .ap inputs";

    PandaString classCtx;
    PandaVector<PandaString> pandaFiles;
    auto loaded = ark::pgo::AotPgoFile::Load(PandaString(out.string().c_str()), classCtx, pandaFiles);
    ASSERT_TRUE(loaded);
    auto *method100 = FindMethod(*loaded, 100U);
    ASSERT_NE(method100, nullptr);
    EXPECT_EQ(method100->GetBranchData()[0].taken, 5U);
}

TEST_F(AptoolMergeTest, AcceptsMapAndApInputs)
{
    auto workspace = PrepareArtifactsDir("map_and_ap");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.map";
    auto out = workspace / "merged.ap";

    CreateProfile(first, "app.abc*1111", {{100U, 1U, 2U, 3U}});
    CreateProfile(second, "app.abc*1111", {{100U, 4U, 5U, 6U}});

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, 0) << "Expected aptool merge to accept .map and .ap inputs";

    PandaString classCtx;
    PandaVector<PandaString> pandaFiles;
    auto loaded = ark::pgo::AotPgoFile::Load(PandaString(out.string().c_str()), classCtx, pandaFiles);
    ASSERT_TRUE(loaded);
    auto *method100 = FindMethod(*loaded, 100U);
    ASSERT_NE(method100, nullptr);
    EXPECT_EQ(method100->GetBranchData()[0].taken, 5U);
}

TEST_F(AptoolMergeTest, AcceptsMapOutputPath)
{
    auto workspace = PrepareArtifactsDir("map_output");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.ap";
    auto out = workspace / "merged.map";

    CreateProfile(first, "app.abc*1111", {{100U, 1U, 2U, 3U}});
    CreateProfile(second, "app.abc*1111", {{100U, 4U, 5U, 6U}});

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second), "--output " + QuoteShellArg(out)});
    ASSERT_EQ(ret, 0) << "Expected aptool merge to accept .map output path";

    PandaString classCtx;
    PandaVector<PandaString> pandaFiles;
    auto loaded = ark::pgo::AotPgoFile::Load(PandaString(out.string().c_str()), classCtx, pandaFiles);
    ASSERT_TRUE(loaded);
}

TEST_F(AptoolMergeTest, VerbosePrintsMergeSummary)
{
    auto workspace = PrepareArtifactsDir("verbose_summary");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.ap";
    auto out = workspace / "merged.ap";
    auto stderrPath = workspace / "merge_stderr.txt";

    CreateProfile(first, "app.abc*1111", {{100U, 1U, 2U, 3U}});
    CreateProfile(second, "app.abc*1111", {{200U, 4U, 5U, 6U}});

    int ret = RunMerge({"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second),
                        "--output " + QuoteShellArg(out), "--verbose"},
                       &stderrPath);
    ASSERT_EQ(ret, MergeExitCodeValue(ark::aptool::merge::MergeExitCode::SUCCESS))
        << "Expected aptool merge --verbose to succeed";

    std::ifstream stderrStream(stderrPath);
    ASSERT_TRUE(stderrStream.is_open()) << "Expected stderr capture file to exist";
    std::stringstream buffer;
    buffer << stderrStream.rdbuf();
    auto text = buffer.str();

    EXPECT_NE(text.find("Profile merged to " + out.string()), std::string::npos);
    EXPECT_NE(text.find("  Input profiles: 2"), std::string::npos);
    EXPECT_NE(text.find("  Methods: 2"), std::string::npos);
    EXPECT_NE(text.find("  Panda files: 1"), std::string::npos);
}

TEST_F(AptoolMergeTest, NonVerboseDoesNotPrintMergeSummary)
{
    auto workspace = PrepareArtifactsDir("non_verbose_summary");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.ap";
    auto out = workspace / "merged.ap";
    auto stderrPath = workspace / "merge_stderr.txt";

    CreateProfile(first, "app.abc*1111", {{100U, 1U, 2U, 3U}});
    CreateProfile(second, "app.abc*1111", {{200U, 4U, 5U, 6U}});

    int ret = RunMerge(
        {"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second), "--output " + QuoteShellArg(out)},
        &stderrPath);
    ASSERT_EQ(ret, MergeExitCodeValue(ark::aptool::merge::MergeExitCode::SUCCESS))
        << "Expected aptool merge without --verbose to succeed";

    std::ifstream stderrStream(stderrPath);
    ASSERT_TRUE(stderrStream.is_open()) << "Expected stderr capture file to exist";
    std::stringstream buffer;
    buffer << stderrStream.rdbuf();
    auto text = buffer.str();

    EXPECT_EQ(text.find("Profile merged to " + out.string()), std::string::npos);
    EXPECT_EQ(text.find("  Input profiles:"), std::string::npos);
    EXPECT_EQ(text.find("  Methods:"), std::string::npos);
    EXPECT_EQ(text.find("  Panda files:"), std::string::npos);
}

TEST_F(AptoolMergeTest, ReturnsProfileWriteErrorWhenOutputIsDirectory)
{
    auto workspace = PrepareArtifactsDir("write_error");
    ASSERT_FALSE(workspace.empty());
    auto first = workspace / "first.ap";
    auto second = workspace / "second.ap";

    CreateProfile(first, "app.abc*1111", {{100U, 1U, 2U, 3U}});
    CreateProfile(second, "app.abc*1111", {{200U, 4U, 5U, 6U}});

    // Use an existing directory as output path to trigger write failure.
    int ret = RunMerge({"--ap-path " + QuoteShellArg(first), "--ap-path " + QuoteShellArg(second),
                        "--output " + QuoteShellArg(workspace)});
    ASSERT_EQ(ret, MergeExitCodeValue(ark::aptool::merge::MergeExitCode::PROFILE_WRITE_ERROR))
        << "Expected aptool merge to report profile write error";
}

}  // namespace ark::compiler
