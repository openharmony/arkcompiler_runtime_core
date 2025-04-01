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

#ifndef AOT_PROFILING_DATA_H
#define AOT_PROFILING_DATA_H

#include "utils/span.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/jit/profiling_data.h"

#include <cstdint>
#include <atomic>

namespace ark::pgo {
using PandaFileIdxType = int32_t;  // >= -1

class AotProfilingData {
public:
    class AotCallSiteInlineCache;
    class AotBranchData;
    class AotThrowData;
    class AotMethodProfilingData {  // NOLINT(cppcoreguidelines-special-member-functions)
    public:
        AotMethodProfilingData(uint32_t methodIdx, uint32_t classIdx, uint32_t inlineCaches, uint32_t branchData,
                               uint32_t throwData)
            : methodIdx_(methodIdx),
              classIdx_(classIdx),
              inlineCaches_(inlineCaches),
              branchData_(branchData),
              throwData_(throwData)
        {
        }
        ~AotMethodProfilingData() = default;

        Span<AotCallSiteInlineCache> GetInlineCaches()
        {
            return Span<AotCallSiteInlineCache>(inlineCaches_.data(), inlineCaches_.size());
        }

        Span<AotBranchData> GetBranchData()
        {
            return Span<AotBranchData>(branchData_.data(), branchData_.size());
        }

        Span<AotThrowData> GetThrowData()
        {
            return Span<AotThrowData>(throwData_.data(), throwData_.size());
        }

        uint32_t GetMethodIdx() const
        {
            return methodIdx_;
        }

        uint32_t GetClassIdx() const
        {
            return classIdx_;
        }

    private:
        uint32_t methodIdx_;
        uint32_t classIdx_;

        PandaVector<AotCallSiteInlineCache> inlineCaches_;
        PandaVector<AotBranchData> branchData_;
        PandaVector<AotThrowData> throwData_;
    };

#pragma pack(push, 4)
    class AotCallSiteInlineCache {  // NOLINT(cppcoreguidelines-pro-type-member-init)
    public:
        static constexpr size_t CLASSES_COUNT = 4;               // CC-OFF(G.NAM.03-CPP) project code style
        static constexpr int32_t MEGAMORPHIC_FLAG = 0xFFFFFFFF;  // CC-OFF(G.NAM.03-CPP) project code style

        void SetBytecodePc(std::atomic_uintptr_t pcPtr)
        {
            pc_ = pcPtr;
        }

        void InitClasses()
        {
            std::fill(classes_.begin(), classes_.end(), std::make_pair(0, -1));
        }

        uint32_t GetClassesSize()
        {
            return classes_.size();
        }

        std::pair<uint32_t, PandaFileIdxType> GetInlineCache(uint32_t idx)
        {
            return classes_[idx];
        }

        void SetInlineCache(uint32_t idx, uint32_t classIdx, PandaFileIdxType fileIdx)
        {
            classes_[idx] = std::make_pair(classIdx, fileIdx);
        }

    private:
        uint32_t pc_;
        std::array<std::pair<uint32_t, PandaFileIdxType>, CLASSES_COUNT> classes_;
    };

    class AotBranchData {
    public:
        void SetBranchData(uint32_t pc, uint64_t taken, uint64_t notTaken)
        {
            pc_ = pc;
            taken_ = taken;
            notTaken_ = notTaken;
        }

    private:
        uint32_t pc_;
        uint64_t taken_;
        uint64_t notTaken_;
    };

    class AotThrowData {
    public:
        void SetThrowData(uint32_t pc, uint64_t taken)
        {
            pc_ = pc;
            taken_ = taken;
        }

    private:
        uint32_t pc_;
        uint64_t taken_;
    };
#pragma pack(pop)

public:
    PandaMap<std::string_view, PandaFileIdxType> &GetPandaFileMap()
    {
        return pandaFileMap_;
    }

    PandaMap<PandaFileIdxType, std::string_view> &GetPandaFileMapReverse()
    {
        return pandaFileMapRev_;
    }

    using MethodsMap = PandaMap<uint32_t, AotMethodProfilingData>;
    PandaMap<PandaFileIdxType, MethodsMap> &GetAllMethods()
    {
        return allMethodsMap_;
    }

    int32_t GetPandaFileIdxByName(const std::string &pandaFileName)
    {
        auto pfIdx = pandaFileMap_.find(pandaFileName);
        if (pfIdx == pandaFileMap_.end()) {
            return -1;
        }
        return pfIdx->second;
    }

    void AddPandaFiles(PandaUnorderedSet<std::string_view> &profiledPandaFiles)
    {
        uint32_t count = 0;
        for (auto &pandaFile : profiledPandaFiles) {
            pandaFileMap_.insert(std::make_pair(pandaFile, count));
            pandaFileMapRev_.insert(std::make_pair(count, pandaFile));
            allMethodsMap_.insert(std::make_pair(count, MethodsMap()));
            count++;
        }
    }

    void AddMethod(PandaFileIdxType pfIdx, uint32_t methodIdx, AotMethodProfilingData &&profData)
    {
        allMethodsMap_[pfIdx].insert(std::make_pair(methodIdx, profData));
    }

private:
    PandaMap<std::string_view, PandaFileIdxType> pandaFileMap_;
    PandaMap<PandaFileIdxType, std::string_view> pandaFileMapRev_;
    PandaMap<PandaFileIdxType, MethodsMap> allMethodsMap_;
};
}  // namespace ark::pgo

#endif  // AOT_PROFILING_DATA_H
