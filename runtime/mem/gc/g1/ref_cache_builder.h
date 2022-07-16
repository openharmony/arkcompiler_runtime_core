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
    RefCacheBuilder(G1GC<LanguageConfig> *gc, RefVector *refs, PandaVector<ObjectHeader *> *objects,
                    os::memory::Mutex *objects_lock)
        : gc_(gc), refs_(refs), objects_(objects), objects_lock_(objects_lock)
    {
    }

    bool operator()(ObjectHeader *object, ObjectHeader *field, uint32_t offset, bool is_volatile)
    {
        if (!gc_->InGCSweepRange(field)) {
            return true;
        }
        if (refs_->size() < refs_->capacity()) {
            // There is room to store references
            refs_->push_back(RefInfo(object, offset, is_volatile));
            ++refs_in_object_;
            return true;
        }
        // There is no room to store references.
        // Drop all references of the current object
        refs_->resize(refs_->size() - refs_in_object_);
        // Store the whole object
        os::memory::LockHolder lock(*objects_lock_);
        objects_->push_back(object);
        // Stop object traversing.
        return false;
    }

private:
    G1GC<LanguageConfig> *gc_;
    size_t refs_in_object_ = 0;
    RefVector *refs_;
    PandaVector<ObjectHeader *> *objects_ GUARDED_BY(objects_lock_);
    os::memory::Mutex *objects_lock_;
};
}  // namespace panda::mem
#endif  // PANDA_RUNTIME_MEM_GC_G1_REF_CACHE_BUILDER_H
