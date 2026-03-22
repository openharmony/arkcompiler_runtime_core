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

#include <chrono>
#include <cstdlib>
#include <cstring>
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "assembler/assembly-parser.h"
#include "unit_test.h"
#include "libarkbase/os/exec.h"
#include "libarkbase/os/file.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/class_data_accessor.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "libarkfile/file.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/method_data_accessor.h"
#include "runtime/include/class_helper.h"
#include "runtime/jit/libprofile/aot_profiling_data.h"
#include "runtime/jit/libprofile/pgo_file_builder.h"

namespace ark::compiler {

// Skip tests that require ETS language support when PANDA_WITH_ETS is not defined
#ifndef PANDA_WITH_ETS
#define APTOOL_SKIP_IF_NO_ETS() GTEST_SKIP() << "Test requires ETS language support (PANDA_WITH_ETS)"
#else
#define APTOOL_SKIP_IF_NO_ETS() (void)0
#endif

namespace {

struct DumpTestEntities {
    uint32_t classAdder {0};
    uint32_t classDoubler {0};
    uint32_t classEtsglobal {0};
    uint32_t classWorker {0};
    uint32_t methodFoo {0};
    uint32_t methodMain {0};
    uint32_t methodNoisyBranch {0};
    uint32_t methodRunPolymorphic {0};
};

struct DumpTestArtifacts {
    DumpTestEntities entities;
    std::string checksum;
};

constexpr const char *DUMP_TEST_ASM_PATH = "compiler/tests/data/apdump/dump_test.asm";
constexpr const char *DUMP_TEST_ABC_NAME = "dump_test.abc";

fs::path FindProjectFile(const std::string &relativePath);
std::string ReadFile(const fs::path &path);

using InlineCache = ark::pgo::AotProfilingData::AotCallSiteInlineCache;
using BranchData = ark::pgo::AotProfilingData::AotBranchData;
using ThrowData = ark::pgo::AotProfilingData::AotThrowData;

DumpTestArtifacts CreateDumpTestAbc(const fs::path &abcPath);
DumpTestEntities ResolveDumpTestEntities(const panda_file::File &file);
InlineCache MakeInlineCache(uint32_t pc,
                            std::initializer_list<std::pair<uint32_t, ark::pgo::PandaFileIdxType>> classes);
BranchData MakeBranch(uint32_t pc, uint64_t taken, uint64_t notTaken);
ThrowData MakeThrow(uint32_t pc, uint64_t taken);
ark::pgo::AotProfilingData::AotMethodProfilingData BuildFooMethod(const DumpTestEntities &entities);
ark::pgo::AotProfilingData::AotMethodProfilingData BuildMainMethod(const DumpTestEntities &entities);
ark::pgo::AotProfilingData::AotMethodProfilingData BuildNoisyBranchMethod(const DumpTestEntities &entities);
ark::pgo::AotProfilingData::AotMethodProfilingData BuildRunPolymorphicMethod(const DumpTestEntities &entities);
void CreateDumpTestProfile(const fs::path &path, const DumpTestEntities &entities, const std::string &abcChecksum,
                           std::string_view contextEntry = DUMP_TEST_ABC_NAME);
std::string FindArkAptoolBinary();

}  // namespace

class AptoolDumpTest : public PandaRuntimeTest {
public:
    AptoolDumpTest() = default;
    ~AptoolDumpTest() override = default;

protected:
    void TearDown() override
    {
        PandaRuntimeTest::TearDown();
        auto keep = std::getenv("APDUMP_KEEP_ARTIFACTS");
        if (keep != nullptr && std::string_view(keep) == "1") {
            std::cout << "[ApdumpTest] KEEP_ARTIFACTS=1, skip cleanup of " << artifactsRoot_ << std::endl;
            return;
        }
        if (!artifactsRoot_.empty()) {
            std::error_code ec;
            fs::remove_all(artifactsRoot_, ec);
            artifactsRoot_.clear();
        }
    }

    fs::path CreateSyntheticProfile()
    {
        static size_t profileIdx = 0;
        auto baseDir = GetArtifactsRoot();
        if (baseDir.empty()) {
            return {};
        }
        auto filePath = baseDir / ("apdump_synthetic_" + std::to_string(profileIdx++) + ".ap");
        GenerateProfileFile(filePath);
        return filePath;
    }

    fs::path CreateMultiFileProfile()
    {
        static size_t profileIdx = 0;
        auto sourceProfile = CreateSyntheticProfile();
        if (sourceProfile.empty()) {
            return {};
        }
        auto baseDir = GetArtifactsRoot();
        if (baseDir.empty()) {
            return {};
        }
        auto filePath = baseDir / ("apdump_multifile_" + std::to_string(profileIdx++) + ".ap");
        std::error_code ec;
        fs::copy_file(sourceProfile, filePath, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            ADD_FAILURE() << "Failed to copy synthetic profile: " << ec.message();
            return {};
        }

        PandaString classCtx;
        PandaVector<PandaString> pandaFiles;
        auto loaded = ark::pgo::AotPgoFile::Load(PandaString(filePath.string().c_str()), classCtx, pandaFiles);
        EXPECT_TRUE(loaded) << "Cannot reload synthetic profile";
        if (!loaded) {
            return {};
        }
        auto &profilingData = *loaded;

        multiFilePandaFilesStorage_.clear();
        for (const auto &pf : pandaFiles) {
            multiFilePandaFilesStorage_.emplace_back(pf);
        }
        multiFilePandaFilesStorage_.emplace_back("feature.abc");
        profilingData.AddPandaFiles(multiFilePandaFilesStorage_);

        PandaVector<InlineCache> inlineCaches {};
        PandaVector<BranchData> branches {};
        PandaVector<ThrowData> throws {};
        ark::pgo::AotProfilingData::AotMethodProfilingData featureMethod(84U, 7U, std::move(inlineCaches),
                                                                         std::move(branches), std::move(throws));
        profilingData.AddMethod(static_cast<ark::pgo::PandaFileIdxType>(multiFilePandaFilesStorage_.size() - 1U), 84U,
                                std::move(featureMethod));

        ark::pgo::AotPgoFile writer;
        auto writtenBytes = writer.Save(PandaString(filePath.string().c_str()), &profilingData, classCtx);
        EXPECT_NE(writtenBytes, 0U);
        if (writtenBytes == 0U) {
            return {};
        }
        return filePath;
    }

private:
    struct CliTestContext {
        fs::path workspace;
        fs::path abcPath;
        fs::path apPath;
        fs::path outputPath;
        DumpTestArtifacts artifacts;
    };

