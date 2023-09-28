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
#ifndef PANDA_RUNTIME_MEM_GC_G1_REF_CACHE_BUILDER_H
#define PANDA_RUNTIME_MEM_GC_G1_REF_CACHE_BUILDER_H

#include "runtime/mem/gc/g1/g1-gc.h"

namespace panda::mem {
/**
 * Gets reference fields from an object and puts it to the ref collection.
 * The ref collection has limited size. If there is no room in the ref collection
 * the whole object is put to the object collection.
 */
template <class LanguageConfig>
class RefCacheBuilder {
    using RefVector = typename G1GC<LanguageConfig>::RefVector;

public:
    RefCacheBuilder(G1GC<LanguageConfig> *gc, PandaList<RefVector *> *refs, size_t region_size_bits,
                    GCMarkingStackType *objects_stack)
        : gc_(gc), refs_(refs), region_size_bits_(region_size_bits), objects_stack_(objects_stack)
    {
    }

    bool operator()(ObjectHeader *object, ObjectHeader *field, uint32_t offset, [[maybe_unused]] bool is_volatile)
    {
        if (!gc_->InGCSweepRange(field)) {
            all_cross_region_refs_processed_ &= panda::mem::IsSameRegion(object, field, region_size_bits_);
            return true;
        }
        RefVector *ref_vector = refs_->back();
        if (ref_vector->size() == ref_vector->capacity()) {
            // There is no room to store references.
            // Create a new vector and store everithing inside it
            auto *new_ref_vector = gc_->GetInternalAllocator()->template New<RefVector>();
            new_ref_vector->reserve(ref_vector->capacity() * 2);
            refs_->push_back(new_ref_vector);
            ref_vector = new_ref_vector;
        }
        ASSERT(ref_vector->size() < ref_vector->capacity());
        // There is room to store references
        ASSERT(objects_stack_ != nullptr);
        if (gc_->mixed_marker_.MarkIfNotMarkedInCollectionSet(field)) {
            objects_stack_->PushToStack(object, field);
        }
        ref_vector->emplace_back(object, offset);
        return true;
    }

    bool AllCrossRegionRefsProcessed() const
    {
        return all_cross_region_refs_processed_;
    }

private:
    G1GC<LanguageConfig> *gc_;
    PandaList<RefVector *> *refs_;
    bool all_cross_region_refs_processed_ = true;
    size_t region_size_bits_;
    // object stack pointer which will be used to store unmarked objects if it is not nullptr
    GCMarkingStackType *objects_stack_ = nullptr;
};
}  // namespace panda::mem
#endif  // PANDA_RUNTIME_MEM_GC_G1_REF_CACHE_BUILDER_H
