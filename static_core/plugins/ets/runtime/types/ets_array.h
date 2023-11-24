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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_ARRAY_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_ARRAY_H_

#include "libpandabase/macros.h"
#include "libpandabase/mem/space.h"
#include "runtime/include/coretypes/array.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_class_root.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace panda::ets {

// Private inheritance, because need to disallow implicit conversion to core type
class EtsArray : private coretypes::Array {
public:
    EtsArray() = delete;
    ~EtsArray() = delete;

    size_t GetLength()
    {
        return GetCoreType()->GetLength();
    }

    size_t GetElementSize()
    {
        return GetCoreType()->ClassAddr<Class>()->GetComponentSize();
    }

    size_t ObjectSize()
    {
        return GetCoreType()->ObjectSize(GetElementSize());
    }

    template <class T>
    T *GetData()
    {
        return reinterpret_cast<T *>(GetCoreType()->GetData());
    }

    EtsClass *GetClass()
    {
        return EtsClass::FromRuntimeClass(GetCoreType()->ClassAddr<Class>());
    }

    bool IsPrimitive()
    {
        auto component_type = GetCoreType()->ClassAddr<Class>()->GetComponentType();
        ASSERT(component_type != nullptr);
        return component_type->IsPrimitive();
    }

    static constexpr uint32_t GetDataOffset()
    {
        return coretypes::Array::GetDataOffset();
    }

    EtsObject *AsObject()
    {
        return reinterpret_cast<EtsObject *>(this);
    }

    const EtsObject *AsObject() const
    {
        return reinterpret_cast<const EtsObject *>(this);
    }

    coretypes::Array *GetCoreType()
    {
        return reinterpret_cast<coretypes::Array *>(this);
    }

    template <class T>
    static T *Create(EtsClass *array_class, uint32_t length, SpaceType space_type = SpaceType::SPACE_TYPE_OBJECT)
    {
        return reinterpret_cast<T *>(coretypes::Array::Create(array_class->GetRuntimeClass(), length, space_type));
    }

    template <class T>
    static T *CreateForPrimitive(EtsClassRoot root, uint32_t length,
                                 SpaceType space_type = SpaceType::SPACE_TYPE_OBJECT)
    {
        EtsClass *array_class = PandaEtsVM::GetCurrent()->GetClassLinker()->GetClassRoot(root);
        return Create<T>(array_class, length, space_type);
    }

    NO_COPY_SEMANTIC(EtsArray);
    NO_MOVE_SEMANTIC(EtsArray);

protected:
    // Use type alias to allow using into derived classes
    using ObjectHeader = ::panda::ObjectHeader;

    template <class T>
    void SetImpl(uint32_t idx, T elem)
    {
        GetCoreType()->Set(idx, elem);
    }

    template <class T>
    T GetImpl(uint32_t idx)
    {
        return GetCoreType()->Get<T>(idx);
    }
};

class EtsObjectArray : public EtsArray {
public:
    static EtsObjectArray *Create(EtsClass *object_class, uint32_t length,
                                  panda::SpaceType space_type = panda::SpaceType::SPACE_TYPE_OBJECT)
    {
        ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
        // Generate Array class name  "[L<object_class>;"
        EtsClassLinker *class_linker = PandaEtsVM::GetCurrent()->GetClassLinker();
        PandaString array_class_name = PandaString("[") + object_class->GetDescriptor();
        EtsClass *array_class = class_linker->GetClass(array_class_name.c_str(), true, object_class->GetClassLoader());
        if (array_class == nullptr) {
            return nullptr;
        }
        return EtsArray::Create<EtsObjectArray>(array_class, length, space_type);
    }

    void Set(uint32_t index, EtsObject *element)
    {
        if (element == nullptr) {
            SetImpl<ObjectHeader *>(index, nullptr);
        } else {
            SetImpl(index, element->GetCoreType());
        }
    }

    EtsObject *Get(uint32_t index)
    {
        return reinterpret_cast<EtsObject *>(
            GetImpl<std::invoke_result_t<decltype(&EtsObject::GetCoreType), EtsObject>>(index));
    }

    static EtsObjectArray *FromCoreType(ObjectHeader *object_header)
    {
        return reinterpret_cast<EtsObjectArray *>(object_header);
    }

    EtsObjectArray() = delete;
    ~EtsObjectArray() = delete;

private:
    NO_COPY_SEMANTIC(EtsObjectArray);
    NO_MOVE_SEMANTIC(EtsObjectArray);
};

template <class ClassType, EtsClassRoot ETS_CLASS_ROOT>
class EtsPrimitiveArray : public EtsArray {
public:
    using ValueType = ClassType;

    static EtsPrimitiveArray *Create(uint32_t length, SpaceType space_type = SpaceType::SPACE_TYPE_OBJECT)
    {
        ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
        // NOLINTNEXTLINE(readability-magic-numbers)
        return EtsArray::CreateForPrimitive<EtsPrimitiveArray>(ETS_CLASS_ROOT, length, space_type);
    }
    void Set(uint32_t index, ClassType element)
    {
        SetImpl(index, element);
    }
    ClassType Get(uint32_t index)
    {
        return GetImpl<ClassType>(index);
    }

    EtsPrimitiveArray() = delete;
    ~EtsPrimitiveArray() = delete;

private:
    NO_COPY_SEMANTIC(EtsPrimitiveArray);
    NO_MOVE_SEMANTIC(EtsPrimitiveArray);
};

using EtsBooleanArray = EtsPrimitiveArray<EtsBoolean, EtsClassRoot::BOOLEAN_ARRAY>;
using EtsByteArray = EtsPrimitiveArray<EtsByte, EtsClassRoot::BYTE_ARRAY>;
using EtsCharArray = EtsPrimitiveArray<EtsChar, EtsClassRoot::CHAR_ARRAY>;
using EtsShortArray = EtsPrimitiveArray<EtsShort, EtsClassRoot::SHORT_ARRAY>;
using EtsIntArray = EtsPrimitiveArray<EtsInt, EtsClassRoot::INT_ARRAY>;
using EtsLongArray = EtsPrimitiveArray<EtsLong, EtsClassRoot::LONG_ARRAY>;
using EtsFloatArray = EtsPrimitiveArray<EtsFloat, EtsClassRoot::FLOAT_ARRAY>;
using EtsDoubleArray = EtsPrimitiveArray<EtsDouble, EtsClassRoot::DOUBLE_ARRAY>;

}  // namespace panda::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_ARRAY_H_