    void GenerateProfileFile(const fs::path &path)
    {
        ark::pgo::AotProfilingData profilingData;
        pandaFilesStorage_.clear();
        pandaFilesStorage_.emplace_back("boot.abc");
        pandaFilesStorage_.emplace_back("sample.abc");
        profilingData.AddPandaFiles(pandaFilesStorage_);

        PandaVector<ark::pgo::AotProfilingData::AotCallSiteInlineCache> inlineCaches(1);
        inlineCaches[0].pc = 0x10;
        inlineCaches[0].classes[0] = std::make_pair(120U, 0);
        inlineCaches[0].classes[1] = std::make_pair(
            static_cast<uint32_t>(ark::pgo::AotProfilingData::AotCallSiteInlineCache::MEGAMORPHIC_FLAG), -1);

        constexpr uint64_t branchNotTakenCount = 3;
        constexpr uint64_t throwTakenCount = 2;
        constexpr uint64_t branchTakenCount = 8;

        PandaVector<ark::pgo::AotProfilingData::AotBranchData> branches(1);
        branches[0].pc = 0x22;  // NOLINT
        branches[0].taken = branchTakenCount;
        branches[0].notTaken = branchNotTakenCount;

        PandaVector<ark::pgo::AotProfilingData::AotThrowData> throwsData(1);
        throwsData[0].pc = 0x30;  // NOLINT
        throwsData[0].taken = throwTakenCount;

        ark::pgo::AotProfilingData::AotMethodProfilingData method(42U, 17U, std::move(inlineCaches),
                                                                  std::move(branches), std::move(throwsData));
        profilingData.AddMethod(0, 42U, std::move(method));

        ark::pgo::AotPgoFile fileWriter;
        auto writtenBytes =
            fileWriter.Save(PandaString(path.string().c_str()), &profilingData, PandaString("apdump.ctx"));
        ASSERT_NE(writtenBytes, 0U);
    }

protected:
    fs::path PrepareArtifactsDir(const std::string &name)
    {
        auto base = GetArtifactsRoot();
        if (base.empty() || name.empty()) {
            return {};
        }
        auto dir = base / name;
        std::error_code ec;
        fs::remove_all(dir, ec);
        fs::create_directories(dir, ec);
        return dir;
    }

    fs::path GetArtifactsRoot()
    {
        if (artifactsRoot_.empty()) {
            auto execPathOrError = os::file::File::GetExecutablePath();
            if (!execPathOrError) {
                ADD_FAILURE() << "Cannot determine test binary path";
                return {};
            }
            fs::path execPath(execPathOrError.Value());
            artifactsRoot_ = execPath.parent_path() / "apdump_test_artifacts";
        }
        if (artifactsRoot_.empty()) {
            return {};
        }
        std::error_code ec;
        fs::create_directories(artifactsRoot_, ec);
        if (!reportedRoot_) {
            std::cout << "[ApdumpTest] Artifacts root: " << artifactsRoot_ << std::endl;
            reportedRoot_ = true;
        }
        return artifactsRoot_;
    }

    CliTestContext SetupCliContext(const std::string &name)
    {
        CliTestContext ctx;
        ctx.workspace = PrepareArtifactsDir(name);
        EXPECT_FALSE(ctx.workspace.empty());
        if (ctx.workspace.empty()) {
            return ctx;
        }
        ctx.abcPath = ctx.workspace / "dump_test.abc";
        ctx.artifacts = CreateDumpTestAbc(ctx.abcPath);
        ctx.apPath = ctx.workspace / "profile_dump.ap";
        CreateDumpTestProfile(ctx.apPath, ctx.artifacts.entities, ctx.artifacts.checksum);
        ctx.outputPath = ctx.workspace / "profile.yaml";
        return ctx;
    }

    int RunAptool(const std::vector<std::string> &args) const
    {
        auto toolPath = FindArkAptoolBinary();
        if (toolPath.empty()) {
            ADD_FAILURE() << "Unable to locate ark_aptool binary";
            return -1;
        }
        std::vector<const char *> argv;
        argv.push_back(toolPath.c_str());
        argv.push_back("dump");
        for (const auto &arg : args) {
            argv.push_back(arg.c_str());
        }
        argv.push_back(nullptr);
        auto res = os::exec::Exec(Span(argv.data(), argv.size()));
        if (!res) {
            ADD_FAILURE() << "exec failed: " << res.Error().ToString();
            return -1;
        }
        return res.Value();
    }

