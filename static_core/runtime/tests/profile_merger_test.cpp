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

#include <cstdint>
#include <iostream>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "jit/libprofile/profile_merger.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_options.h"

namespace ark::test {
namespace {

using InlineCache = pgo::AotProfilingData::AotCallSiteInlineCache;
using BranchData = pgo::AotProfilingData::AotBranchData;
using ThrowData = pgo::AotProfilingData::AotThrowData;

struct MergerInput {
    PandaVector<PandaString> pandaFilesStorage;
    pgo::AotProfilingData data;
};

struct MethodStatsSample {
    InlineCache inlineCache;
    BranchData branchData;
    ThrowData throwData;
};

void SetPandaFiles(MergerInput &input, std::initializer_list<const char *> pandaFiles)
{
    input.pandaFilesStorage.clear();
    input.pandaFilesStorage.reserve(pandaFiles.size());
    for (const auto *pf : pandaFiles) {
        input.pandaFilesStorage.emplace_back(pf);
    }
    input.data.AddPandaFiles(input.pandaFilesStorage);
}

InlineCache MakeInlineCache(uint32_t pc, std::initializer_list<std::pair<uint32_t, pgo::PandaFileIdxType>> classes)
{
    InlineCache ic {pc, {}};
    InlineCache::ClearClasses(ic.classes);
    size_t idx = 0;
    for (const auto &cls : classes) {
        if (idx >= InlineCache::CLASSES_COUNT) {
            break;
        }
        ic.classes[idx++] = cls;
    }
    return ic;
}

// CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters) helper keeps test-callsite readability.
// NOLINTNEXTLINE(readability-function-size)
void AddMethod(MergerInput &input, std::string_view pandaFile, uint32_t methodIdx, uint32_t classIdx,
               PandaVector<InlineCache> inlineCaches, PandaVector<BranchData> branches, PandaVector<ThrowData> throws)
{
    auto pfIdx = input.data.GetPandaFileIdxByName(pandaFile);
    ASSERT_NE(pfIdx, -1);
    pgo::AotProfilingData::AotMethodProfilingData methodData(methodIdx, classIdx, std::move(inlineCaches),
                                                             std::move(branches), std::move(throws));
    input.data.AddMethod(pfIdx, methodIdx, std::move(methodData));
}

MergerInput MakeInput(std::initializer_list<const char *> pandaFiles)
{
    MergerInput input;
    SetPandaFiles(input, pandaFiles);
    return input;
}

void AddMethodWithSingleStats(MergerInput &input, std::string_view pandaFile, uint32_t methodIdx, uint32_t classIdx,
                              const MethodStatsSample &sample)
{
    PandaVector<InlineCache> inlineCaches;
    inlineCaches.push_back(sample.inlineCache);
    PandaVector<BranchData> branches;
    branches.push_back(sample.branchData);
    PandaVector<ThrowData> throwsData;
    throwsData.push_back(sample.throwData);
    AddMethod(input, pandaFile, methodIdx, classIdx, std::move(inlineCaches), std::move(branches),
              std::move(throwsData));
}

void AddMethodWithSingleInlineCache(MergerInput &input, std::string_view pandaFile, uint32_t methodIdx,
                                    uint32_t classIdx, InlineCache inlineCache)
{
    PandaVector<InlineCache> inlineCaches;
    inlineCaches.push_back(inlineCache);
    PandaVector<BranchData> emptyBranches;
    PandaVector<ThrowData> emptyThrows;
    AddMethod(input, pandaFile, methodIdx, classIdx, std::move(inlineCaches), std::move(emptyBranches),
              std::move(emptyThrows));
}

PandaVector<const pgo::AotProfilingData *> MakeInputs(std::initializer_list<const pgo::AotProfilingData *> profiles)
{
    PandaVector<const pgo::AotProfilingData *> inputs;
    inputs.reserve(profiles.size());
    for (auto *profile : profiles) {
        inputs.push_back(profile);
    }
    return inputs;
}

const pgo::AotProfilingData::AotMethodProfilingData *FindMergedMethod(const pgo::MergedProfile &output,
                                                                      pgo::PandaFileIdxType pfIdx, uint32_t methodIdx)
{
    const auto &allMethods = output.data.GetAllMethods();
    auto fileIt = allMethods.find(pfIdx);
    if (fileIt == allMethods.end()) {
        return nullptr;
    }
    auto methodIt = fileIt->second.find(methodIdx);
    if (methodIt == fileIt->second.end()) {
        return nullptr;
    }
    return &methodIt->second;
}

std::set<std::pair<uint32_t, pgo::PandaFileIdxType>> GetNonEmptyClasses(const InlineCache &ic)
{
    std::set<std::pair<uint32_t, pgo::PandaFileIdxType>> out;
    for (const auto &cls : ic.classes) {
        if (cls.second < 0) {
            continue;
        }
        out.insert(cls);
    }
    return out;
}

void AppendMethodSignature(std::stringstream &ss, uint32_t pfIdx, uint32_t methodIdx,
                           const pgo::AotProfilingData::AotMethodProfilingData &method)
{
    ss << pfIdx << ':' << methodIdx << ':' << method.GetClassIdx() << "|IC{";
    for (const auto &ic : method.GetInlineCaches()) {
        ss << ic.pc << ':';
        for (const auto &cls : ic.classes) {
            ss << cls.first << '@' << cls.second << ',';
        }
        ss << ';';
    }
    ss << "}BR{";
    for (const auto &br : method.GetBranchData()) {
        ss << br.pc << ':' << br.taken << ':' << br.notTaken << ';';
    }
    ss << "}TH{";
    for (const auto &th : method.GetThrowData()) {
        ss << th.pc << ':' << th.taken << ';';
    }
    ss << "}|";
}

std::string BuildMergedProfileSignature(const pgo::MergedProfile &merged)
{
    std::stringstream ss;
    auto &pfMapRev = merged.data.GetPandaFileMapReverse();
    ss << "PF{";
    for (const auto &[pfIdx, pfName] : pfMapRev) {
        ss << pfIdx << '=' << pfName << ';';
    }
    ss << "}M{";

    auto &allMethods = merged.data.GetAllMethods();
    for (const auto &[pfIdx, methods] : allMethods) {
        for (const auto &[methodIdx, method] : methods) {
            AppendMethodSignature(ss, pfIdx, methodIdx, method);
        }
    }
    ss << '}';
    return ss.str();
}

}  // namespace

class ProfileMergerCoreTest : public testing::Test {
public:
    ProfileMergerCoreTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetRunGcInPlace(true);
        options.SetVerifyCallStack(false);
        options.SetGcType("epsilon");
        Runtime::Create(options);
    }

