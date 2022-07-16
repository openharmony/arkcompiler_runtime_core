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

#ifndef COMPILER_AOT_AOT_MANAGER_H
#define COMPILER_AOT_AOT_MANAGER_H

#include "aot_file.h"
#include "file.h"
#include "utils/arena_containers.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "utils/expected.h"

namespace panda::compiler {
class RuntimeInterface;

class AotManager {
    using BitSetElement = uint32_t;
    static constexpr size_t MASK_WIDTH = BITS_PER_BYTE * sizeof(BitSetElement);

public:
    explicit AotManager() = default;

    NO_MOVE_SEMANTIC(AotManager);
    NO_COPY_SEMANTIC(AotManager);
    ~AotManager() = default;

    Expected<bool, std::string> AddFile(const std::string &file_name, RuntimeInterface *runtime, uint32_t gc_type,
                                        bool force = false);

    const AotFile *GetFile(const std::string &file_name) const;

    const AotPandaFile *FindPandaFile(const std::string &file_name);

    PandaString GetBootClassContext() const
    {
        return boot_class_context_;
    }

    void SetBootClassContext(PandaString context)
    {
        boot_class_context_ = std::move(context);
    }

    PandaString GetAppClassContext() const
    {
        return app_class_context_;
    }

    void SetAppClassContext(PandaString context)
    {
        app_class_context_ = std::move(context);
    }

    void VerifyClassHierarchy(bool only_boot = false);

    uint32_t GetAotStringRootsCount()
    {
        // use counter to get roots count without acquiring vector's lock
        // Atomic with acquire order reason: data race with aot_string_gc_roots_count_ with dependecies on reads after
        // the load which should become visible
        return aot_string_gc_roots_count_.load(std::memory_order_acquire);
    }

    void RegisterAotStringRoot(ObjectHeader **slot, bool is_young);

    template <typename Callback>
    void VisitAotStringRoots(Callback cb, bool visit_only_young)
    {
        ASSERT(aot_string_gc_roots_.empty() ||
               (aot_string_young_set_.size() - 1) == (aot_string_gc_roots_.size() - 1) / MASK_WIDTH);

        if (!visit_only_young) {
            for (auto root : aot_string_gc_roots_) {
                cb(root);
            }
            return;
        }

        if (!has_young_aot_string_refs_) {
            return;
        }

        // Atomic with acquire order reason: data race with aot_string_gc_roots_count_ with dependecies on reads after
        // the load which should become visible
        size_t total_roots = aot_string_gc_roots_count_.load(std::memory_order_acquire);
        for (size_t idx = 0; idx < aot_string_young_set_.size(); idx++) {
            auto mask = aot_string_young_set_[idx];
            if (mask == 0) {
                continue;
            }
            for (size_t offset = 0; offset < MASK_WIDTH && idx * MASK_WIDTH + offset < total_roots; offset++) {
                if ((mask & (1ULL << offset)) != 0) {
                    cb(aot_string_gc_roots_[idx * MASK_WIDTH + offset]);
                }
            }
        }
    }

    template <typename Callback, typename IsYoungPredicate>
    void UpdateAotStringRoots(Callback cb, IsYoungPredicate p)
    {
        ASSERT(aot_string_gc_roots_.empty() ||
               (aot_string_young_set_.size() - 1) == (aot_string_gc_roots_.size() - 1) / MASK_WIDTH);

        has_young_aot_string_refs_ = false;
        size_t idx = 0;
        for (auto root : aot_string_gc_roots_) {
            cb(root);
            uint64_t bitmask = 1ULL << (idx % MASK_WIDTH);

            if ((aot_string_young_set_[idx / MASK_WIDTH] & bitmask) != 0) {
                bool is_young = p(*root);
                has_young_aot_string_refs_ |= is_young;
                if (!is_young) {
                    aot_string_young_set_[idx / MASK_WIDTH] &= ~bitmask;
                }
            }

            idx++;
        }
    }

    bool InAotFileRange(uintptr_t pc)
    {
        for (auto &aot_file : aot_files_) {
            auto code = reinterpret_cast<uintptr_t>(aot_file->GetCode());
            if (pc >= code && pc < code + reinterpret_cast<uintptr_t>(aot_file->GetCodeSize())) {
                return true;
            }
        }
        return false;
    }

    bool HasAotFiles()
    {
        return !aot_files_.empty();
    }

private:
    PandaVector<std::unique_ptr<AotFile>> aot_files_;
    PandaUnorderedMap<std::string, AotPandaFile> files_map_;
    PandaString boot_class_context_;
    PandaString app_class_context_;

    os::memory::RecursiveMutex aot_string_roots_lock_;
    PandaVector<ObjectHeader **> aot_string_gc_roots_;
    std::atomic_uint32_t aot_string_gc_roots_count_ {0};
    bool has_young_aot_string_refs_ {false};
    PandaVector<BitSetElement> aot_string_young_set_;
};

class AotClassContextCollector {
public:
    explicit AotClassContextCollector(PandaString *acc) : acc_(acc) {};
    bool operator()(const panda_file::File &pf);

    DEFAULT_MOVE_SEMANTIC(AotClassContextCollector);
    DEFAULT_COPY_SEMANTIC(AotClassContextCollector);
    ~AotClassContextCollector() = default;

private:
    PandaString *acc_;
};
}  // namespace panda::compiler

#endif  // COMPILER_AOT_AOT_MANAGER_H