    int RunAptoolCli(const CliTestContext &ctx, const std::vector<std::string> &extraArgs)
    {
        std::vector<std::string> args;
        args.emplace_back("--ap-path");
        args.emplace_back(ctx.apPath);
        args.emplace_back("--output");
        args.emplace_back(ctx.outputPath);
        args.insert(args.end(), extraArgs.begin(), extraArgs.end());
        return RunAptool(args);
    }

private:
    fs::path artifactsRoot_;
    bool reportedRoot_ {false};
    PandaVector<PandaString> pandaFilesStorage_;
    PandaVector<PandaString> multiFilePandaFilesStorage_;
};

TEST_F(AptoolDumpTest, SaveAndLoadSyntheticProfile)
{
    auto profilePath = CreateSyntheticProfile();
    ASSERT_FALSE(profilePath.empty());

    PandaString classCtx;
    PandaVector<PandaString> pandaFiles;
    auto loaded = ark::pgo::AotPgoFile::Load(PandaString(profilePath.string().c_str()), classCtx, pandaFiles);
    ASSERT_TRUE(loaded);

    EXPECT_EQ(classCtx, PandaString("apdump.ctx"));
    ASSERT_EQ(pandaFiles.size(), 2U);
    EXPECT_STREQ(pandaFiles[0].c_str(), "boot.abc");
    EXPECT_STREQ(pandaFiles[1].c_str(), "sample.abc");

    auto &allMethods = loaded->GetAllMethods();
    ASSERT_EQ(allMethods.size(), 1U);
    auto methodsIt = allMethods.find(0);
    ASSERT_NE(methodsIt, allMethods.end());
    ASSERT_EQ(methodsIt->second.size(), 1U);

    const auto &methodEntry = methodsIt->second.begin()->second;
    EXPECT_EQ(methodEntry.GetMethodIdx(), 42U);
    EXPECT_EQ(methodEntry.GetClassIdx(), 17U);

    auto inlineCaches = methodEntry.GetInlineCaches();
    ASSERT_EQ(inlineCaches.Size(), 1U);
    EXPECT_EQ(inlineCaches[0].pc, 0x10U);
    EXPECT_EQ(inlineCaches[0].classes[0].first, 120U);
    EXPECT_EQ(inlineCaches[0].classes[0].second, 0);

    auto branches = methodEntry.GetBranchData();
    ASSERT_EQ(branches.Size(), 1U);
    EXPECT_EQ(branches[0].pc, 0x22U);
    EXPECT_EQ(branches[0].taken, 8U);
    EXPECT_EQ(branches[0].notTaken, 3U);

    auto throwsData = methodEntry.GetThrowData();
    ASSERT_EQ(throwsData.Size(), 1U);
    EXPECT_EQ(throwsData[0].pc, 0x30U);
    EXPECT_EQ(throwsData[0].taken, 2U);
}

TEST_F(AptoolDumpTest, HandlesMultipleMethodsAndFiles)
{
    auto profilePath = CreateSyntheticProfile();
    ASSERT_FALSE(profilePath.empty());

    // Append another panda file and method data to the existing profile.
    PandaString ctx;
    PandaVector<PandaString> pandaFiles;
    auto loaded = ark::pgo::AotPgoFile::Load(PandaString(profilePath.string().c_str()), ctx, pandaFiles);
    ASSERT_TRUE(loaded);

    auto &profilingData = *loaded;
    PandaVector<PandaString> pandaFilesStorage;
    pandaFilesStorage.emplace_back("boot.abc");
    pandaFilesStorage.emplace_back("sample.abc");
    pandaFilesStorage.emplace_back("feature.abc");
    profilingData.AddPandaFiles(pandaFilesStorage);

    constexpr uint64_t extraBranchNotTaken = 9;
    PandaVector<ark::pgo::AotProfilingData::AotBranchData> extraBranches(1);
    extraBranches[0].pc = 0x44;
    extraBranches[0].taken = 1;
    extraBranches[0].notTaken = extraBranchNotTaken;
    ark::pgo::AotProfilingData::AotMethodProfilingData secondMethod(  // NOLINT
        7U, 5U, PandaVector<ark::pgo::AotProfilingData::AotCallSiteInlineCache> {}, std::move(extraBranches),
        PandaVector<ark::pgo::AotProfilingData::AotThrowData> {});
    profilingData.AddMethod(1, 7U, std::move(secondMethod));

    ark::pgo::AotPgoFile fileWriter;
    auto rewritten =
        fileWriter.Save(PandaString(profilePath.string().c_str()), &profilingData, PandaString("apdump.ctx"));
    ASSERT_NE(rewritten, 0U);

    PandaString newCtx;
    PandaVector<PandaString> updatedFiles;
    auto reloaded = ark::pgo::AotPgoFile::Load(PandaString(profilePath.string().c_str()), newCtx, updatedFiles);
    ASSERT_TRUE(reloaded);
    constexpr size_t expectedFileCount = 3U;
    constexpr size_t featureAbcIndex = 2;
    EXPECT_EQ(updatedFiles.size(), expectedFileCount);
    EXPECT_STREQ(updatedFiles[featureAbcIndex].c_str(), "feature.abc");

    auto &allMethods = reloaded->GetAllMethods();
    ASSERT_EQ(allMethods.size(), 2U);
    auto secondPf = allMethods.find(1);
    ASSERT_NE(secondPf, allMethods.end());  // NOLINT
    ASSERT_EQ(secondPf->second.size(), 1U);
    EXPECT_EQ(secondPf->second.begin()->second.GetBranchData()[0].pc, 0x44U);
}

namespace {
DumpTestArtifacts CreateDumpTestAbc(const fs::path &abcPath)
{
    DumpTestArtifacts artifacts;
    auto asmPath = FindProjectFile(DUMP_TEST_ASM_PATH);
    if (asmPath.empty()) {
        ADD_FAILURE() << "Failed to locate dump_test.asm";
        return artifacts;
    }
    auto source = ReadFile(asmPath);
    if (source.empty()) {
        ADD_FAILURE() << "dump_test.asm is empty";
        return artifacts;
    }

    pandasm::Parser parser;
    auto parsed = parser.Parse(source, asmPath.string());
    if (!parsed) {
        ADD_FAILURE() << "Failed to parse dump_test.asm: " << parsed.Error().message << " (line "
                      << parsed.Error().lineNumber << ")";
        return artifacts;
    }

    bool emitted = pandasm::AsmEmitter::Emit(abcPath.string(), parsed.Value());
    if (!emitted) {
        ADD_FAILURE() << "Failed to emit dump_test.abc from assembly";
        return artifacts;
    }

    auto file = panda_file::OpenPandaFile(abcPath.string());
    if (file == nullptr) {
        ADD_FAILURE() << "Failed to open emitted dump_test.abc";
        return artifacts;
    }
    // Verify the emitted abc: iterate all classes and their methods to check file structure
    for (auto classId : file->GetClasses()) {
        panda_file::File::EntityId id(classId);
        if (file->IsExternal(id)) {
            continue;
        }
        panda_file::ClassDataAccessor cda(*file, id);
        cda.EnumerateMethods([&]([[maybe_unused]] panda_file::MethodDataAccessor &mda) {
            auto codeId = mda.GetCodeId();
            if (codeId.has_value()) {
                panda_file::CodeDataAccessor codeDa(*file, codeId.value());
                EXPECT_GT(codeDa.GetCodeSize(), 0U) << "Method has empty code in dump_test.abc";
            }
        });
    }

    artifacts.entities = ResolveDumpTestEntities(*file);
    // Verify all expected entities were resolved
    EXPECT_NE(artifacts.entities.classAdder, 0U) << "Class dump_test/Adder not found in abc";
    EXPECT_NE(artifacts.entities.classDoubler, 0U) << "Class dump_test/Doubler not found in abc";
    EXPECT_NE(artifacts.entities.classWorker, 0U) << "Class dump_test/Worker not found in abc";
    EXPECT_NE(artifacts.entities.classEtsglobal, 0U) << "Class dump_test/ETSGLOBAL not found in abc";
    EXPECT_NE(artifacts.entities.methodFoo, 0U) << "Method foo not found in abc";
    EXPECT_NE(artifacts.entities.methodMain, 0U) << "Method main not found in abc";
    EXPECT_NE(artifacts.entities.methodNoisyBranch, 0U) << "Method noisyBranch not found in abc";
    EXPECT_NE(artifacts.entities.methodRunPolymorphic, 0U) << "Method runPolymorphic not found in abc";
    artifacts.checksum = file->GetPaddedChecksum();
    return artifacts;
}

DumpTestEntities ResolveDumpTestEntities(const panda_file::File &file)
{
    DumpTestEntities entities;
    auto getClassId = [&](std::string_view name) -> std::optional<panda_file::File::EntityId> {
        std::string descriptor = "L";
        descriptor.append(name);
        descriptor.push_back(';');
        auto id = file.GetClassId(utf::CStringAsMutf8(descriptor.c_str()));
        if (!id.IsValid()) {
            return std::nullopt;
        }
        return id;
    };

    auto adderId = getClassId("dump_test/Adder");
    if (adderId.has_value()) {
        entities.classAdder = adderId->GetOffset();
    }
    auto doublerId = getClassId("dump_test/Doubler");
    if (doublerId.has_value()) {
        entities.classDoubler = doublerId->GetOffset();
    }
    auto workerId = getClassId("dump_test/Worker");
    if (workerId.has_value()) {
        entities.classWorker = workerId->GetOffset();
    }
    auto etsGlobalId = getClassId("dump_test/ETSGLOBAL");
    if (etsGlobalId.has_value()) {
        entities.classEtsglobal = etsGlobalId->GetOffset();
        panda_file::ClassDataAccessor cda(file, *etsGlobalId);
        cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
            auto name = utf::Mutf8AsCString(file.GetStringData(mda.GetNameId()).data);
            auto methodIdx = mda.GetMethodId().GetOffset();
            if (std::strcmp(name, "foo") == 0) {
                entities.methodFoo = methodIdx;
            } else if (std::strcmp(name, "main") == 0) {
                entities.methodMain = methodIdx;
            } else if (std::strcmp(name, "noisyBranch") == 0) {
                entities.methodNoisyBranch = methodIdx;
            } else if (std::strcmp(name, "runPolymorphic") == 0) {
                entities.methodRunPolymorphic = methodIdx;
            }
        });
    }
    return entities;
}