    ~ProfileMergerCoreTest() override
    {
        Runtime::Destroy();
    }
};

TEST_F(ProfileMergerCoreTest, RejectClassIdxMismatch)
{
    auto in1 = MakeInput({"/a.abc"});
    auto in2 = MakeInput({"/a.abc"});

    PandaVector<InlineCache> emptyIcs;
    PandaVector<BranchData> emptyBranches;
    PandaVector<ThrowData> emptyThrows;
    AddMethod(in1, "/a.abc", 10U, 100U, emptyIcs, emptyBranches, emptyThrows);
    AddMethod(in2, "/a.abc", 10U, 101U, emptyIcs, emptyBranches, emptyThrows);

    auto inputs = MakeInputs({&in1.data, &in2.data});

    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    std::string error;
    EXPECT_FALSE(merger.Merge(inputs, output, &error));
    EXPECT_NE(error.find("class index mismatch"), std::string::npos);
}

TEST_F(ProfileMergerCoreTest, MergeOldOnlyKeepsOldData)
{
    auto oldInput = MakeInput({"/a.abc"});
    auto newInput = MakeInput({"/a.abc"});

    PandaVector<InlineCache> oldIcs;
    oldIcs.push_back(MakeInlineCache(10U, {{111U, 0}}));
    PandaVector<BranchData> oldBranches;
    oldBranches.push_back({1U, 5U, 7U});
    PandaVector<ThrowData> oldThrows;
    oldThrows.push_back({2U, 9U});
    AddMethod(oldInput, "/a.abc", 100U, 1000U, std::move(oldIcs), std::move(oldBranches), std::move(oldThrows));

    auto inputs = MakeInputs({&oldInput.data, &newInput.data});

    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    std::string error;
    ASSERT_TRUE(merger.Merge(inputs, output, &error)) << error;

    auto *merged = FindMergedMethod(output, 0, 100U);
    ASSERT_NE(merged, nullptr);

    auto branches = merged->GetBranchData();
    ASSERT_EQ(branches.size(), 1U);
    EXPECT_EQ(branches[0].pc, 1U);
    EXPECT_EQ(branches[0].taken, 5U);
    EXPECT_EQ(branches[0].notTaken, 7U);

    auto throwsData = merged->GetThrowData();
    ASSERT_EQ(throwsData.size(), 1U);
    EXPECT_EQ(throwsData[0].pc, 2U);
    EXPECT_EQ(throwsData[0].taken, 9U);
}

