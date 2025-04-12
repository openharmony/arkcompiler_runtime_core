/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_AOT_DATA_H
#define PANDA_AOT_DATA_H

#include <map>
#include "runtime_interface.h"

namespace ark::panda_file {
class File;
}  // namespace ark::panda_file

namespace ark::compiler {
class Graph;

class SharedSlowPathData {
public:
    SharedSlowPathData()
    {
        ASSERT(std::all_of(entrypointsOffsets_.begin(), entrypointsOffsets_.end(), [](auto v) { return v == 0; }));
    }
    void SetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t offset)
    {
        entrypointsOffsets_[static_cast<size_t>(id)] = offset;
    }
    uintptr_t GetSharedSlowPathOffset(RuntimeInterface::EntrypointId id)
    {
        return entrypointsOffsets_[static_cast<size_t>(id)];
    }

private:
    static constexpr size_t SIZE = static_cast<size_t>(RuntimeInterface::EntrypointId::COUNT);
    std::array<uintptr_t, SIZE> entrypointsOffsets_ {};
};

class AotData {
    static constexpr uintptr_t INVALID_ADDRESS = std::numeric_limits<uintptr_t>::max();
    using AddressType = uintptr_t;
    using File = const panda_file::File;
    using MethodPtr = RuntimeInterface::MethodPtr;
    using ClassPtr = RuntimeInterface::ClassPtr;

public:
    struct AotDataArgs {
        File *pfile;
        Graph *graph;
        SharedSlowPathData *slowPathData;
        AddressType codeAddr;
        uint64_t *intfInlineCacheIndex;
        std::array<std::map<std::pair<const File *, uint32_t>, int32_t> *, 4U> mapArgs32;
        std::array<std::map<std::pair<const File *, uint64_t>, int32_t> *, 2U> mapArgs64;
    };

    explicit AotData(const AotDataArgs &args)
        : pfile_(args.pfile),
          graph_(args.graph),
          slowPathData_(args.slowPathData),
          codeAddress_(args.codeAddr),
          intfInlineCacheIndex_(args.intfInlineCacheIndex),
          gotPlt_(args.mapArgs32[0U]),
          gotVirtIndexes_(args.mapArgs32[1U]),
          gotClass_(args.mapArgs32[2U]),
          gotString_(args.mapArgs32[3U]),
          gotIntfInlineCache_(args.mapArgs64[0U]),
          gotCommon_(args.mapArgs64[1U])
    {
    }

    intptr_t GetEpTableOffset() const;
    intptr_t GetEntrypointOffset(uint64_t pc, int32_t slotId) const;
    intptr_t GetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t pc) const;
    void SetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t pc);
    intptr_t GetPltSlotOffset(uint64_t pc, uint32_t methodId);
    intptr_t GetVirtIndexSlotOffset(uint64_t pc, uint32_t methodId);
    intptr_t GetClassSlotOffset(uint64_t pc, uint32_t klassId, bool init);
    intptr_t GetCommonSlotOffset(uint64_t pc, uint32_t id);
    intptr_t GetStringSlotOffset(uint64_t pc, uint32_t stringId);
    uint64_t GetInfInlineCacheSlotOffset(uint64_t pc, uint64_t cacheIdx);

    int32_t GetClassSlotId(uint32_t klassId);
    int32_t GetStringSlotId(uint32_t stringId);
    int32_t GetPltSlotId(uint32_t methodId);
    int32_t GetIntfInlineCacheSlotId(uint64_t cacheIdx);

    AddressType GetCodeOffset() const
    {
        return codeAddress_;
    }
    bool GetUseCha() const
    {
        return useCha_;
    }
    void SetUseCha(bool useCha)
    {
        useCha_ = useCha;
    }

    void SetIntfInlineCacheIndex(uint64_t intfInlineCacheIndex)
    {
        *intfInlineCacheIndex_ = intfInlineCacheIndex;
    }

    uint64_t GetIntfInlineCacheIndex() const
    {
        return *intfInlineCacheIndex_;
    }

    bool HasProfileData() const
    {
        return hasProfileData_;
    }

    void SetHasProfileData(bool hasProfileData)
    {
        hasProfileData_ = hasProfileData;
    }

private:
    inline int32_t GetSlotId() const;

    const File *pfile_;
    Graph *graph_ {nullptr};
    SharedSlowPathData *slowPathData_;
    AddressType codeAddress_ {INVALID_ADDRESS};
    uint64_t *intfInlineCacheIndex_;
    std::map<std::pair<const File *, uint32_t>, int32_t> *gotPlt_;
    std::map<std::pair<const File *, uint32_t>, int32_t> *gotVirtIndexes_;
    std::map<std::pair<const File *, uint32_t>, int32_t> *gotClass_;
    std::map<std::pair<const File *, uint32_t>, int32_t> *gotString_;
    std::map<std::pair<const File *, uint64_t>, int32_t> *gotIntfInlineCache_;
    std::map<std::pair<const File *, uint64_t>, int32_t> *gotCommon_;
    bool useCha_ {false};
    bool hasProfileData_ {false};
};
}  // namespace ark::compiler

#endif  // PANDA_AOT_DATA_H