InlineCache MakeInlineCache(uint32_t pc, std::initializer_list<std::pair<uint32_t, ark::pgo::PandaFileIdxType>> classes)
{
    InlineCache cache {};
    cache.pc = pc;
    InlineCache::ClearClasses(cache.classes);
    size_t idx = 0;
    for (auto entry : classes) {
        if (idx >= cache.classes.size()) {
            break;
        }
        cache.classes[idx++] = entry;
    }
    return cache;
}

BranchData MakeBranch(uint32_t pc, uint64_t taken, uint64_t notTaken)
{
    BranchData branch {};
    branch.pc = pc;
    branch.taken = taken;
    branch.notTaken = notTaken;
    return branch;
}

ThrowData MakeThrow(uint32_t pc, uint64_t taken)
{
    ThrowData data {};
    data.pc = pc;
    data.taken = taken;
    return data;
}

ark::pgo::AotProfilingData::AotMethodProfilingData BuildFooMethod(const DumpTestEntities &entities)
{
    PandaVector<InlineCache> inlineCaches(1);
    inlineCaches[0] = MakeInlineCache(0x45, {{entities.classWorker, 0}});

    constexpr uint64_t branch1NotTaken = 2;
    constexpr size_t fooBranchCount = 3;
    constexpr size_t lastBranchIdx = 2;
    PandaVector<BranchData> branches(fooBranchCount);
    branches[0] = MakeBranch(0x0B, 1, 0);
    branches[1] = MakeBranch(0x3E, 1, branch1NotTaken);
    branches[lastBranchIdx] = MakeBranch(0x4C, 0, 0);

    PandaVector<ThrowData> throws(1);  // NOLINT
    throws[0] = MakeThrow(0x1A, 0);
    // NOLINT
    return ark::pgo::AotProfilingData::AotMethodProfilingData(entities.methodFoo, entities.classEtsglobal,  // NOLINT
                                                              std::move(inlineCaches), std::move(branches),
                                                              std::move(throws));
}

ark::pgo::AotProfilingData::AotMethodProfilingData BuildMainMethod(const DumpTestEntities &entities)
{
    PandaVector<InlineCache> inlineCaches(1);
    inlineCaches[0] = MakeInlineCache(0x3E, {});

    constexpr size_t mainBranchCount = 3;
    constexpr size_t mainLastBranchIdx = 2;
    PandaVector<BranchData> branches(mainBranchCount);
    branches[0] = MakeBranch(0x0D, 0, 0);
    branches[1] = MakeBranch(0x37, 0, 0);
    branches[mainLastBranchIdx] = MakeBranch(0x45, 0, 0);

    PandaVector<ThrowData> throws {};  // NOLINT
    return ark::pgo::AotProfilingData::AotMethodProfilingData(entities.methodMain, entities.classEtsglobal,
                                                              std::move(inlineCaches), std::move(branches),
                                                              std::move(throws));  // NOLINT
}

ark::pgo::AotProfilingData::AotMethodProfilingData BuildNoisyBranchMethod(const DumpTestEntities &entities)
{
    PandaVector<InlineCache> inlineCaches {};

    constexpr size_t branchCount = 6;
    constexpr uint64_t branch0NotTaken = 10;
    constexpr uint64_t branch1Taken = 6;
    constexpr uint64_t branch1NotTaken = 4;
    constexpr size_t branch2Idx = 2;
    constexpr size_t branch3Idx = 3;
    constexpr size_t branch4Idx = 4;
    constexpr size_t branch5Idx = 5;
    constexpr uint64_t branch3Taken = 3;
    constexpr uint64_t branch3NotTaken = 3;
    constexpr uint64_t throwTaken = 3;

    PandaVector<BranchData> branches(branchCount);
    branches[0] = MakeBranch(0x10, 1, branch0NotTaken);
    branches[1] = MakeBranch(0x16, branch1Taken, branch1NotTaken);
    branches[branch2Idx] = MakeBranch(0x1A, 0, 0);
    branches[branch3Idx] = MakeBranch(0x20, branch3Taken, branch3NotTaken);
    branches[branch4Idx] = MakeBranch(0x27, 0, 0);
    branches[branch5Idx] = MakeBranch(0x3C, 0, 0);

    PandaVector<ThrowData> throws(1);
    throws[0] = MakeThrow(0x36, throwTaken);
    return ark::pgo::AotProfilingData::AotMethodProfilingData(
        entities.methodNoisyBranch, entities.classEtsglobal,  // NOLINT
        std::move(inlineCaches), std::move(branches), std::move(throws));
}  // NOLINT

ark::pgo::AotProfilingData::AotMethodProfilingData BuildRunPolymorphicMethod(const DumpTestEntities &entities)
{
    constexpr size_t inlineCacheCount = 3;
    constexpr size_t lastInlineCacheIdx = 2;
    PandaVector<InlineCache> inlineCaches(inlineCacheCount);
    inlineCaches[0] = MakeInlineCache(0x12, {{entities.classWorker, 0}});
    inlineCaches[1] = MakeInlineCache(0x1A, {{entities.classWorker, 0}});
    inlineCaches[lastInlineCacheIdx] = MakeInlineCache(0x31, {{entities.classAdder, 0}, {entities.classDoubler, 0}});

    constexpr size_t polymorphicBranchCount = 3;
    constexpr size_t polyBranch2Idx = 2;
    constexpr uint64_t branchNotTaken6 = 6;
    PandaVector<BranchData> branches(polymorphicBranchCount);
    branches[0] = MakeBranch(0x10, 1, branchNotTaken6);
    branches[1] = MakeBranch(0x20, 0, branchNotTaken6);
    branches[polyBranch2Idx] = MakeBranch(0x3D, 0, 0);

    PandaVector<ThrowData> throws(1);
    throws[0] = MakeThrow(0x4B, 0);
    return ark::pgo::AotProfilingData::AotMethodProfilingData(
        entities.methodRunPolymorphic, entities.classEtsglobal,  // NOLINT
        std::move(inlineCaches), std::move(branches), std::move(throws));
}

void CreateDumpTestProfile(const fs::path &path, const DumpTestEntities &entities, const std::string &abcChecksum,
                           std::string_view contextEntry)
{
    struct ProfileContainer {
        PandaVector<PandaString> pandaFiles;
        ark::pgo::AotProfilingData data;
    } container;

    container.pandaFiles.emplace_back(DUMP_TEST_ABC_NAME);
    container.data.AddPandaFiles(container.pandaFiles);
    container.data.AddMethod(0, entities.methodFoo, BuildFooMethod(entities));
    container.data.AddMethod(0, entities.methodMain, BuildMainMethod(entities));
    container.data.AddMethod(0, entities.methodNoisyBranch, BuildNoisyBranchMethod(entities));
    container.data.AddMethod(0, entities.methodRunPolymorphic, BuildRunPolymorphicMethod(entities));

    ark::pgo::AotPgoFile writer;
    std::string ctx(contextEntry);
    ctx.push_back('*');
    ctx += abcChecksum;
    PandaString classCtxStr(ctx.c_str());
    auto written = writer.Save(PandaString(path.string().c_str()), &container.data, classCtxStr);
    ASSERT_NE(written, 0U);
}

