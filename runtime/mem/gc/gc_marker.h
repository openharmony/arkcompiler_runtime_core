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
#ifndef PANDA_RUNTIME_MEM_GC_GC_MARKER_H
#define PANDA_RUNTIME_MEM_GC_GC_MARKER_H

#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/bitmap.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/include/object_header.h"
#include "runtime/include/language_config.h"

namespace panda::coretypes {
class DynClass;
}  // namespace panda::coretypes

namespace panda::mem {

class GCMarkerBase {
public:
    explicit GCMarkerBase(GC *gc) : gc_(gc) {}

    GC *GetGC() const
    {
        return gc_;
    }

private:
    GC *gc_;
};

template <typename Marker, LangTypeT LANG_TYPE, bool HAS_VALUE_OBJECT_TYPES>
class GCMarker;

template <typename Marker>
class GCMarker<Marker, LANG_TYPE_STATIC, false> : public GCMarkerBase {
public:
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;

    explicit GCMarker(GC *gc) : GCMarkerBase(gc) {}

    void MarkInstance(GCMarkingStackType *objects_stack, const ReferenceCheckPredicateT &ref_pred,
                      const ObjectHeader *object, const BaseClass *cls);

private:
    Marker *AsMarker()
    {
        return static_cast<Marker *>(this);
    }

    /**
     * Iterate over all fields with references of object and add all not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param object
     * @param base_cls - class of object(used for perf in case if class for the object already was obtained)
     */
    void HandleObject(GCMarkingStackType *objects_stack, const ObjectHeader *object, const Class *cls);

    /**
     * Iterate over class data and add all found not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param cls - class
     */
    void HandleClass(GCMarkingStackType *objects_stack, const Class *cls);

    /**
     * For arrays of objects add all not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param array_object - array object
     * @param cls - class of array object(used for perf)
     */
    void HandleArrayClass(GCMarkingStackType *objects_stack, const coretypes::Array *array_object, const Class *cls);
};

template <typename Marker>
class GCMarker<Marker, LANG_TYPE_DYNAMIC, false> : public GCMarkerBase {
public:
    using ReferenceCheckPredicateT = typename GC::ReferenceCheckPredicateT;

    explicit GCMarker(GC *gc) : GCMarkerBase(gc) {}

    void MarkInstance(GCMarkingStackType *objects_stack, const ReferenceCheckPredicateT &ref_pred,
                      const ObjectHeader *object, const BaseClass *cls);

private:
    Marker *AsMarker()
    {
        return static_cast<Marker *>(this);
    }

    /**
     * Iterate over all fields with references of object and add all not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param object
     * @param base_cls - class of object(used for perf in case if class for the object already was obtained)
     */
    void HandleObject(GCMarkingStackType *objects_stack, const ObjectHeader *object, const BaseClass *cls);

    /**
     * Iterate over class data and add all found not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param cls - class
     */
    void HandleClass(GCMarkingStackType *objects_stack, const coretypes::DynClass *cls);

    /**
     * For arrays of objects add all not null object references to the objects_stack
     * @param objects_stack - stack with objects
     * @param array_object - array object
     * @param cls - class of array object(used for perf)
     */
    void HandleArrayClass(GCMarkingStackType *objects_stack, const coretypes::Array *array_object,
                          const BaseClass *cls);
};

template <typename Marker, LangTypeT LANG_TYPE, bool HAS_VALUE_OBJECT_TYPES>
class DefaultGCMarker : public GCMarker<Marker, LANG_TYPE, HAS_VALUE_OBJECT_TYPES> {
    using Base = GCMarker<Marker, LANG_TYPE, HAS_VALUE_OBJECT_TYPES>;

public:
    explicit DefaultGCMarker(GC *gc) : GCMarker<Marker, LANG_TYPE, HAS_VALUE_OBJECT_TYPES>(gc) {}

    template <bool reversed_mark = false>
    bool MarkIfNotMarked(ObjectHeader *object) const
    {
        MarkBitmap *bitmap = GetMarkBitMap(object);
        if (bitmap != nullptr) {
            if (atomic_mark_flag_) {
                return !bitmap->AtomicTestAndSet(object);
            }
            if (bitmap->Test(object)) {
                return false;
            }
            bitmap->Set(object);
            return true;
        }
        if (atomic_mark_flag_) {
            if (IsObjectHeaderMarked<reversed_mark, true>(object)) {
                return false;
            }
            MarkObjectHeader<reversed_mark, true>(object);
        } else {
            if (IsObjectHeaderMarked<reversed_mark, false>(object)) {
                return false;
            }
            MarkObjectHeader<reversed_mark, false>(object);
        }
        return true;
    }

    template <bool reversed_mark = false>
    void Mark(ObjectHeader *object) const
    {
        MarkBitmap *bitmap = GetMarkBitMap(object);
        if (bitmap != nullptr) {
            if (atomic_mark_flag_) {
                bitmap->AtomicTestAndSet(object);
            } else {
                bitmap->Set(object);
            }
            return;
        }
        if constexpr (reversed_mark) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            if (atomic_mark_flag_) {
                object->SetUnMarkedForGC<true>();
            } else {
                object->SetUnMarkedForGC<false>();
            }
            return;
        }
        if (atomic_mark_flag_) {
            object->SetMarkedForGC<true>();
        } else {
            object->SetMarkedForGC<false>();
        }
    }

