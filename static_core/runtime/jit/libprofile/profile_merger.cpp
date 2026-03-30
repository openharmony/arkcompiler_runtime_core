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

#include "profile_merger.h"

#include <algorithm>
#include <limits>
#include <map>
#include <set>
#include <string_view>
#include <vector>

namespace ark::pgo {
namespace {

using InlineCache = AotProfilingData::AotCallSiteInlineCache;

// Sum with saturation to avoid overflow when accumulating counters.
uint64_t SaturatingAdd(uint64_t a, uint64_t b)
{
    uint64_t res = a + b;
    if (res < a) {
        return std::numeric_limits<uint64_t>::max();
    }
    return res;
}

struct MergedInlineCache {
    bool megamorphic {false};
    std::set<std::pair<uint32_t, PandaFileIdxType>> classes;
};

struct BranchCounters {
    uint64_t taken {0};
    uint64_t notTaken {0};
};

struct ThrowCounters {
    uint64_t taken {0};
};

struct MergedMethod {
    uint32_t classIdx {0};
    bool hasClassIdx {false};
    std::map<uint32_t, MergedInlineCache> inlineCaches;
    std::map<uint32_t, BranchCounters> branches;
    std::map<uint32_t, ThrowCounters> throws;
};

using MergedMethodsMap = std::map<PandaFileIdxType, std::map<uint32_t, MergedMethod>>;

bool ValidatePfIdx(PandaFileIdxType inPfIdx, const std::vector<PandaFileIdxType> &remap, std::string *error)
{
    if (inPfIdx < 0 || static_cast<size_t>(inPfIdx) >= remap.size() || remap[static_cast<size_t>(inPfIdx)] < 0) {
        if (error != nullptr) {
            *error = "invalid panda file index in methods section";
        }
        return false;
    }
    return true;
}

// Build output panda file list and index mapping in canonical path order.
bool BuildOutputPandaFiles(const PandaVector<const AotProfilingData *> &inputs, MergedProfile &output,
                           std::map<std::string_view, PandaFileIdxType> &outIndex, std::string *error)
{
    for (const auto &input : inputs) {
        if (input == nullptr) {
            if (error != nullptr) {
                *error = "null profiling data in input";
            }
            return false;
        }
        for (const auto &[_, pf] : input->GetPandaFileMapReverse()) {
            outIndex.try_emplace(pf, -1);
        }
    }
    for (auto &[pf, idx] : outIndex) {
        // Persist actual string storage first; outIndex key is string_view only.
        output.pandaFilesStorage.emplace_back(pf.data(), pf.size());
        idx = static_cast<PandaFileIdxType>(output.pandaFilesStorage.size() - 1U);
    }
    // AddPandaFiles stores string_view in AotProfilingData, so it must reference pandaFilesStorage.
    output.data.AddPandaFiles(output.pandaFilesStorage);
    return true;
}

// Map input panda file indices to output indices.
void BuildPfIdxRemap(const AotProfilingData &data, const std::map<std::string_view, PandaFileIdxType> &outIndex,
                     std::vector<PandaFileIdxType> &remap)
{
    const auto &pfMapRev = data.GetPandaFileMapReverse();
    remap.assign(pfMapRev.size(), -1);
    for (const auto &[inPfIdx, pf] : pfMapRev) {
        auto it = outIndex.find(pf);
        ASSERT(it != outIndex.end());
        auto remapIdx = static_cast<size_t>(inPfIdx);
        ASSERT(remapIdx < remap.size());
        remap[remapIdx] = it->second;
    }
}

// Merge inline cache targets per callsite with megamorphic detection.
void MergeInlineCaches(Span<const InlineCache> caches, const std::vector<PandaFileIdxType> &remap, MergedMethod &out)
{
    for (const auto &ic : caches) {
        auto &mergedIc = out.inlineCaches[ic.pc];
        if (mergedIc.megamorphic) {
            continue;
        }
        bool hasMega = false;
        for (const auto &cls : ic.classes) {
            if (cls.first == static_cast<uint32_t>(InlineCache::MEGAMORPHIC_FLAG)) {
                hasMega = true;
                break;
            }
            if (cls.second < 0) {
                continue;
            }
            auto inPf = static_cast<size_t>(cls.second);
            if (inPf >= remap.size()) {
                continue;
            }
            PandaFileIdxType outPf = remap[inPf];
            mergedIc.classes.insert({cls.first, outPf});
            if (mergedIc.classes.size() > InlineCache::CLASSES_COUNT) {
                hasMega = true;
                break;
            }
        }
        if (hasMega) {
            mergedIc.megamorphic = true;
            mergedIc.classes.clear();
        }
    }
}

// Merge branch counters by summation.
void MergeBranchData(Span<const AotProfilingData::AotBranchData> branches, MergedMethod &out)
{
    for (const auto &br : branches) {
        auto &entry = out.branches[br.pc];
        entry.taken = SaturatingAdd(entry.taken, br.taken);
        entry.notTaken = SaturatingAdd(entry.notTaken, br.notTaken);
    }
}

// Merge throw counters by summation.
void MergeThrowData(Span<const AotProfilingData::AotThrowData> throwsData, MergedMethod &out)
{
    for (const auto &th : throwsData) {
        auto &entry = out.throws[th.pc];
        entry.taken = SaturatingAdd(entry.taken, th.taken);
    }
}

// Merge one method into the aggregated map, enforcing classIdx consistency.
// CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters) method-merge inputs are coupled and kept explicit.
// NOLINTNEXTLINE(readability-function-size)
bool MergeSingleMethod(PandaFileIdxType outPfIdx, uint32_t methodIdx,
                       const AotProfilingData::AotMethodProfilingData &method,
                       const std::vector<PandaFileIdxType> &remap, MergedMethodsMap &merged, std::string *error)
{
    auto &mergedMethod = merged[outPfIdx][methodIdx];
    if (!mergedMethod.hasClassIdx) {
        mergedMethod.classIdx = method.GetClassIdx();
        mergedMethod.hasClassIdx = true;
    } else if (mergedMethod.classIdx != method.GetClassIdx()) {
        if (error != nullptr) {
            *error = "class index mismatch for method " + std::to_string(methodIdx);
        }
        return false;
    }
    MergeInlineCaches(method.GetInlineCaches(), remap, mergedMethod);
    MergeBranchData(method.GetBranchData(), mergedMethod);
    MergeThrowData(method.GetThrowData(), mergedMethod);
    return true;
}

// Merge all methods for a single panda file.
bool MergeMethodsForFile(PandaFileIdxType outPfIdx, const AotProfilingData::MethodsMap &methods,
                         const std::vector<PandaFileIdxType> &remap, MergedMethodsMap &merged, std::string *error)
{
    for (const auto &[methodIdx, method] : methods) {
        if (!MergeSingleMethod(outPfIdx, methodIdx, method, remap, merged, error)) {
            return false;
        }
    }
    return true;
}

// Merge methods from one input profile into the aggregated map.
bool MergeInputMethods(const AotProfilingData &input, const std::map<std::string_view, PandaFileIdxType> &outIndex,
                       MergedMethodsMap &merged, std::string *error)
{
    std::vector<PandaFileIdxType> remap;
    BuildPfIdxRemap(input, outIndex, remap);
    for (const auto &[inPfIdx, methods] : input.GetAllMethods()) {
        if (!ValidatePfIdx(inPfIdx, remap, error)) {
            return false;
        }
        PandaFileIdxType outPfIdx = remap[static_cast<size_t>(inPfIdx)];
        if (!MergeMethodsForFile(outPfIdx, methods, remap, merged, error)) {
            return false;
        }
    }
    return true;
}

void FillInlineCacheClasses(const MergedInlineCache &ic, InlineCache &outIc)
{
    if (ic.megamorphic || ic.classes.size() > InlineCache::CLASSES_COUNT) {
        outIc.classes[0] = {static_cast<uint32_t>(InlineCache::MEGAMORPHIC_FLAG), -1};
        return;
    }

    size_t idx = 0;
    for (const auto &cls : ic.classes) {
        outIc.classes[idx++] = cls;
    }
}

// Convert merged inline cache state into serialized profile records.
PandaVector<InlineCache> BuildInlineCachesForMethod(const MergedMethod &method)
{
    PandaVector<InlineCache> inlineCaches;
    inlineCaches.reserve(method.inlineCaches.size());
    for (const auto &[pc, ic] : method.inlineCaches) {
        InlineCache outIc {};
        outIc.pc = pc;
        InlineCache::ClearClasses(outIc.classes);
        FillInlineCacheClasses(ic, outIc);
        inlineCaches.emplace_back(outIc);
    }
    return inlineCaches;
}

// Convert merged branch counters into serialized profile records.
PandaVector<AotProfilingData::AotBranchData> BuildBranchDataForMethod(const MergedMethod &method)
{
    PandaVector<AotProfilingData::AotBranchData> branches;
    branches.reserve(method.branches.size());
    for (const auto &[pc, br] : method.branches) {
        branches.push_back({pc, br.taken, br.notTaken});
    }
    return branches;
}

// Convert merged throw counters into serialized profile records.
PandaVector<AotProfilingData::AotThrowData> BuildThrowDataForMethod(const MergedMethod &method)
{
    PandaVector<AotProfilingData::AotThrowData> throwsData;
    throwsData.reserve(method.throws.size());
    for (const auto &[pc, th] : method.throws) {
        throwsData.push_back({pc, th.taken});
    }
    return throwsData;
}

// Materialize merged methods into AotProfilingData.
void MaterializeMergedProfile(const MergedMethodsMap &merged, MergedProfile &output)
{
    for (const auto &[pfIdx, methods] : merged) {
        for (const auto &[methodIdx, method] : methods) {
            auto inlineCaches = BuildInlineCachesForMethod(method);
            auto branches = BuildBranchDataForMethod(method);
            auto throwsData = BuildThrowDataForMethod(method);

            AotProfilingData::AotMethodProfilingData profData(methodIdx, method.classIdx, std::move(inlineCaches),
                                                              std::move(branches), std::move(throwsData));
            output.data.AddMethod(pfIdx, methodIdx, std::move(profData));
        }
    }
}

}  // namespace

bool ProfileMerger::Merge(const PandaVector<const AotProfilingData *> &inputs, MergedProfile &output,
                          std::string *error) const
{
    if (inputs.empty()) {
        if (error != nullptr) {
            *error = "no input profiles";
        }
        return false;
    }

    output = {};

    std::map<std::string_view, PandaFileIdxType> outIndex;
    if (!BuildOutputPandaFiles(inputs, output, outIndex, error)) {
        return false;
    }

    MergedMethodsMap merged;
    for (const auto &input : inputs) {
        if (!MergeInputMethods(*input, outIndex, merged, error)) {
            return false;
        }
    }

    MaterializeMergedProfile(merged, output);
    return true;
}

}  // namespace ark::pgo