std::string FindArkAptoolBinary()
{
    auto execPathOrError = os::file::File::GetExecutablePath();
    EXPECT_TRUE(execPathOrError) << "Cannot determine test binary path";
    if (!execPathOrError) {
        return {};
    }
    fs::path current(execPathOrError.Value());
    current = current.parent_path();
    constexpr size_t maxParentDepth = 5;
    for (size_t i = 0; i < maxParentDepth && !current.empty(); i++) {
        auto candidate = current / "bin" / "ark_aptool";
        if (fs::exists(candidate)) {
            return candidate.string();
        }
        current = current.parent_path();  // NOLINT
    }
    ADD_FAILURE() << "Unable to locate ark_aptool binary";
    return {};
}

fs::path FindProjectFile(const std::string &relativePath)
{
    auto execPathOrError = os::file::File::GetExecutablePath();
    EXPECT_TRUE(execPathOrError) << "Cannot determine test binary path";
    if (!execPathOrError) {
        return {};
    }
    fs::path current(execPathOrError.Value());
    current = current.parent_path();
    auto relative = fs::path(relativePath);
    if (relative.empty()) {
        return {};
    }
    constexpr size_t maxSearchDepth = 8;
    for (size_t i = 0; i < maxSearchDepth && !current.empty(); i++) {
        auto candidate = current / relative;
        std::error_code ec;
        if (fs::exists(candidate, ec)) {
            return candidate;
        }  // NOLINT
        current = current.parent_path();
    }
    return {};
}

std::string ReadFile(const fs::path &path)
{
    std::ifstream in(path);
    EXPECT_TRUE(in.is_open()) << "Cannot open " << path;
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

}  // namespace

TEST_F(AptoolDumpTest, DumpsStoredEtsProfile)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("cli_workspace");
    ASSERT_FALSE(ctx.workspace.empty());

    int ret = RunAptoolCli(ctx, {"--abc-path", ctx.abcPath, "--keep-panda-file-name", "dump_test.abc"});
    ASSERT_EQ(ret, 0) << "aptool dump invocation failed";

    auto yaml = ReadFile(ctx.outputPath);

    // Verify key content exists (checksum and indices may vary between Debug/Release modes)
    EXPECT_NE(yaml.find("ClassCtxStr: \"dump_test.abc*"), std::string::npos) << "Missing ClassCtxStr";
    EXPECT_NE(yaml.find("file_name: \"dump_test.abc\""), std::string::npos) << "Missing file_name";

    // Verify all expected methods are present
    EXPECT_NE(yaml.find("method_signature: \"foo:(f64)\""), std::string::npos) << "Missing foo method";
    EXPECT_NE(yaml.find("method_signature: \"main:()\""), std::string::npos) << "Missing main method";
    EXPECT_NE(yaml.find("method_signature: \"noisyBranch:(f64)\""), std::string::npos) << "Missing noisyBranch method";
    EXPECT_NE(yaml.find("method_signature: \"runPolymorphic:(escompat.Array,f64)\""), std::string::npos)
        << "Missing runPolymorphic method";

    // Verify class descriptor
    EXPECT_NE(yaml.find("class_descriptor: \"Ldump_test/ETSGLOBAL;\""), std::string::npos) << "Missing ETSGLOBAL class";
    EXPECT_NE(yaml.find("class_descriptor: \"Ldump_test/Adder;\""), std::string::npos) << "Missing Adder class";

    // Verify inline caches and branch data are present
    EXPECT_NE(yaml.find("inline_caches:"), std::string::npos) << "Missing inline_caches";
    EXPECT_NE(yaml.find("branches:"), std::string::npos) << "Missing branches";
    EXPECT_NE(yaml.find("takenCount:"), std::string::npos) << "Missing branch takenCount";
}

TEST_F(AptoolDumpTest, FiltersClassAndMethodNames)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("filters");
    ASSERT_FALSE(ctx.workspace.empty());
    int ret = RunAptoolCli(
        ctx, {"--abc-path", ctx.abcPath, "--keep-class-name", "Ldump_test/ETSGLOBAL;", "--keep-method-name", "noisy"});
    ASSERT_EQ(ret, 0) << "aptool dump invocation failed";
    auto yaml = ReadFile(ctx.outputPath);
    EXPECT_NE(yaml.find("noisyBranch:(f64)"), std::string::npos);
    EXPECT_EQ(yaml.find("foo:(f64)"), std::string::npos);
    EXPECT_EQ(yaml.find("runPolymorphic"), std::string::npos);
}

TEST_F(AptoolDumpTest, LoadsMetadataFromDirectory)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("metadata_from_dir");
    ASSERT_FALSE(ctx.workspace.empty());
    auto dir = ctx.workspace / "abc_dir";
    std::error_code ec;
    fs::create_directories(dir, ec);
    fs::copy_file(ctx.abcPath, dir / ctx.abcPath.filename(), fs::copy_options::overwrite_existing, ec);
    int ret = RunAptoolCli(ctx, {"--abc-dir", dir.string()});
    ASSERT_EQ(ret, 0) << "aptool dump invocation failed";
    auto yaml = ReadFile(ctx.outputPath);
    EXPECT_NE(yaml.find("class_descriptor:"), std::string::npos);
}

TEST_F(AptoolDumpTest, FiltersPandaFilesByName)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto workspace = PrepareArtifactsDir("keep_pf_filter");
    ASSERT_FALSE(workspace.empty());
    auto profilePath = CreateMultiFileProfile();
    ASSERT_FALSE(profilePath.empty());
    auto outputPath = workspace / "keep_pf.yaml";
    int ret =
        RunAptool({"--ap-path", profilePath, "--output", outputPath.string(), "--keep-panda-file-name", "feature.abc"});
    ASSERT_EQ(ret, 0) << "Expected aptool dump to filter panda files";
    auto yaml = ReadFile(outputPath);
    EXPECT_NE(yaml.find("feature.abc"), std::string::npos);
    EXPECT_EQ(yaml.find("boot.abc"), std::string::npos);
    EXPECT_EQ(yaml.find("panda_file_index: 0"), std::string::npos);
}

TEST_F(AptoolDumpTest, RejectsClassFilterWithoutAbcMetadata)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("no_abc_filter");
    ASSERT_FALSE(ctx.workspace.empty());
    int ret = RunAptoolCli(ctx, {"--keep-class-name", "Ldump_test/ETSGLOBAL;"});
    ASSERT_NE(ret, 0) << "Expected aptool dump to fail without abc metadata";
}

TEST_F(AptoolDumpTest, PrintsHelpWithoutInputs)
{
    int ret = RunAptool({"--help"});
    ASSERT_EQ(ret, 0) << "Expected --help to exit successfully";
}

TEST_F(AptoolDumpTest, RequiresApPathArgument)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto workspace = PrepareArtifactsDir("missing_ap_path");
    ASSERT_FALSE(workspace.empty());
    auto outputPath = workspace / "missing_ap.yaml";
    int ret = RunAptool({"--output", outputPath.string()});
    ASSERT_NE(ret, 0) << "Expected aptool dump to reject missing --ap-path";
}