TEST_F(ProfileMergerCoreTest, MergeNewOnlyKeepsNewData)
{
    auto oldInput = MakeInput({"/a.abc"});
    auto newInput = MakeInput({"/a.abc"});

    PandaVector<InlineCache> newIcs;
    newIcs.push_back(MakeInlineCache(20U, {{222U, 0}}));
    PandaVector<BranchData> newBranches;
    newBranches.push_back({3U, 8U, 13U});
    PandaVector<ThrowData> newThrows;
    newThrows.push_back({4U, 21U});
    AddMethod(newInput, "/a.abc", 101U, 1001U, std::move(newIcs), std::move(newBranches), std::move(newThrows));

    auto inputs = MakeInputs({&oldInput.data, &newInput.data});

    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    std::string error;
    ASSERT_TRUE(merger.Merge(inputs, output, &error)) << error;

    auto *merged = FindMergedMethod(output, 0, 101U);
    ASSERT_NE(merged, nullptr);

    auto branches = merged->GetBranchData();
    ASSERT_EQ(branches.size(), 1U);
    EXPECT_EQ(branches[0].pc, 3U);
    EXPECT_EQ(branches[0].taken, 8U);
    EXPECT_EQ(branches[0].notTaken, 13U);

    auto throwsData = merged->GetThrowData();
    ASSERT_EQ(throwsData.size(), 1U);
    EXPECT_EQ(throwsData[0].pc, 4U);
    EXPECT_EQ(throwsData[0].taken, 21U);
}