    template <bool reversed_mark = false>
    void UnMark(ObjectHeader *object) const
    {
        MarkBitmap *bitmap = GetMarkBitMap(object);
        if (bitmap != nullptr) {
            return;  // no need for bitmap
        }
        if constexpr (reversed_mark) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            if (atomic_mark_flag_) {
                object->SetMarkedForGC<true>();
            } else {
                object->SetMarkedForGC<false>();
            }
            return;
        }
        if (atomic_mark_flag_) {
            object->SetUnMarkedForGC<true>();
        } else {
            object->SetUnMarkedForGC<false>();
        }
    }

    template <bool reversed_mark = false>
    bool IsMarked(const ObjectHeader *object) const
    {
        MarkBitmap *bitmap = GetMarkBitMap(object);
        if (bitmap != nullptr) {
            if (atomic_mark_flag_) {
                return bitmap->AtomicTest(object);
            }
            return bitmap->Test(object);
        }
        bool is_marked = atomic_mark_flag_ ? object->IsMarkedForGC<true>() : object->IsMarkedForGC<false>();
        if constexpr (reversed_mark) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            return !is_marked;
        }
        return is_marked;
    }

    template <bool reversed_mark = false>
    ObjectStatus MarkChecker(const ObjectHeader *object) const
    {
        if constexpr (!reversed_mark) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            // If ClassAddr is not set - it means object header initialization is in progress now
            if (object->AtomicClassAddr<BaseClass>() == nullptr) {
                return ObjectStatus::ALIVE_OBJECT;
            }
        }
        ObjectStatus object_status =
            IsMarked<reversed_mark>(object) ? ObjectStatus::ALIVE_OBJECT : ObjectStatus::DEAD_OBJECT;
        LOG(DEBUG, GC) << " Mark check for " << std::hex << object << std::dec
                       << " object is alive: " << static_cast<bool>(object_status);
        return object_status;
    }

    MarkBitmap *GetMarkBitMap(const void *object) const
    {
        for (auto bitmap : mark_bitmaps_) {
            if (bitmap->IsAddrInRange(object)) {
                return bitmap;
            }
        }
        return nullptr;
    }

    void ClearMarkBitMaps()
    {
        mark_bitmaps_.clear();
    }

    void AddMarkBitMap(MarkBitmap *bitmap)
    {
        mark_bitmaps_.push_back(bitmap);
    }

    template <typename It>
    void AddMarkBitMaps(It start, It end)
    {
        mark_bitmaps_.insert(mark_bitmaps_.end(), start, end);
    }

    bool HasBitMap(MarkBitmap *bitmap)
    {
        return std::find(mark_bitmaps_.begin(), mark_bitmaps_.end(), bitmap) != mark_bitmaps_.end();
    }

    void SetAtomicMark(bool flag)
    {
        atomic_mark_flag_ = flag;
    }

    bool GetAtomicMark() const
    {
        return atomic_mark_flag_;
    }

    void BindBitmaps(bool clear_pygote_space_bitmaps)
    {
        // Set marking bitmaps
        ClearMarkBitMaps();
        auto pygote_space_allocator = Base::GetGC()->GetObjectAllocator()->GetPygoteSpaceAllocator();
        if (pygote_space_allocator != nullptr) {
            // clear live bitmaps if we decide to rebuild it in full gc,
            // it will be used as marked bitmaps and updated at end of gc
            if (clear_pygote_space_bitmaps) {
                pygote_space_allocator->ClearLiveBitmaps();
            }
            auto &bitmaps = pygote_space_allocator->GetLiveBitmaps();
            AddMarkBitMaps(bitmaps.begin(), bitmaps.end());
        }
    }

private:
    template <bool reversed_mark = false, bool atomic_mark = true>
    bool IsObjectHeaderMarked(const ObjectHeader *object) const
    {
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (reversed_mark) {  // NOLINT(bugprone-suspicious-semicolon)
            return !object->IsMarkedForGC<atomic_mark>();
        }
        return object->IsMarkedForGC<atomic_mark>();
    }

    template <bool reversed_mark = false, bool atomic_mark = true>
    void MarkObjectHeader(ObjectHeader *object) const
    {
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (reversed_mark) {  // NOLINT(bugprone-suspicious-semicolon)
            object->SetUnMarkedForGC<atomic_mark>();
            return;
        }
        object->SetMarkedForGC<atomic_mark>();
    }

private:
    // Bitmaps for mark object
    PandaVector<MarkBitmap *> mark_bitmaps_;
    bool atomic_mark_flag_ = true;
};

template <class Marker>
class NoAtomicGCMarkerScope {
public:
    explicit NoAtomicGCMarkerScope(Marker *marker)
    {
        ASSERT(marker != nullptr);
        gc_marker_ = marker;
        old_state_ = gc_marker_->GetAtomicMark();
        if (old_state_) {
            gc_marker_->SetAtomicMark(false);
        }
    }

    NO_COPY_SEMANTIC(NoAtomicGCMarkerScope);
    NO_MOVE_SEMANTIC(NoAtomicGCMarkerScope);

    ~NoAtomicGCMarkerScope()
    {
        if (old_state_) {
            gc_marker_->SetAtomicMark(old_state_);
        }
    }

private:
    Marker *gc_marker_;
    bool old_state_ = false;
};

template <LangTypeT LANG_TYPE, bool HAS_VALUE_OBJECT_TYPES>
class DefaultGCMarkerImpl : public DefaultGCMarker<DefaultGCMarkerImpl<LANG_TYPE, HAS_VALUE_OBJECT_TYPES>, LANG_TYPE,
                                                   HAS_VALUE_OBJECT_TYPES> {
    using Base =
        DefaultGCMarker<DefaultGCMarkerImpl<LANG_TYPE, HAS_VALUE_OBJECT_TYPES>, LANG_TYPE, HAS_VALUE_OBJECT_TYPES>;

public:
    explicit DefaultGCMarkerImpl(GC *gc) : Base(gc) {}
};

}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_MARKER_H