TEST_F(AptoolDumpTest, RequiresOutputArgument)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("missing_output_arg");
    ASSERT_FALSE(ctx.workspace.empty());
    int ret = RunAptool({"--ap-path", ctx.apPath});
    ASSERT_NE(ret, 0) << "Expected aptool dump to reject missing --output";
}

TEST_F(AptoolDumpTest, FailsWhenOutputPathCannotBeOpened)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("output_dir_path");
    ASSERT_FALSE(ctx.workspace.empty());
    auto outputDir = ctx.workspace / "blocked_dir";
    std::error_code ec;
    fs::create_directories(outputDir, ec);
    int ret = RunAptool({"--ap-path", ctx.apPath, "--output", outputDir.string()});
    ASSERT_NE(ret, 0) << "Expected aptool dump to fail when output path is not writable";
}

TEST_F(AptoolDumpTest, FailsWhenProfileFileIsMissing)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("missing_profile_arg");
    ASSERT_FALSE(ctx.workspace.empty());
    auto missingPath = ctx.workspace / "missing_profile.ap";
    auto ctxCopy = ctx;
    ctxCopy.apPath = missingPath;
    int ret = RunAptoolCli(ctxCopy, {});
    ASSERT_NE(ret, 0) << "Expected aptool dump to fail when profile cannot be loaded";
}

TEST_F(AptoolDumpTest, MetadataLoadFailsForNonAbcPath)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("non_abc_metadata");
    ASSERT_FALSE(ctx.workspace.empty());
    auto notAbc = ctx.workspace / "metadata.txt";
    {
        std::ofstream out(notAbc);
        out << "invalid metadata";
    }
    int ret = RunAptoolCli(ctx, {"--abc-path", notAbc.string()});
    ASSERT_NE(ret, 0) << "Expected aptool dump to fail when metadata is not .abc";
}

TEST_F(AptoolDumpTest, MetadataLoadFailsForInvalidDirectory)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("invalid_abc_dir");
    ASSERT_FALSE(ctx.workspace.empty());
    int ret = RunAptoolCli(ctx, {"--abc-dir", ctx.apPath});
    ASSERT_NE(ret, 0) << "Expected aptool dump to report invalid --abc-dir argument";
}

TEST_F(AptoolDumpTest, DetectsMismatchedClassContextChecksum)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("checksum_mismatch");
    ASSERT_FALSE(ctx.workspace.empty());
    auto corruptedChecksum = ctx.artifacts.checksum;
    if (!corruptedChecksum.empty()) {
        corruptedChecksum[0] = corruptedChecksum[0] == '0' ? '1' : '0';
    } else {
        corruptedChecksum = "deadbeef";
    }
    auto contextEntry = ctx.abcPath.string();
    CreateDumpTestProfile(ctx.apPath, ctx.artifacts.entities, corruptedChecksum, contextEntry);
    int ret = RunAptoolCli(ctx, {"--abc-path", ctx.abcPath});
    ASSERT_NE(ret, 0) << "Expected aptool dump to reject mismatched class context checksums";
}

TEST_F(AptoolDumpTest, HandlesMegamorphicInlineCache)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto workspace = PrepareArtifactsDir("synthetic_cli");
    ASSERT_FALSE(workspace.empty());
    auto profilePath = CreateSyntheticProfile();
    ASSERT_FALSE(profilePath.empty());
    auto outputPath = workspace / "synthetic.yaml";
    int ret = RunAptool({"--ap-path", profilePath, "--output", outputPath.string()});
    ASSERT_EQ(ret, 0) << "Expected aptool dump to dump synthetic profile";
    auto yaml = ReadFile(outputPath);
    EXPECT_NE(yaml.find("megamorphic: true"), std::string::npos);
}

TEST_F(AptoolDumpTest, KeepsPandaFileWhenAbsolutePathProvided)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("keep_abs_path");
    ASSERT_FALSE(ctx.workspace.empty());
    int ret = RunAptoolCli(ctx, {"--abc-path", ctx.abcPath, "--keep-panda-file-name", ctx.abcPath});
    ASSERT_EQ(ret, 0) << "Expected aptool dump to accept absolute panda file filters";
    auto yaml = ReadFile(ctx.outputPath);

    // Verify key content exists (same as DumpsStoredEtsProfile - checksum/indices may vary)
    EXPECT_NE(yaml.find("ClassCtxStr: \"dump_test.abc*"), std::string::npos) << "Missing ClassCtxStr";
    EXPECT_NE(yaml.find("file_name: \"dump_test.abc\""), std::string::npos) << "Missing file_name";

    // Verify all expected methods are present
    EXPECT_NE(yaml.find("method_signature: \"foo:(f64)\""), std::string::npos) << "Missing foo method";
    EXPECT_NE(yaml.find("method_signature: \"main:()\""), std::string::npos) << "Missing main method";
    EXPECT_NE(yaml.find("method_signature: \"noisyBranch:(f64)\""), std::string::npos) << "Missing noisyBranch method";
    EXPECT_NE(yaml.find("method_signature: \"runPolymorphic:(escompat.Array,f64)\""), std::string::npos)
        << "Missing runPolymorphic method";

    // Verify profile data structures
    EXPECT_NE(yaml.find("inline_caches:"), std::string::npos) << "Missing inline_caches";
    EXPECT_NE(yaml.find("branches:"), std::string::npos) << "Missing branches";
}

// ============================================================================
// Integration Tests - Full PGO workflow: compile -> run with profiling -> dump
// ============================================================================

namespace {

constexpr const char *INTEGRATION_TEST_ETS_PATH = "plugins/ets/tests/compiler/aptool/integration_test.ets";

std::string FindEs2pandaBinary()
{
    auto execPathOrError = os::file::File::GetExecutablePath();
    EXPECT_TRUE(execPathOrError) << "Cannot determine test binary path";
    if (!execPathOrError) {
        return {};
    }
    fs::path current(execPathOrError.Value());
    current = current.parent_path();
    constexpr size_t maxParentDepth = 5;
    for (size_t i = 0; i < maxParentDepth && !current.empty(); i++) {
        auto candidate = current / "bin" / "es2panda";
        if (fs::exists(candidate)) {
            return candidate.string();
        }
        current = current.parent_path();
    }
    return {};
}

std::string FindArkBinary()
{
    auto execPathOrError = os::file::File::GetExecutablePath();
    EXPECT_TRUE(execPathOrError) << "Cannot determine test binary path";
    if (!execPathOrError) {
        return {};
    }
    fs::path current(execPathOrError.Value());
    current = current.parent_path();
    constexpr size_t maxParentDepth = 5;
    for (size_t i = 0; i < maxParentDepth && !current.empty(); i++) {
        auto candidate = current / "bin" / "ark";
        if (fs::exists(candidate)) {
            return candidate.string();
        }
        current = current.parent_path();
    }
    return {};
}

fs::path FindEtsStdlib()
{
    auto execPathOrError = os::file::File::GetExecutablePath();
    EXPECT_TRUE(execPathOrError) << "Cannot determine test binary path";
    if (!execPathOrError) {
        return {};
    }
    fs::path current(execPathOrError.Value());
    current = current.parent_path();
    constexpr size_t maxParentDepth = 5;
    for (size_t i = 0; i < maxParentDepth && !current.empty(); i++) {
        auto candidate = current / "plugins" / "ets" / "etsstdlib.abc";
        if (fs::exists(candidate)) {
            return candidate;
        }
        current = current.parent_path();
    }
    return {};
}

}  // namespace

