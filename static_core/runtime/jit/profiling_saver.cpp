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

#include "profiling_saver.h"

#include <string>
#include <string_view>
#include <vector>

#include "jit/libprofile/pgo_class_context_utils.h"
#include "jit/libprofile/profile_merger.h"
#include "profiling_loader.h"

namespace ark {
namespace {

uint64_t ComputeDelta(uint64_t current, uint64_t last)
{
    if (current < last) {
        return current;
    }
    return current - last;
}

void FinalizePendingSaveState(ProfilingSaver::PendingLastSavedMap &pending)
{
    for (auto &[method, lastSaved] : pending) {
        if (method == nullptr) {
            continue;
        }
        auto *runtimeProfData = method->GetProfilingData();
        if (runtimeProfData == nullptr) {
            continue;
        }
        for (size_t i = 0; i < lastSaved.branches.size(); i++) {
            runtimeProfData->SetLastSavedBranchTaken(i, lastSaved.branches[i].taken);
            runtimeProfData->SetLastSavedBranchNotTaken(i, lastSaved.branches[i].notTaken);
        }
        for (size_t i = 0; i < lastSaved.throws.size(); i++) {
            runtimeProfData->SetLastSavedThrowTaken(i, lastSaved.throws[i]);
        }
        runtimeProfData->MarkLastSavedValid();
        runtimeProfData->DataSaved();
    }
}

struct DiskProfileState {
    pgo::AotProfilingData data;
    bool hasDiskProfile {false};
};

void LoadDiskProfile(const PandaString &saveFilePath, const PandaString &runtimeClassCtxStr,
                     ProfilingLoader &profilingLoader, DiskProfileState &diskProfile, PandaString &classCtxToSave)
{
    classCtxToSave = runtimeClassCtxStr;
    diskProfile.hasDiskProfile = false;
    auto profileCtxOrError = profilingLoader.LoadProfile(saveFilePath);
    if (!profileCtxOrError) {
        LOG(INFO, RUNTIME) << "[profile_saver] No previous profile data found. Saving new profile data.";
        return;
    }

    std::string ctxError;
    std::vector<std::string_view> ctxInputs;
    ctxInputs.emplace_back(profileCtxOrError->c_str(), profileCtxOrError->size());
    ctxInputs.emplace_back(runtimeClassCtxStr.c_str(), runtimeClassCtxStr.size());
    std::string mergedClassCtx;
    if (!pgo::PgoClassContextUtils::Merge(ctxInputs, mergedClassCtx, &ctxError)) {
        LOG(WARNING, RUNTIME)
            << "[profile_saver] Previous profile data found. Class context mismatch. Clearing old profile data: "
            << ctxError;
        return;
    }
    classCtxToSave = PandaString(mergedClassCtx);

    LOG(INFO, RUNTIME) << "[profile_saver] Previous profile data found. Merging with new profile data.";
    diskProfile.data = std::move(profilingLoader.GetAotProfilingData());
    diskProfile.hasDiskProfile = true;
}

}  // namespace

void ProfilingSaver::UpdateInlineCaches(pgo::AotProfilingData::AotCallSiteInlineCache *ic,
                                        std::vector<Class *> &runtimeClasses, pgo::AotProfilingData *profileData)
{
    for (uint32_t i = 0; i < runtimeClasses.size(); i++) {
        auto storedClass = ic->classes[i];

        auto runtimeCls = runtimeClasses[i];
        // Megamorphic Call, update first item, and return.
        if (i == 0 && CallSiteInlineCache::IsMegamorphic(runtimeCls)) {
            ic->classes[i] = {ic->MEGAMORPHIC_FLAG, -1};
            return;
        }

        if (storedClass.first == runtimeCls->GetFileId().GetOffset()) {
            continue;
        }

        auto clsPandaFile = runtimeCls->GetPandaFile()->GetFullFileName();
        auto pfIdx = profileData->GetPandaFileIdxByName(clsPandaFile);
        auto clsIdx = runtimeCls->GetFileId().GetOffset();

        ic->classes[i] = {clsIdx, pfIdx};
    }
}

void ProfilingSaver::CreateInlineCaches(pgo::AotProfilingData::AotMethodProfilingData *profilingData,
                                        Span<CallSiteInlineCache> &runtimeICs, pgo::AotProfilingData *profileData)
{
    auto icCount = runtimeICs.size();
    auto aotICs = profilingData->GetInlineCaches();
    for (size_t i = 0; i < icCount; i++) {
        aotICs[i] = {static_cast<uint32_t>(runtimeICs[i].GetBytecodePc()), {}};
        pgo::AotProfilingData::AotCallSiteInlineCache::ClearClasses(aotICs[i].classes);
        auto classes = runtimeICs[i].GetClassesCopy();
        UpdateInlineCaches(&aotICs[i], classes, profileData);
    }
}

void ProfilingSaver::CreateBranchData(pgo::AotProfilingData::AotMethodProfilingData *profilingData,
                                      Span<BranchData> &runtimeBranch, PendingMethodLastSaved &currentLastSaved,
                                      const ProfilingData &runtimeProfData, bool applyLastSaved)
{
    auto brCount = runtimeBranch.size();
    auto aotBrs = profilingData->GetBranchData();
    bool useLastSaved = applyLastSaved && runtimeProfData.HasLastSaved();
    ASSERT(currentLastSaved.branches.size() == brCount);

    for (size_t i = 0; i < brCount; i++) {
        auto pc = static_cast<uint32_t>(runtimeBranch[i].GetPc());
        uint64_t currentTaken = static_cast<uint64_t>(runtimeBranch[i].GetTakenCounter());
        uint64_t currentNotTaken = static_cast<uint64_t>(runtimeBranch[i].GetNotTakenCounter());
        currentLastSaved.branches[i] = {currentTaken, currentNotTaken};

        uint64_t taken = currentTaken;
        uint64_t notTaken = currentNotTaken;
        if (useLastSaved) {
            taken = ComputeDelta(currentTaken, runtimeProfData.GetLastSavedBranchTaken(i));
            notTaken = ComputeDelta(currentNotTaken, runtimeProfData.GetLastSavedBranchNotTaken(i));
        }
        aotBrs[i] = {pc, taken, notTaken};
    }
}

void ProfilingSaver::CreateThrowData(pgo::AotProfilingData::AotMethodProfilingData *profilingData,
                                     Span<ThrowData> &runtimeThrow, PendingMethodLastSaved &currentLastSaved,
                                     const ProfilingData &runtimeProfData, bool applyLastSaved)
{
    auto thCount = runtimeThrow.size();
    auto aotThs = profilingData->GetThrowData();
    bool useLastSaved = applyLastSaved && runtimeProfData.HasLastSaved();
    ASSERT(currentLastSaved.throws.size() == thCount);

    for (size_t i = 0; i < thCount; i++) {
        auto pc = static_cast<uint32_t>(runtimeThrow[i].GetPc());
        uint64_t currentTaken = static_cast<uint64_t>(runtimeThrow[i].GetTakenCounter());
        currentLastSaved.throws[i] = currentTaken;

        uint64_t taken = currentTaken;
        if (useLastSaved) {
            taken = ComputeDelta(currentTaken, runtimeProfData.GetLastSavedThrowTaken(i));
        }
        aotThs[i] = {pc, taken};
    }
}

void ProfilingSaver::AddMethod(pgo::AotProfilingData *profileData, Method *method, int32_t pandaFileIdx,
                               PendingLastSavedMap &pendingLastSaved, bool applyLastSaved)
{
    auto *runtimeProfData = method->GetProfilingData();
    if (UNLIKELY(runtimeProfData == nullptr)) {
        return;
    }
    auto runtimeICs = runtimeProfData->GetInlineCaches();
    uint32_t vcallsCount = runtimeICs.size();

    auto runtimeBrs = runtimeProfData->GetBranchData();
    uint32_t branchCount = runtimeBrs.size();

    auto runtimeThs = runtimeProfData->GetThrowData();
    uint32_t throwCount = runtimeThs.size();

    auto profilingData = pgo::AotProfilingData::AotMethodProfilingData(method->GetFileId().GetOffset(),
                                                                       method->GetClass()->GetFileId().GetOffset(),
                                                                       vcallsCount, branchCount, throwCount);
    PendingMethodLastSaved currentLastSaved;
    currentLastSaved.branches.resize(branchCount);
    currentLastSaved.throws.resize(throwCount);
    CreateInlineCaches(&profilingData, runtimeICs, profileData);
    CreateBranchData(&profilingData, runtimeBrs, currentLastSaved, *runtimeProfData, applyLastSaved);
    CreateThrowData(&profilingData, runtimeThs, currentLastSaved, *runtimeProfData, applyLastSaved);

    auto methodIdx = method->GetFileId().GetOffset();
    pendingLastSaved[method] = std::move(currentLastSaved);
    profileData->AddMethod(pandaFileIdx, methodIdx, std::move(profilingData));
}

// CC-OFFNXT(G.FUN.01-CPP, readability-function-size_parameters) grouping related save-state inputs keeps API clear.
// NOLINTNEXTLINE(readability-function-size)
uint32_t ProfilingSaver::AddProfiledMethods(pgo::AotProfilingData *profileData, PandaList<Method *> &profiledMethods,
                                            PandaList<Method *>::const_iterator profiledMethodsFinal,
                                            const PandaUnorderedSet<std::string> &saveablePandaFiles,
                                            PendingLastSavedMap &pendingLastSaved, bool applyLastSaved)
{
    uint32_t addedMethods = 0;
    bool notFinal = true;
    for (auto it = profiledMethods.cbegin(); notFinal; it++) {
        if ((*it) == (*profiledMethodsFinal)) {
            notFinal = false;
        }
        auto method = *it;
        ASSERT((method->GetProfilingData()) != nullptr);
        if (!method->GetProfilingData()->IsUpdateSinceLastSave()) {
            continue;
        }
        auto pandaFileName = method->GetPandaFile()->GetFullFileName();
        if (saveablePandaFiles.find(std::string(pandaFileName)) == saveablePandaFiles.end()) {
            continue;
        }
        auto pandaFileIdx = profileData->GetPandaFileIdxByName(pandaFileName);
        AddMethod(profileData, method, pandaFileIdx, pendingLastSaved, applyLastSaved);
        addedMethods++;
    }
    return addedMethods;
}

// CC-OFFNXT(G.FUN.01-CPP) Save path keeps fast-path and merge/fallback state transitions in one place for clarity.
// NOLINTNEXTLINE(readability-function-size)
void ProfilingSaver::SaveProfile(const PandaString &saveFilePath, const PandaString &classCtxStr,
                                 PandaList<Method *> &profiledMethods,
                                 PandaList<Method *>::const_iterator profiledMethodsFinal,
                                 PandaUnorderedSet<std::string> &allPandaFiles,
                                 const PandaUnorderedSet<std::string> &saveablePandaFiles)
{
    ProfilingLoader profilingLoader;
    DiskProfileState diskProfile;
    PandaString classCtxToSave;
    LoadDiskProfile(saveFilePath, classCtxStr, profilingLoader, diskProfile, classCtxToSave);

    pgo::AotPgoFile pgoFile;
    uint32_t writtenBytes = 0;
    PendingLastSavedMap pendingLastSaved;

    auto populateAotProfilingData = [this, &allPandaFiles, &profiledMethods, &profiledMethodsFinal, &saveablePandaFiles,
                                     &pendingLastSaved](pgo::AotProfilingData &profData,
                                                        bool applyLastSaved) -> uint32_t {
        // Add all panda files for pfIdx resolution (inline caches may reference classes from any file)
        profData.AddPandaFiles(allPandaFiles);
        // Only save methods from the saveable set
        return AddProfiledMethods(&profData, profiledMethods, profiledMethodsFinal, saveablePandaFiles,
                                  pendingLastSaved, applyLastSaved);
    };

    // Choose save strategy by on-disk state: full write for first save, otherwise merge with disk.
    if (!diskProfile.hasDiskProfile) {
        // Fast path persists full runtime counters and records snapshot in one pass.
        pgo::AotProfilingData fullRuntimeData;
        if (!populateAotProfilingData(fullRuntimeData, false)) {
            LOG(INFO, RUNTIME) << "[profile_saver] No updated methods for " << saveFilePath << ", skip saving.";
            return;
        }
        writtenBytes = pgoFile.Save(saveFilePath, &fullRuntimeData, classCtxToSave);
    } else {
        pgo::AotProfilingData runtimeProfileData;
        // Merge path feeds delta-transformed data into merger while still recording full snapshots.
        if (!populateAotProfilingData(runtimeProfileData, true)) {
            LOG(INFO, RUNTIME) << "[profile_saver] No updated methods for " << saveFilePath << ", skip saving.";
            return;
        }
        pgo::ProfileMerger merger;
        pgo::MergedProfile merged;
        PandaVector<const pgo::AotProfilingData *> inputs;
        inputs.push_back(&diskProfile.data);
        inputs.push_back(&runtimeProfileData);

        std::string mergeError;
        bool mergedOk = merger.Merge(inputs, merged, &mergeError);
        pgo::AotProfilingData fallbackData;
        auto *dataToSave = mergedOk ? &merged.data : &fallbackData;
        if (!mergedOk) {
            LOG(INFO, RUNTIME) << "[profile_saver] Merge failed. Saving new profile data only: " << mergeError;
            classCtxToSave = classCtxStr;
            populateAotProfilingData(fallbackData, false);
        }
        writtenBytes = pgoFile.Save(saveFilePath, dataToSave, classCtxToSave);
    }

    // Commit pending last-saved snapshots after profile data is successfully persisted.
    if (writtenBytes > 0) {
        FinalizePendingSaveState(pendingLastSaved);
        LOG(INFO, RUNTIME) << "[profile_saver] Profile data saved to " << saveFilePath << " with " << writtenBytes
                           << " bytes.";
    } else {
        LOG(ERROR, RUNTIME) << "[profile_saver] Failed to save profile data to " << saveFilePath << ".";
    }
}
}  // namespace ark
