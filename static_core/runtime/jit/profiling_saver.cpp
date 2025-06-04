/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

namespace ark {
void ProfilingSaver::UpdateInlineCaches(pgo::AotProfilingData::AotCallSiteInlineCache *ic,
                                        std::vector<Class *> &runtimeClasses, pgo::AotProfilingData *profileData)
{
    for (uint32_t i = 0; i < runtimeClasses.size();) {
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
        i++;
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
                                      Span<BranchData> &runtimeBranch)
{
    auto brCount = runtimeBranch.size();
    auto aotBrs = profilingData->GetBranchData();

    for (size_t i = 0; i < brCount; i++) {
        aotBrs[i] = {static_cast<uint32_t>(runtimeBranch[i].GetPc()),
                     static_cast<uint64_t>(runtimeBranch[i].GetTakenCounter()),
                     static_cast<uint64_t>(runtimeBranch[i].GetNotTakenCounter())};
    }
}

void ProfilingSaver::CreateThrowData(pgo::AotProfilingData::AotMethodProfilingData *profilingData,
                                     Span<ThrowData> &runtimeThrow)
{
    auto thCount = runtimeThrow.size();
    auto aotThs = profilingData->GetThrowData();

    for (size_t i = 0; i < thCount; i++) {
        aotThs[i] = {static_cast<uint32_t>(runtimeThrow[i].GetPc()),
                     static_cast<uint64_t>(runtimeThrow[i].GetTakenCounter())};
    }
}

void ProfilingSaver::AddMethod(pgo::AotProfilingData *profileData, const Method *method, int32_t pandaFileIdx)
{
    auto *runtimeProfData = method->GetProfilingData();
    auto runtimeICs = runtimeProfData->GetInlineCaches();
    uint32_t vcallsCount = runtimeICs.size();

    auto runtimeBrs = runtimeProfData->GetBranchData();
    uint32_t branchCount = runtimeBrs.size();

    auto runtimeThs = runtimeProfData->GetThrowData();
    uint32_t throwCount = runtimeThs.size();

    auto profilingData = pgo::AotProfilingData::AotMethodProfilingData(method->GetFileId().GetOffset(),
                                                                       method->GetClass()->GetFileId().GetOffset(),
                                                                       vcallsCount, branchCount, throwCount);
    CreateInlineCaches(&profilingData, runtimeICs, profileData);
    CreateBranchData(&profilingData, runtimeBrs);
    CreateThrowData(&profilingData, runtimeThs);

    auto methodIdx = method->GetFileId().GetOffset();
    profileData->AddMethod(pandaFileIdx, methodIdx, std::move(profilingData));
}

void ProfilingSaver::AddProfiledMethods(pgo::AotProfilingData *profileData,
                                        PandaVector<const Method *> &profiledMethods)
{
    auto pfMap = profileData->GetPandaFileMap();
    for (auto method : profiledMethods) {
        auto pandaFileName = method->GetPandaFile()->GetFullFileName();
        if (pfMap.find(PandaString(pandaFileName)) == pfMap.end()) {
            continue;
        }
        auto pandaFileIdx = profileData->GetPandaFileIdxByName(pandaFileName);
        AddMethod(profileData, method, pandaFileIdx);
    }
}

void ProfilingSaver::SaveProfile(const PandaString &saveFilePath, const PandaString &classCtxStr,
                                 PandaVector<const Method *> profiledMethods,
                                 PandaUnorderedSet<std::string_view> &profiledPandaFiles)
{
    pgo::AotProfilingData profData;
    profData.AddPandaFiles(profiledPandaFiles);

    AddProfiledMethods(&profData, profiledMethods);

    pgo::AotPgoFile pgoFile;
    auto writtenBytes = pgoFile.Save(saveFilePath, &profData, classCtxStr);
    if (writtenBytes > 0) {
        LOG(INFO, RUNTIME) << "Profile data saved to " << saveFilePath << " with " << writtenBytes << " bytes.";
    } else {
        LOG(ERROR, RUNTIME) << "Failed to save profile data to " << saveFilePath << ".";
    }
}
}  // namespace ark