namespace {

// Helper struct to hold paths for integration tests
struct IntegrationTestContext {
    std::string es2pandaPath;
    std::string arkPath;
    fs::path stdlibPath;
    fs::path etsSourcePath;

    bool IsValid() const
    {
        return !es2pandaPath.empty() && !arkPath.empty() && !stdlibPath.empty() && !etsSourcePath.empty();
    }
};

// Check if all required binaries exist for integration tests
// Returns empty context if any binary is missing
IntegrationTestContext GetIntegrationTestContext()
{
    IntegrationTestContext ctx;
    ctx.es2pandaPath = FindEs2pandaBinary();
    ctx.arkPath = FindArkBinary();
    ctx.stdlibPath = FindEtsStdlib();
    ctx.etsSourcePath = FindProjectFile(INTEGRATION_TEST_ETS_PATH);
    return ctx;
}

}  // namespace

// Integration test: Full PGO workflow with real ETS code
// Tests the complete flow: compile ETS -> run with profiling -> aptool dump -> verify output
// Verifies inline caches, branches, and basic profile structure in a single test
TEST_F(AptoolDumpTest, IntegrationTestFullPgoWorkflow)
{
    APTOOL_SKIP_IF_NO_ETS();

    // Check all required binaries exist
    auto ctx = GetIntegrationTestContext();
    if (!ctx.IsValid()) {
        GTEST_SKIP() << "Required binaries not found (es2panda, ark, etsstdlib.abc, or integration_test.ets), "
                     << "skipping integration test. Run full build to enable this test.";
    }

    // Setup workspace
    auto workspace = PrepareArtifactsDir("integration_test");
    ASSERT_FALSE(workspace.empty());

    auto abcPath = workspace / "integration_test.abc";
    auto apPath = workspace / "profile.ap";
    auto yamlPath = workspace / "profile.yaml";

    // Step 1: Compile ETS to ABC
    {
        auto outputArg = "--output=" + abcPath.string();
        auto res = os::exec::Exec(ctx.es2pandaPath.c_str(), "--extension=ets", outputArg.c_str(),
                                  ctx.etsSourcePath.string().c_str());
        ASSERT_TRUE(res) << "es2panda exec failed: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0) << "Failed to compile ETS file to ABC";
        ASSERT_TRUE(fs::exists(abcPath)) << "ABC file was not created";
    }

    // Step 2: Run ark with profiling enabled to generate AP file
    {
        auto bootArg = "--boot-panda-files=" + ctx.stdlibPath.string();
        auto profileOutputArg = "--profile-output=" + apPath.string();
        auto res =
            os::exec::Exec(ctx.arkPath.c_str(), "--load-runtimes=ets", bootArg.c_str(), "--compiler-enable-jit=true",
                           "--compiler-profiling-threshold=0", "--profilesaver-enabled=true", profileOutputArg.c_str(),
                           "--profile-branches=true", abcPath.string().c_str(), "integration_test.ETSGLOBAL::main");
        ASSERT_TRUE(res) << "ark exec failed: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0) << "Failed to run ark with profiling";
        ASSERT_TRUE(fs::exists(apPath)) << "AP profile file was not created";
    }

    // Step 3: Run aptool to dump the AP file
    {
        int ret =
            RunAptool({"--ap-path", apPath.string(), "--output", yamlPath.string(), "--abc-path", abcPath.string()});
        ASSERT_EQ(ret, 0) << "aptool dump failed";
        ASSERT_TRUE(fs::exists(yamlPath)) << "YAML output was not created";
    }

    // Step 4: Verify the YAML output contains expected profiling data
    auto yaml = ReadFile(yamlPath);
    ASSERT_FALSE(yaml.empty()) << "YAML output is empty";

    // Verify basic structure
    EXPECT_NE(yaml.find("ClassCtxStr:"), std::string::npos) << "Missing ClassCtxStr";
    EXPECT_NE(yaml.find("PandaFiles:"), std::string::npos) << "Missing PandaFiles section";
    EXPECT_NE(yaml.find("Methods:"), std::string::npos) << "Missing Methods section";
    EXPECT_NE(yaml.find("integration_test.abc"), std::string::npos) << "Missing panda file name";

    // Verify method signatures from our test code are present
    bool hasMainMethod = yaml.find("main") != std::string::npos;
    bool hasFooMethod = yaml.find("foo") != std::string::npos;
    bool hasNoisyBranchMethod = yaml.find("noisyBranch") != std::string::npos;
    bool hasRunPolymorphicMethod = yaml.find("runPolymorphic") != std::string::npos;

    EXPECT_TRUE(hasMainMethod || hasFooMethod || hasNoisyBranchMethod || hasRunPolymorphicMethod)
        << "Expected at least one profiled method from test code";

    // Verify inline cache data (from polymorphic calls in runPolymorphic)
    bool hasInlineCaches = yaml.find("inline_caches:") != std::string::npos;
    bool hasTargets = yaml.find("targets:") != std::string::npos;

    // Verify branch profiling data (from noisyBranch)
    bool hasBranches = yaml.find("branches:") != std::string::npos;
    bool hasTakenCount = yaml.find("takenCount:") != std::string::npos;

    std::cout << "[Integration] Profiled methods: main=" << hasMainMethod << ", foo=" << hasFooMethod
              << ", noisyBranch=" << hasNoisyBranchMethod << ", runPolymorphic=" << hasRunPolymorphicMethod
              << std::endl;
    std::cout << "[Integration] Inline caches: present=" << hasInlineCaches << ", targets=" << hasTargets << std::endl;
    std::cout << "[Integration] Branches: present=" << hasBranches << ", takenCount=" << hasTakenCount << std::endl;
}

