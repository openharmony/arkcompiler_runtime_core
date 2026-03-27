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

#include "runtime/include/class_index.h"

#include <securec.h>

#include "libarkbase/utils/hash.h"
#include "libarkbase/utils/logger.h"

namespace ark {

void ClassIndex::Builder::Add(const uint8_t *key, Index index)
{
    uint32_t hash = GetHash32String(key);
    uint32_t rehash = GetHash32String2(key);
    classes_.push_back(ClassEntry {
        hash,
        {rehash, index},
    });
}

void ClassIndex::Builder::Build(ClassIndex &out, mem::InternalAllocatorPtr allocator)
{
    LOG(DEBUG, CLASS_LINKER) << "ClassIndex: classes " << classes_.size();
    out.Clear(allocator);
    if (classes_.empty()) {
        return;
    }

    uint32_t length = classes_.size() * 3 / 2;  // use 1.5x spaces for hash table
    // serialize the hash table into linear table.
    // The hash table is a set of linked lists. Classes with the same hash entry will be linked together,
    // and index of the first entry is stored in the hashTable.
    Span<uint32_t> hashTable {allocator->AllocArray<uint32_t>(length), length};
    Span<HashEntry> bucket {allocator->AllocArray<HashEntry>(classes_.size()), classes_.size()};

    // init hash table to FF FF FF FF...
    memset_s(hashTable.data(), sizeof(uint32_t) * length, -1, sizeof(uint32_t) * length);

    for (uint32_t bucketIdx = 0; bucketIdx < classes_.size(); bucketIdx++) {
        auto &[hash, info] = classes_[bucketIdx];
        bucket[bucketIdx] = {INVALID_IDX, info};

        // find tail of the hash chain
        uint32_t *hashNext = &hashTable[hash % length];
        while (*hashNext != INVALID_IDX) {
            hashNext = &bucket[*hashNext].next;
        }
        *hashNext = bucketIdx;
    }

    out.hashTable_ = hashTable;
    out.bucket_ = bucket;
}

void ClassIndex::Clear(mem::InternalAllocatorPtr allocator)
{
    if (!hashTable_.empty()) {
        allocator->Free(hashTable_.data());
        hashTable_ = {};
    }
    if (!bucket_.empty()) {
        allocator->Free(bucket_.data());
        bucket_ = {};
    }
}

ClassIndex::Index ClassIndex::Find(const uint8_t *key) const
{
    if (hashTable_.empty()) {
        return INVALID_INDEX;
    }
    uint32_t hash = GetHash32String(key);
    uint32_t idx = hashTable_[hash % hashTable_.size()];
    if (idx == INVALID_IDX) {
        return INVALID_INDEX;
    }
    // find class entry in linked list with rehash
    uint32_t rehash = GetHash32String2(key);
    do {
        const HashEntry &entry = bucket_[idx];
        if (entry.info.rehash == rehash) {
            return entry.info.index;
        }
        idx = entry.next;
    } while (idx != INVALID_IDX);

    return INVALID_INDEX;
}

uint32_t ClassIndex::GetHash32String2(const uint8_t *key)
{
    static constexpr uint32_t SEED2 = 0x9747b28cU;
    using Hash2 = MurmurHash32<SEED2>;
    return Hash2::GetHash32String(key);
}

}  // namespace ark
