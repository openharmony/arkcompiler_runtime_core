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
#ifndef PANDA_RUNTIME_MEM_OBJECT_HELPERS_H
#define PANDA_RUNTIME_MEM_OBJECT_HELPERS_H

#include <functional>

#include "libpandabase/utils/logger.h"
#include "libpandabase/mem/mem.h"

#include "runtime/mem/gc/gc_root_type.h"
#include "runtime/include/object_header.h"

namespace panda {
class Class;
class HClass;
class Field;
class ManagedThread;
class PandaVM;
}  // namespace panda

namespace panda::coretypes {
class DynClass;
class Array;
}  // namespace panda::coretypes

namespace panda::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_DEBUG_OBJ_HELPERS LOG(DEBUG, GC) << vm->GetGC()->GetLogPrefix()

class GC;

inline size_t GetObjectSize(const void *mem)
{
    return static_cast<const ObjectHeader *>(mem)->ObjectSize();
}

Logger::Buffer GetDebugInfoAboutObject(const ObjectHeader *header);

constexpr size_t GetMinimalObjectSize()
{
    return GetAlignedObjectSize(ObjectHeader::ObjectHeaderSize());
}

/**
 * Validate that object is correct from point of view of GC.
 * For example it checks that class of the object is not nullptr.
 * @param from_object object from which we found object by reference
 * @param object object which we want to validate
 */
inline void ValidateObject([[maybe_unused]] const ObjectHeader *from_object,
                           [[maybe_unused]] const ObjectHeader *object)
{
#ifndef NDEBUG
    if (object == nullptr) {
        return;
    }
    // from_object can be null, because sometimes we call Validate when we don't know previous object (for example when
    // we extract it from stack)
    if (object->template ClassAddr<BaseClass>() == nullptr) {
        LOG(ERROR, GC) << " Broken object doesn't have class: " << object << " accessed from object: " << from_object;
        UNREACHABLE();
    }
#endif  // !NDEBUG
}

/**
 * Validate that object (which is gc-root) is correct from point of view of GC
 * See ValidateObject(from_object, object) for further explanation
 * @param root_type type of the root
 * @param object object (root) which we want to validate
 */
inline void ValidateObject([[maybe_unused]] RootType root_type, [[maybe_unused]] const ObjectHeader *object)
{
#ifndef NDEBUG
    if (object == nullptr) {
        return;
    }
    ASSERT_DO(object->template ClassAddr<BaseClass>() != nullptr, LOG(FATAL, GC)
                                                                      << " Broken object doesn't have class: " << object
                                                                      << " accessed from root: " << root_type);
#endif  // !NDEBUG
}

void DumpObject(ObjectHeader *object_header, std::basic_ostream<char, std::char_traits<char>> *o_stream = &std::cerr);

void DumpClass(const Class *cls, std::basic_ostream<char, std::char_traits<char>> *o_stream = &std::cerr);

[[nodiscard]] ObjectHeader *GetForwardAddress(ObjectHeader *object_header);

const char *GetFieldName(const Field &field);

size_t GetDynClassInstanceSize(coretypes::DynClass *object);

class GCStaticObjectHelpers {
public:
    /**
     * Traverse all kinds of object_header and call obj_visitor for each reference field.
     */
    static void TraverseAllObjects(ObjectHeader *object_header,
                                   const std::function<void(ObjectHeader *, ObjectHeader *)> &obj_visitor);

    /**
     * Traverse all kinds of object_header and call handler for each reference field.
     * The handler accepts the object, the reference value, offset to the reference in the object and
     * the flag whether the field is volatile.
     */
    template <typename Handler>
    static void TraverseAllObjectsWithInfo(ObjectHeader *object, Handler &handler);

    static void UpdateRefsToMovedObjects(ObjectHeader *object);

    /**
     * Update a single reference field in the object to the moved value.
     * Return the moved value.
     */
    static ObjectHeader *UpdateRefToMovedObject(ObjectHeader *object, ObjectHeader *ref, uint32_t offset,
                                                bool is_volatile);

private:
    template <typename Handler>
    static void TraverseClass(Class *cls, Handler &handler);
    template <typename Handler>
    static void TraverseArray(coretypes::Array *array, Class *cls, Handler &handler);
    template <typename Handler>
    static void TraverseObject(ObjectHeader *object_header, Class *cls, Handler &handler);
};

class GCDynamicObjectHelpers {
public:
    /**
     * Traverse all kinds of object_header and call obj_visitor for each reference field.
     */
    static void TraverseAllObjects(ObjectHeader *object_header,
                                   const std::function<void(ObjectHeader *, ObjectHeader *)> &obj_visitor);

    /**
     * Traverse all kinds of object_header and call handler for each reference field.
     * The handler accepts the object, the reference value, offset to the reference in the object and
     * the flag whether the field is volatile.
     */
    template <typename Handler>
    static void TraverseAllObjectsWithInfo(ObjectHeader *object_header, Handler &handler);

    static void UpdateRefsToMovedObjects(ObjectHeader *object);

    /**
     * Update a single reference field in the object to the moved value.
     * Return the moved value.
     */
    static ObjectHeader *UpdateRefToMovedObject(ObjectHeader *object, ObjectHeader *ref, uint32_t offset,
                                                bool is_volatile);

    static void RecordDynWeakReference(GC *gc, coretypes::TaggedType *value);
    static void HandleDynWeakReferences(GC *gc);

private:
    template <typename Handler>
    static void TraverseArray(coretypes::Array *array, HClass *cls, Handler &handler);
    template <typename Handler>
    static void TraverseClass(coretypes::DynClass *dyn_class, Handler &handler);
    template <typename Handler>
    static void TraverseObject(ObjectHeader *object_header, HClass *cls, Handler &handler);

    static void UpdateDynArray(PandaVM *vm, coretypes::Array *array, array_size_t index, ObjectHeader *obj_ref);

    static void UpdateDynObjectRef(PandaVM *vm, ObjectHeader *object_header, size_t offset,
                                   ObjectHeader *field_obj_ref);
};

template <LangTypeT LangType>
class GCObjectHelpers {
};

template <>
class GCObjectHelpers<LANG_TYPE_STATIC> {
public:
    using Value = GCStaticObjectHelpers;
};

template <>
class GCObjectHelpers<LANG_TYPE_DYNAMIC> {
public:
    using Value = GCDynamicObjectHelpers;
};

template <LangTypeT LangType>
using ObjectHelpers = typename GCObjectHelpers<LangType>::Value;

}  // namespace panda::mem

#endif  // PANDA_OBJECT_HELPERS_H