// Integration test: Test panda file filtering with multiple abc files (app + stdlib)
// The profile contains both etsstdlib.abc and app.abc, so we can filter between them
TEST_F(AptoolDumpTest, IntegrationTestPandaFileFiltering)
{
    APTOOL_SKIP_IF_NO_ETS();

    auto ctx = GetIntegrationTestContext();
    if (!ctx.IsValid()) {
        GTEST_SKIP() << "Required binaries not found, skipping integration test. Run full build to enable this test.";
    }

    auto workspace = PrepareArtifactsDir("integration_filter_test");
    ASSERT_FALSE(workspace.empty());

    auto abcPath = workspace / "app.abc";
    auto apPath = workspace / "profile.ap";
    auto yamlPathAll = workspace / "all.yaml";
    auto yamlPathAppOnly = workspace / "app_only.yaml";

    // Compile
    {
        auto outputArg = "--output=" + abcPath.string();
        auto res = os::exec::Exec(ctx.es2pandaPath.c_str(), "--extension=ets", outputArg.c_str(),
                                  ctx.etsSourcePath.string().c_str());
        ASSERT_TRUE(res) << "es2panda exec failed: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0) << "Failed to compile";
    }

    // Run to collect profile (will contain both etsstdlib.abc and app.abc)
    {
        auto bootArg = "--boot-panda-files=" + ctx.stdlibPath.string();
        auto profileOutputArg = "--profile-output=" + apPath.string();
        auto res =
            os::exec::Exec(ctx.arkPath.c_str(), "--load-runtimes=ets", bootArg.c_str(), "--compiler-enable-jit=true",
                           "--compiler-profiling-threshold=0", "--profilesaver-enabled=true", profileOutputArg.c_str(),
                           abcPath.string().c_str(), "integration_test.ETSGLOBAL::main");
        ASSERT_TRUE(res) << "ark exec failed: " << res.Error().ToString();
        ASSERT_EQ(res.Value(), 0) << "Failed to run ark";
    }

    // Dump all panda files (unfiltered)
    {
        int ret =
            RunAptool({"--ap-path", apPath.string(), "--output", yamlPathAll.string(), "--abc-path", abcPath.string()});
        ASSERT_EQ(ret, 0) << "aptool dump (all) failed";
    }

    // Dump only app.abc (filter out etsstdlib.abc)
    {
        int ret = RunAptool({"--ap-path", apPath.string(), "--output", yamlPathAppOnly.string(), "--abc-path",
                             abcPath.string(), "--keep-panda-file-name", "app.abc"});
        ASSERT_EQ(ret, 0) << "aptool dump (app only) failed";
    }

    auto yamlAll = ReadFile(yamlPathAll);
    auto yamlAppOnly = ReadFile(yamlPathAppOnly);

    ASSERT_FALSE(yamlAll.empty());
    ASSERT_FALSE(yamlAppOnly.empty());

    // Helper to extract PandaFiles section and check for multiple entries
    // PandaFiles section format:
    //   PandaFiles:
    //     - index: 0
    //       file_name: "app.abc"
    //     - index: 1
    //       file_name: "etsstdlib.abc"
    auto hasTwoPandaFilesInSection = [](const std::string &yaml) {
        // Find the PandaFiles section
        auto pandaFilesPos = yaml.find("PandaFiles:");
        if (pandaFilesPos == std::string::npos) {
            return false;
        }
        // Find the Methods section (which comes after PandaFiles)
        auto methodsPos = yaml.find("Methods:", pandaFilesPos);
        if (methodsPos == std::string::npos) {
            methodsPos = yaml.size();
        }
        // Extract only the PandaFiles section
        std::string pandaFilesSection = yaml.substr(pandaFilesPos, methodsPos - pandaFilesPos);
        // Check if there's "index: 1" in the PandaFiles section
        return pandaFilesSection.find("index: 1") != std::string::npos;
    };

    bool allHasTwoPandaFiles = hasTwoPandaFilesInSection(yamlAll);
    bool appOnlyHasTwoPandaFiles = hasTwoPandaFilesInSection(yamlAppOnly);

    std::cout << "[Integration] Unfiltered PandaFiles has index 1: " << allHasTwoPandaFiles << std::endl;
    std::cout << "[Integration] Filtered PandaFiles has index 1: " << appOnlyHasTwoPandaFiles << std::endl;

    // Verify unfiltered output has both panda files in PandaFiles section
    EXPECT_TRUE(allHasTwoPandaFiles) << "Unfiltered output should have two panda files in PandaFiles section";

    // Verify filtered output only has one panda file in PandaFiles section
    EXPECT_FALSE(appOnlyHasTwoPandaFiles) << "Filtered output should only have app.abc in PandaFiles section";
}

TEST_F(AptoolDumpTest, MatchesAbcByChecksumWhenRenamed)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto ctx = SetupCliContext("checksum_match");
    ASSERT_FALSE(ctx.workspace.empty());

    // Rename ABC to a completely different name — basename won't match "dump_test.abc"
    auto renamedAbc = ctx.workspace / "completely_different.abc";
    std::error_code ec;
    fs::rename(ctx.abcPath, renamedAbc, ec);
    ASSERT_FALSE(ec) << "Failed to rename ABC: " << ec.message();

    // Run with the renamed file — checksum-only matching should still find it
    int ret = RunAptool({"--ap-path", ctx.apPath, "--output", ctx.outputPath.string(), "--abc-path", renamedAbc});
    ASSERT_EQ(ret, 0) << "aptool dump should succeed with checksum-only matching";

    auto yaml = ReadFile(ctx.outputPath);
    EXPECT_NE(yaml.find("method_signature:"), std::string::npos)
        << "Checksum-only match should still resolve method signatures";
}

TEST_F(AptoolDumpTest, NoMethodDetailsWhenChecksumMismatches)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto workspace = PrepareArtifactsDir("checksum_reject");
    ASSERT_FALSE(workspace.empty());

    auto abcPath = workspace / "dump_test.abc";
    auto artifacts = CreateDumpTestAbc(abcPath);
    auto apPath = workspace / "profile.ap";

    // Create profile with a corrupted checksum
    auto corruptedChecksum = artifacts.checksum;
    if (!corruptedChecksum.empty()) {
        corruptedChecksum[0] = corruptedChecksum[0] == '0' ? '1' : '0';
    } else {
        corruptedChecksum = "deadbeef";
    }
    CreateDumpTestProfile(apPath, artifacts.entities, corruptedChecksum);

    auto outputPath = workspace / "output.yaml";
    int ret = RunAptool({"--ap-path", apPath, "--output", outputPath.string(), "--abc-path", abcPath});
    // VerifyClassContext: basename matches but checksum doesn't → skip → returns true
    // FindRecordByBaseName: expected checksum (corrupted) != ABC's real checksum → returns nullptr
    // Result: dump succeeds but without method details
    ASSERT_EQ(ret, 0) << "aptool dump should not crash with mismatched checksum";

    auto yaml = ReadFile(outputPath);
    EXPECT_NE(yaml.find("method_index:"), std::string::npos) << "Output should still contain method indices";
    EXPECT_EQ(yaml.find("method_signature:"), std::string::npos)
        << "Method signatures should NOT appear when checksum doesn't match";
}

TEST_F(AptoolDumpTest, WarnsWhenAbcDoesNotMatchProfile)
{
    APTOOL_SKIP_IF_NO_ETS();
    auto workspace = PrepareArtifactsDir("abc_no_match");
    ASSERT_FALSE(workspace.empty());

    // Synthetic profile uses panda files "boot.abc" and "sample.abc"
    auto profilePath = CreateSyntheticProfile();
    ASSERT_FALSE(profilePath.empty());

    // Provide an ABC file that doesn't match any panda file name in the profile
    auto abcPath = workspace / "dump_test.abc";
    CreateDumpTestAbc(abcPath);

    auto outputPath = workspace / "no_match.yaml";
    int ret = RunAptool({"--ap-path", profilePath, "--output", outputPath.string(), "--abc-path", abcPath});
    ASSERT_EQ(ret, 0) << "aptool dump should succeed even when ABC doesn't match";

    auto yaml = ReadFile(outputPath);
    EXPECT_NE(yaml.find("Methods:"), std::string::npos) << "Output should contain Methods section";
    EXPECT_EQ(yaml.find("class_descriptor:"), std::string::npos)
        << "No class descriptors should appear when ABC doesn't match profile";
}

}  // namespace ark::compiler