TEST_F(ProfileMergerCoreTest, MergeWithPfRemapAndStats)
{
    auto in1 = MakeInput({"/a.abc", "/b.abc"});
    auto in2 = MakeInput({"/b.abc", "/a.abc"});

    AddMethodWithSingleStats(
        in1, "/a.abc", 10U, 100U,
        {MakeInlineCache(11U, {{1000U, in1.data.GetPandaFileIdxByName("/b.abc")}}), {1U, 5U, 7U}, {2U, 3U}});
    AddMethodWithSingleStats(
        in2, "/a.abc", 10U, 100U,
        {MakeInlineCache(11U, {{2000U, in2.data.GetPandaFileIdxByName("/b.abc")}}), {1U, 2U, 1U}, {2U, 4U}});

    auto inputs = MakeInputs({&in1.data, &in2.data});

    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    std::string error;
    ASSERT_TRUE(merger.Merge(inputs, output, &error)) << error;

    auto &pfMapRev = output.data.GetPandaFileMapReverse();
    ASSERT_EQ(pfMapRev.size(), 2U);
    EXPECT_EQ(pfMapRev[0], "/a.abc");
    EXPECT_EQ(pfMapRev[1], "/b.abc");

    auto *method = FindMergedMethod(output, 0, 10U);
    ASSERT_NE(method, nullptr);

    auto branches = method->GetBranchData();
    ASSERT_EQ(branches.size(), 1U);
    EXPECT_EQ(branches[0].pc, 1U);
    EXPECT_EQ(branches[0].taken, 7U);
    EXPECT_EQ(branches[0].notTaken, 8U);

    auto throws = method->GetThrowData();
    ASSERT_EQ(throws.size(), 1U);
    EXPECT_EQ(throws[0].pc, 2U);
    EXPECT_EQ(throws[0].taken, 7U);

    auto inlineCaches = method->GetInlineCaches();
    ASSERT_EQ(inlineCaches.size(), 1U);
    std::set<std::pair<uint32_t, pgo::PandaFileIdxType>> expectedClasses {{1000U, 1}, {2000U, 1}};
    EXPECT_EQ(GetNonEmptyClasses(inlineCaches[0]), expectedClasses);
}

TEST_F(ProfileMergerCoreTest, EscalateToMegamorphicWhenClassTargetsOverflow)
{
    auto in1 = MakeInput({"/a.abc"});
    auto in2 = MakeInput({"/a.abc"});

    AddMethodWithSingleInlineCache(in1, "/a.abc", 10U, 100U,
                                   MakeInlineCache(20U, {{1U, 0}, {2U, 0}, {3U, 0}, {4U, 0}}));
    AddMethodWithSingleInlineCache(in2, "/a.abc", 10U, 100U, MakeInlineCache(20U, {{5U, 0}}));

    auto inputs = MakeInputs({&in1.data, &in2.data});

    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    std::string error;
    ASSERT_TRUE(merger.Merge(inputs, output, &error)) << error;

    auto *method = FindMergedMethod(output, 0, 10U);
    ASSERT_NE(method, nullptr);
    auto inlineCaches = method->GetInlineCaches();
    ASSERT_EQ(inlineCaches.size(), 1U);
    EXPECT_EQ(inlineCaches[0].classes[0].first, static_cast<uint32_t>(InlineCache::MEGAMORPHIC_FLAG));
    EXPECT_EQ(inlineCaches[0].classes[0].second, -1);
}

TEST_F(ProfileMergerCoreTest, SaturatingAddForBranchAndThrowCounters)
{
    auto in1 = MakeInput({"/a.abc"});
    auto in2 = MakeInput({"/a.abc"});

    PandaVector<InlineCache> emptyIcs;
    PandaVector<BranchData> in1Branches;
    in1Branches.push_back({1U, std::numeric_limits<uint64_t>::max() - 1U, 10U});
    PandaVector<ThrowData> in1Throws;
    in1Throws.push_back({2U, std::numeric_limits<uint64_t>::max()});
    AddMethod(in1, "/a.abc", 10U, 100U, emptyIcs, std::move(in1Branches), std::move(in1Throws));

    PandaVector<BranchData> in2Branches;
    in2Branches.push_back({1U, 10U, std::numeric_limits<uint64_t>::max()});
    PandaVector<ThrowData> in2Throws;
    in2Throws.push_back({2U, 1U});
    AddMethod(in2, "/a.abc", 10U, 100U, emptyIcs, std::move(in2Branches), std::move(in2Throws));

    auto inputs = MakeInputs({&in1.data, &in2.data});

    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    std::string error;
    ASSERT_TRUE(merger.Merge(inputs, output, &error)) << error;

    auto *method = FindMergedMethod(output, 0, 10U);
    ASSERT_NE(method, nullptr);
    auto branches = method->GetBranchData();
    ASSERT_EQ(branches.size(), 1U);
    EXPECT_EQ(branches[0].taken, std::numeric_limits<uint64_t>::max());
    EXPECT_EQ(branches[0].notTaken, std::numeric_limits<uint64_t>::max());

    auto throws = method->GetThrowData();
    ASSERT_EQ(throws.size(), 1U);
    EXPECT_EQ(throws[0].taken, std::numeric_limits<uint64_t>::max());
}

