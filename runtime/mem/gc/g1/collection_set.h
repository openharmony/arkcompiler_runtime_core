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
#ifndef PANDA_RUNTIME_MEM_GC_G1_COLLECTION_SET_H
#define PANDA_RUNTIME_MEM_GC_G1_COLLECTION_SET_H

#include "libpandabase/macros.h"
#include "libpandabase/utils/range.h"
#include "runtime/mem/region_space.h"
#include "runtime/include/mem/panda_containers.h"

namespace panda::mem {

/**
 * Represent a set of regions grouped by type.
 */
class CollectionSet {
public:
    CollectionSet()
    {
        tenured_begin_ = 0;
        humongous_begin_ = 0;
    }

    explicit CollectionSet(PandaVector<Region *> &&young_regions) : collection_set_(young_regions)
    {
        tenured_begin_ = collection_set_.size();
        humongous_begin_ = tenured_begin_;
    }

    ~CollectionSet() = default;

    void AddRegion(Region *region)
    {
        ASSERT(region->HasFlag(RegionFlag::IS_OLD));
        collection_set_.push_back(region);
        if (!region->HasFlag(RegionFlag::IS_LARGE_OBJECT) && humongous_begin_ != collection_set_.size()) {
            std::swap(collection_set_[humongous_begin_], collection_set_.back());
            ++humongous_begin_;
        }
    }

    auto begin()
    {
        return collection_set_.begin();
    }

    auto begin() const
    {
        return collection_set_.begin();
    }

    auto end()
    {
        return collection_set_.end();
    }

    auto end() const
    {
        return collection_set_.end();
    }

    size_t size() const
    {
        return collection_set_.size();
    }

    bool empty() const
    {
        return collection_set_.empty();
    }

    auto Young()
    {
        return Range<PandaVector<Region *>::iterator>(begin(), begin() + tenured_begin_);
    }

    auto Young() const
    {
        return Range<PandaVector<Region *>::const_iterator>(begin(), begin() + tenured_begin_);
    }

    auto Tenured()
    {
        return Range<PandaVector<Region *>::iterator>(begin() + tenured_begin_, begin() + humongous_begin_);
    }

    auto Tenured() const
    {
        return Range<PandaVector<Region *>::const_iterator>(begin() + tenured_begin_, begin() + humongous_begin_);
    }

    auto Humongous()
    {
        return Range<PandaVector<Region *>::iterator>(begin() + humongous_begin_, end());
    }

    auto Humongous() const
    {
        return Range<PandaVector<Region *>::const_iterator>(begin() + humongous_begin_, end());
    }

    auto Movable()
    {
        return Range<PandaVector<Region *>::iterator>(begin(), begin() + humongous_begin_);
    }

    auto Movable() const
    {
        return Range<PandaVector<Region *>::const_iterator>(begin(), begin() + humongous_begin_);
    }

    DEFAULT_COPY_SEMANTIC(CollectionSet);
    DEFAULT_MOVE_SEMANTIC(CollectionSet);

private:
    PandaVector<Region *> collection_set_;
    size_t tenured_begin_;
    size_t humongous_begin_;
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_COLLECTION_SET_H
