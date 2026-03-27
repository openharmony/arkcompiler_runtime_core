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

#ifndef PANDA_RUNTIME_CLASS_INDEX_H_
#define PANDA_RUNTIME_CLASS_INDEX_H_

#include <cstddef>
#include <cstdint>
#include <utility>

#include "runtime/include/mem/allocator.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark {

/**
 * Class Index.
 *
 * Uses a hashtable to index classes in ark files.
 *
 * The hashtable is a list of class entries, organized by the hashcode of the class name.
 */
class ClassIndex {
public:
    using Index = std::pair<uint32_t, uint32_t>;
    static constexpr uint32_t INVALID_IDX = uint32_t(-1);
    static constexpr auto INVALID_INDEX = std::make_pair(INVALID_IDX, INVALID_IDX);
    static constexpr size_t CLASS_ENTRY_SIZE = 16;

    struct ClassInfo {
        /** Use rehash to identify the class with the same hash values */
        uint32_t rehash = 0;
        /** Index of the abc file, and offset of the class in file */
        Index index;
    };

    struct HashEntry {
        /** Index of the next entry */
        uint32_t next = INVALID_IDX;
        ClassInfo info;
    };

    static_assert(sizeof(HashEntry) == CLASS_ENTRY_SIZE);

    class Builder {
    public:
        void Add(const uint8_t *key, Index index);

        void Build(ClassIndex &out, mem::InternalAllocatorPtr allocator);

    private:
        struct ClassEntry {
            uint32_t hash = 0;

            ClassInfo info;
        };
        static_assert(sizeof(ClassEntry) == CLASS_ENTRY_SIZE);

        PandaVector<ClassEntry> classes_;
    };

    bool Empty() const
    {
        return bucket_.empty();
    }

    void Clear(mem::InternalAllocatorPtr allocator);

    Index Find(const uint8_t *key) const;

private:
    static uint32_t GetHash32String2(const uint8_t *key);

    friend class Builder;

    Span<uint32_t> hashTable_;
    Span<HashEntry> bucket_;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_CLASS_INDEX_H_
