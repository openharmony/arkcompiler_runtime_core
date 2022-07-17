/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

namespace panda::panda_file {
class File;
}  // namespace panda::panda_file

namespace panda::compiler {
class Graph;

class SharedSlowPathData {
public:
    SharedSlowPathData()
    {
        ASSERT(std::all_of(entrypoints_offsets_.begin(), entrypoints_offsets_.end(), [](auto v) { return v == 0; }));
    }
    void SetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t offset)
    {
        entrypoints_offsets_[static_cast<size_t>(id)] = offset;
    }
    uintptr_t GetSharedSlowPathOffset(RuntimeInterface::EntrypointId id)
    {
        return entrypoints_offsets_[static_cast<size_t>(id)];
    }

private:
    static constexpr size_t SIZE = static_cast<size_t>(RuntimeInterface::EntrypointId::COUNT);
    std::array<uintptr_t, SIZE> entrypoints_offsets_ {};
};

class AotData {
    static constexpr uintptr_t INVALID_ADDRESS = std::numeric_limits<uintptr_t>::max();
    using AddressType = uintptr_t;
    using File = const panda_file::File;
    using MethodPtr = RuntimeInterface::MethodPtr;
    using ClassPtr = RuntimeInterface::ClassPtr;

public:
    AotData(const File *pfile, Graph *graph, AddressType code_addr, uint64_t *intf_inline_cache_index,
            std::map<std::pair<const File *, uint32_t>, int32_t> *got_plt,
            std::map<std::pair<const File *, uint32_t>, int32_t> *got_virt_indexes,
            std::map<std::pair<const File *, uint32_t>, int32_t> *got_class,
            std::map<std::pair<const File *, uint32_t>, int32_t> *got_string,
            std::map<std::pair<const File *, uint64_t>, int32_t> *got_intf_inline_cache,
            SharedSlowPathData *slow_path_data)
        : pfile_(pfile),
          graph_(graph),
          slow_path_data_(slow_path_data),
          code_address_(code_addr),
          intf_inline_cache_index_(intf_inline_cache_index),
          got_plt_(got_plt),
          got_virt_indexes_(got_virt_indexes),
          got_class_(got_class),
          got_string_(got_string),
          got_intf_inline_cache_(got_intf_inline_cache)
    {
    }

    intptr_t GetEpTableOffset() const;
    intptr_t GetEntrypointOffset(uint64_t pc, int32_t slot_id) const;
    intptr_t GetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t pc) const;
    void SetSharedSlowPathOffset(RuntimeInterface::EntrypointId id, uintptr_t pc);
    intptr_t GetPltSlotOffset(uint64_t pc, uint32_t method_id);
    intptr_t GetVirtIndexSlotOffset(uint64_t pc, uint32_t method_id);
    intptr_t GetClassSlotOffset(uint64_t pc, uint32_t klass_id, bool init);
    intptr_t GetStringSlotOffset(uint64_t pc, uint32_t string_id);
    uint64_t GetInfInlineCacheSlotOffset(uint64_t pc, uint64_t index);
    AddressType GetCodeOffset() const
    {
        return code_address_;
    }
    bool GetUseCha() const
    {
        return use_cha_;
    }
    void SetUseCha(bool use_cha)
    {
        use_cha_ = use_cha;
    }

    void SetIntfInlineCacheIndex(uint64_t intf_inline_cache_index)
    {
        *intf_inline_cache_index_ = intf_inline_cache_index;
    }

    uint64_t GetIntfInlineCacheIndex() const
    {
        return *intf_inline_cache_index_;
    }

private:
    inline int32_t GetSlotId() const;

    const File *pfile_;
    Graph *graph_ {nullptr};
    SharedSlowPathData *slow_path_data_;
    AddressType code_address_ {INVALID_ADDRESS};
    uint64_t *intf_inline_cache_index_;
    std::map<std::pair<const File *, uint32_t>, int32_t> *got_plt_;
    std::map<std::pair<const File *, uint32_t>, int32_t> *got_virt_indexes_;
    std::map<std::pair<const File *, uint32_t>, int32_t> *got_class_;
    std::map<std::pair<const File *, uint32_t>, int32_t> *got_string_;
    std::map<std::pair<const File *, uint64_t>, int32_t> *got_intf_inline_cache_;
    bool use_cha_ {false};
};
}  // namespace panda::compiler

#endif  // PANDA_AOT_DATA_H
