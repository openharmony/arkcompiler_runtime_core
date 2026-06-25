/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_RUNTIME_COMMON_INTERFACES_OBJECTS_BASE_OBJECT_H
#define COMMON_RUNTIME_COMMON_INTERFACES_OBJECTS_BASE_OBJECT_H

#include <cstdint>
#include <cstdio>

#include "common_interfaces/objects/ref_field.h"
#include "runtime/include/object_header.h"
#include "common_interfaces/objects/base_class.h"
#include "common_interfaces/objects/base_state_word.h"
#include "common_interfaces/heap/visitor.h"

namespace ark::common_vm {

using MemoryOrder = std::memory_order;
using ::ark::mem::MAddress;
using ::ark::mem::RefField;
using ::ark::mem::RefFieldVisitor;
using ::ark::mem::TypeInfo;

class BaseObject : public ObjectHeader {
public:
    BaseObject() = default;
    static BaseObject *Cast(MAddress address)
    {
        return reinterpret_cast<BaseObject *>(address);
    }

    inline size_t GetSize() const
    {
        return reinterpret_cast<const ObjectHeader *>(this)->ObjectSize();
    }

    inline bool IsValidObject() const
    {
        return true;
    }

    void ForEachRefField(const RefFieldVisitor &fieldHandler, const RefFieldVisitor &weakFieldHandler);

    inline BaseObject *GetForwardingPointer() const
    {
        if (IsForwarded()) {
            return reinterpret_cast<BaseObject *>(
                reinterpret_cast<const ObjectHeader *>(this)->GetMark().GetForwardingAddress());
        }
        return nullptr;
    }

    inline bool IsForwarding() const
    {
        return reinterpret_cast<const ObjectHeader *>(this)->GetMark().IsReadBarrierSet();
    }

    static inline intptr_t FieldOffset(BaseObject *object, const void *field)
    {
        return reinterpret_cast<intptr_t>(field) - reinterpret_cast<intptr_t>(object);
    }

    // The interfaces following only use for common code compiler. It will be deleted later.
    TypeInfo *GetComponentTypeInfo() const
    {
        return reinterpret_cast<TypeInfo *>(const_cast<BaseObject *>(this));
    }

    inline bool HasRefField() const
    {
        return true;
    }

    inline TypeInfo *GetTypeInfo() const
    {
        return reinterpret_cast<TypeInfo *>(const_cast<BaseObject *>(this));
    }

    bool IsRawArray() const
    {
        return true;
    }

    void ForEachRefInStruct([[maybe_unused]] const RefFieldVisitor &visitor, [[maybe_unused]] MAddress aggStart,
                            [[maybe_unused]] MAddress aggEnd)
    {
    }

    BaseClass *GetBaseClass() const
    {
        return ClassAddr<BaseClass>();
    }

    // Size of object header
    static constexpr size_t BaseObjectSize()
    {
        return sizeof(BaseObject);
    }
};

using ObjectPtr = BaseObject *;
using ObjectVisitor = std::function<void(ObjectPtr)>;

struct ObjectRef {
    BaseObject *object;
};

using RawRefVisitor = std::function<void(ObjectRef &)>;
using RootVisitor = RawRefVisitor;
using StackPtrVisitor = RawRefVisitor;

}  // namespace ark::common_vm

namespace ark::mem {
using ::ark::common_vm::BaseObject;
}  // namespace ark::mem

#endif  // COMMON_RUNTIME_COMMON_INTERFACES_OBJECTS_BASE_OBJECT_H
