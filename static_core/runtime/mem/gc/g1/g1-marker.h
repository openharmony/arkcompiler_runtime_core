/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_G1_G1_MARKER_H
#define PANDA_RUNTIME_MEM_GC_G1_G1_MARKER_H

#include "runtime/mem/gc/gc_marker.h"

namespace ark::mem {
template <class LanguageConfig>
class G1GCPauseMarker : public GCMarker<G1GCPauseMarker<LanguageConfig>, LanguageConfig::LANG_TYPE> {
public:
    explicit G1GCPauseMarker(GC *gc) : GCMarker<G1GCPauseMarker<LanguageConfig>, LanguageConfig::LANG_TYPE>(gc) {}

    ALWAYS_INLINE bool MarkIfNotMarked(ObjectHeader *object) const
    {
        MarkBitmap *bitmap = ObjectToRegion(object)->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        return !bitmap->AtomicTestAndSet(object);
    }

    ALWAYS_INLINE static bool IsMarked(const ObjectHeader *object)
    {
        MarkBitmap *bitmap = ObjectToRegion(object)->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        return bitmap->AtomicTest(object);
    }

    ALWAYS_INLINE static void Mark(ObjectHeader *object)
    {
        MarkBitmap *bitmap = ObjectToRegion(object)->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        bitmap->AtomicTestAndSet(object);
    }
};

template <class LanguageConfig>
class G1GCConcurrentMarker : public GCMarker<G1GCConcurrentMarker<LanguageConfig>, LanguageConfig::LANG_TYPE> {
public:
    explicit G1GCConcurrentMarker(GC *gc)
        : GCMarker<G1GCConcurrentMarker<LanguageConfig>, LanguageConfig::LANG_TYPE>(gc)
    {
    }

    ALWAYS_INLINE bool MarkIfNotMarked(ObjectHeader *object) const
    {
        MarkBitmap *bitmap = ObjectToRegion(object)->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        bool result = !bitmap->Test(object);
        bitmap->Set(object);
        return result;
    }

    ALWAYS_INLINE bool IsMarked(const ObjectHeader *object) const
    {
        MarkBitmap *bitmap = ObjectToRegion(object)->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        return bitmap->Test(object);
    }

    ALWAYS_INLINE void Mark(ObjectHeader *object)
    {
        MarkBitmap *bitmap = ObjectToRegion(object)->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        bitmap->Set(object);
    }
};

template <class LanguageConfig>
class G1GCMixedMarker : public GCMarker<G1GCMixedMarker<LanguageConfig>, LanguageConfig::LANG_TYPE> {
public:
    explicit G1GCMixedMarker(GC *gc) : GCMarker<G1GCMixedMarker<LanguageConfig>, LanguageConfig::LANG_TYPE>(gc) {}

    ALWAYS_INLINE bool MarkIfNotMarked(ObjectHeader *object) const
    {
        Region *objRegion = ObjectToRegion(object);
        if (!objRegion->IsInCollectionSet()) {
            return false;
        }
        MarkBitmap *bitmap = objRegion->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        return !bitmap->AtomicTestAndSet(object);
    }

    ALWAYS_INLINE bool MarkIfNotMarkedInCollectionSet(ObjectHeader *object) const
    {
        ASSERT(this->GetGC()->InGCSweepRange(object));
        MarkBitmap *bitmap = ObjectToRegion(object)->GetMarkBitmap();
        ASSERT(bitmap != nullptr);
        return !bitmap->AtomicTestAndSet(object);
    }

    ALWAYS_INLINE bool IsMarked(const ObjectHeader *object) const
    {
        return G1GCPauseMarker<LanguageConfig>::IsMarked(object);
    }

    ALWAYS_INLINE void Mark(ObjectHeader *object) const
    {
        G1GCPauseMarker<LanguageConfig>::Mark(object);
    }
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_G1_G1_MARKER_H