TEST_F(ProfileMergerCoreTest, RejectNullInputPointers)
{
    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    std::string error;

    PandaVector<const pgo::AotProfilingData *> inputsWithNullData;
    inputsWithNullData.push_back(nullptr);
    error.clear();
    EXPECT_FALSE(merger.Merge(inputsWithNullData, output, &error));
    EXPECT_NE(error.find("null profiling data"), std::string::npos);
}

TEST_F(ProfileMergerCoreTest, RejectEmptyInputs)
{
    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    PandaVector<const pgo::AotProfilingData *> emptyInputs;
    std::string error;

    EXPECT_FALSE(merger.Merge(emptyInputs, output, &error));
    EXPECT_NE(error.find("no input profiles"), std::string::npos);
}

TEST_F(ProfileMergerCoreTest, RejectInvalidMethodPfIdx)
{
    auto input = MakeInput({"/a.abc"});
    pgo::AotProfilingData::AotMethodProfilingData methodData(10U, 100U, 0U, 0U, 0U);
    // Inject a methods-section pfIdx that is not representable by input pandaFiles list.
    input.data.AddMethod(1, 10U, std::move(methodData));

    auto inputs = MakeInputs({&input.data});

    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    std::string error;
    EXPECT_FALSE(merger.Merge(inputs, output, &error));
    EXPECT_NE(error.find("invalid panda file index"), std::string::npos);
}

TEST_F(ProfileMergerCoreTest, DeduplicateInputPandaFilesUseCanonicalPathOrder)
{
    auto in1 = MakeInput({"/c.abc", "/a.abc", "/c.abc"});
    auto in2 = MakeInput({"/b.abc", "/a.abc", "/b.abc"});

    auto inputs = MakeInputs({&in1.data, &in2.data});

    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    std::string error;
    ASSERT_TRUE(merger.Merge(inputs, output, &error)) << error;

    auto &pfMapRev = output.data.GetPandaFileMapReverse();
    ASSERT_EQ(pfMapRev.size(), 3U);
    auto it = pfMapRev.begin();
    ASSERT_NE(it, pfMapRev.end());
    EXPECT_EQ(it->second, "/a.abc");
    ++it;
    ASSERT_NE(it, pfMapRev.end());
    EXPECT_EQ(it->second, "/b.abc");
    ++it;
    ASSERT_NE(it, pfMapRev.end());
    EXPECT_EQ(it->second, "/c.abc");
    ++it;
    EXPECT_EQ(it, pfMapRev.end());
}

TEST_F(ProfileMergerCoreTest, MergeOrderIndependent)
{
    auto in1 = MakeInput({"/c.abc", "/a.abc"});
    auto in2 = MakeInput({"/b.abc", "/a.abc"});

    AddMethodWithSingleStats(
        in1, "/a.abc", 10U, 100U,
        {MakeInlineCache(10U, {{100U, in1.data.GetPandaFileIdxByName("/c.abc")}}), {1U, 5U, 7U}, {2U, 3U}});
    AddMethodWithSingleStats(
        in2, "/a.abc", 10U, 100U,
        {MakeInlineCache(10U, {{200U, in2.data.GetPandaFileIdxByName("/b.abc")}}), {1U, 11U, 13U}, {2U, 17U}});

    pgo::ProfileMerger merger;
    std::string error;

    auto inputs12 = MakeInputs({&in1.data, &in2.data});
    pgo::MergedProfile out12;
    ASSERT_TRUE(merger.Merge(inputs12, out12, &error)) << error;

    auto inputs21 = MakeInputs({&in2.data, &in1.data});
    pgo::MergedProfile out21;
    ASSERT_TRUE(merger.Merge(inputs21, out21, &error)) << error;

    EXPECT_EQ(BuildMergedProfileSignature(out12), BuildMergedProfileSignature(out21));
}

// CC-OFFNXT(G.FUN.01-CPP) test validates grouping-independence end to end.
TEST_F(ProfileMergerCoreTest, MergeGroupingIndependent)
{
    auto inA = MakeInput({"/c.abc", "/a.abc"});
    auto inB = MakeInput({"/b.abc", "/a.abc"});
    auto inC = MakeInput({"/d.abc", "/a.abc"});

    AddMethodWithSingleStats(
        inA, "/a.abc", 10U, 100U,
        {MakeInlineCache(10U, {{100U, inA.data.GetPandaFileIdxByName("/c.abc")}}), {1U, 2U, 3U}, {2U, 5U}});
    AddMethodWithSingleStats(
        inB, "/a.abc", 10U, 100U,
        {MakeInlineCache(10U, {{200U, inB.data.GetPandaFileIdxByName("/b.abc")}}), {1U, 7U, 11U}, {2U, 13U}});
    AddMethodWithSingleStats(
        inC, "/a.abc", 10U, 100U,
        {MakeInlineCache(10U, {{300U, inC.data.GetPandaFileIdxByName("/d.abc")}}), {1U, 17U, 19U}, {2U, 23U}});

    pgo::ProfileMerger merger;
    std::string error;

    auto inputsABC = MakeInputs({&inA.data, &inB.data, &inC.data});
    pgo::MergedProfile outABC;
    ASSERT_TRUE(merger.Merge(inputsABC, outABC, &error)) << error;

    auto inputsAB = MakeInputs({&inA.data, &inB.data});
    pgo::MergedProfile outAB;
    ASSERT_TRUE(merger.Merge(inputsAB, outAB, &error)) << error;

    auto inputsABThenC = MakeInputs({&outAB.data, &inC.data});
    pgo::MergedProfile outABThenC;
    ASSERT_TRUE(merger.Merge(inputsABThenC, outABThenC, &error)) << error;

    auto inputsBC = MakeInputs({&inB.data, &inC.data});
    pgo::MergedProfile outBC;
    ASSERT_TRUE(merger.Merge(inputsBC, outBC, &error)) << error;

    auto inputsAThenBC = MakeInputs({&inA.data, &outBC.data});
    pgo::MergedProfile outAThenBC;
    ASSERT_TRUE(merger.Merge(inputsAThenBC, outAThenBC, &error)) << error;

    auto sigABC = BuildMergedProfileSignature(outABC);
    EXPECT_EQ(sigABC, BuildMergedProfileSignature(outABThenC));
    EXPECT_EQ(sigABC, BuildMergedProfileSignature(outAThenBC));
}

TEST_F(ProfileMergerCoreTest, MergeSucceedsWhenStatsIsNull)
{
    auto input = MakeInput({"/a.abc"});

    PandaVector<InlineCache> emptyIcs;
    PandaVector<BranchData> branches;
    branches.push_back({1U, 2U, 3U});
    PandaVector<ThrowData> throwsData;
    throwsData.push_back({2U, 4U});
    AddMethod(input, "/a.abc", 10U, 100U, std::move(emptyIcs), std::move(branches), std::move(throwsData));

    auto inputs = MakeInputs({&input.data});

    pgo::ProfileMerger merger;
    pgo::MergedProfile output;
    std::string error;
    ASSERT_TRUE(merger.Merge(inputs, output, &error)) << error;

    auto *method = FindMergedMethod(output, 0, 10U);
    ASSERT_NE(method, nullptr);
    auto mergedBranches = method->GetBranchData();
    ASSERT_EQ(mergedBranches.size(), 1U);
    EXPECT_EQ(mergedBranches[0].taken, 2U);
    EXPECT_EQ(mergedBranches[0].notTaken, 3U);
}

}  // namespace ark::test
