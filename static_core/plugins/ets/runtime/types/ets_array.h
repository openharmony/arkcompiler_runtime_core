/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_class_root.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets {

// Private inheritance, because need to disallow implicit conversion to core type
class EtsArray : private coretypes::Array {
public:
    EtsArray() = delete;
    ~EtsArray() = delete;

    PANDA_PUBLIC_API size_t GetLength()
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
        auto componentType = GetCoreType()->ClassAddr<Class>()->GetComponentType();
        ASSERT(componentType != nullptr);
        return componentType->IsPrimitive();
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

    NO_COPY_SEMANTIC(EtsArray);
    NO_MOVE_SEMANTIC(EtsArray);

protected:
    // Use type alias to allow using into derived classes
    using ObjectHeader = ::ark::ObjectHeader;

    template <class T>
    static T *Create(EtsClass *arrayClass, uint32_t length, SpaceType spaceType = SpaceType::SPACE_TYPE_OBJECT,
                     bool pinned = false)
    {
        return reinterpret_cast<T *>(
            coretypes::Array::Create(arrayClass->GetRuntimeClass(), length, spaceType, pinned));
    }

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

template <typename Component>
class EtsTypedObjectArray : public EtsArray {
public:
    using ValueType = Component *;

    static EtsTypedObjectArray *Create(EtsClass *objectClass, uint32_t length,
                                       ark::SpaceType spaceType = ark::SpaceType::SPACE_TYPE_OBJECT)
    {
        ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
        // Generate Array class name  "[L<object_class>;"
        EtsClassLinker *classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
        PandaString arrayClassName = PandaString("[") + objectClass->GetDescriptor();
        EtsClass *arrayClass = classLinker->GetClass(arrayClassName.c_str(), true, objectClass->GetLoadContext());
        if (arrayClass == nullptr) {
            return nullptr;
        }
        return EtsArray::Create<EtsTypedObjectArray>(arrayClass, length, spaceType);
    }

    void Set(uint32_t index, Component *element)
    {
        if (element == nullptr) {
            SetImpl<ObjectHeader *>(index, nullptr);
        } else {
            SetImpl(index, element->GetCoreType());
        }
    }

    PANDA_PUBLIC_API Component *Get(uint32_t index)
    {
        return reinterpret_cast<Component *>(
            GetImpl<std::invoke_result_t<decltype(&Component::GetCoreType), Component>>(index));
    }

    void Set(uint32_t index, Component *element, std::memory_order memoryOrder)
    {
        auto offset = index * sizeof(ObjectPointerType);
        GetCoreType()->SetObject(offset, element == nullptr ? nullptr : element->GetCoreType(), memoryOrder);
    }

    Component *Get(uint32_t index, std::memory_order memoryOrder)
    {
        auto offset = index * sizeof(ObjectPointerType);
        return Component::FromCoreType(GetCoreType()->GetObject(offset, memoryOrder));
    }

    static EtsTypedObjectArray *FromCoreType(ObjectHeader *objectHeader)
    {
        return reinterpret_cast<EtsTypedObjectArray *>(objectHeader);
    }

    void CopyDataTo(EtsTypedObjectArray *dst)
    {
        ASSERT(dst != nullptr);
        ASSERT(GetLength() <= dst->GetLength());

        if (std::size_t count = GetLength() * OBJECT_POINTER_SIZE) {
            Span<uint8_t> srcSpan(GetData<uint8_t>(), count);
            Span<uint8_t> dstSpan(dst->GetData<uint8_t>(), count);
            CopyData(srcSpan, dstSpan);
            // Call barriers.
            // PreBarrier isn't needed as links inside the source object arn't changed.
            auto *barrierSet = ManagedThread::GetCurrent()->GetBarrierSet();
            if (!mem::IsEmptyBarrier(barrierSet->GetPostType())) {
                barrierSet->PostBarrier(dst, 0, dst->ObjectSize());
            }
        }
    }

    EtsTypedObjectArray() = delete;
    ~EtsTypedObjectArray() = delete;

private:
    NO_COPY_SEMANTIC(EtsTypedObjectArray);
    NO_MOVE_SEMANTIC(EtsTypedObjectArray);

    using WordType = uintptr_t;
    using AtomicWord = std::atomic<WordType>;
    using AtomicRef = std::atomic<ark::ObjectPointerType>;

    static constexpr const std::size_t WORD_SIZE = sizeof(WordType);

    void CopyData(Span<uint8_t> &src, Span<uint8_t> &dst)
    {
        ASSERT((src.Size() % OBJECT_POINTER_SIZE) == 0);
        ASSERT(src.Size() <= dst.Size());

        // WORDs and REFERENCEs must be loaded/stored atomically
        constexpr const std::memory_order ORDER = std::memory_order_relaxed;
        // 1. copy by words if any
        std::size_t i = 0;
        std::size_t stop = (src.Size() / WORD_SIZE) * WORD_SIZE;
        for (; i < stop; i += WORD_SIZE) {
            // Atomic with parameterized order reason: memory order defined as constexpr
            reinterpret_cast<AtomicWord *>(&dst[i])->store(reinterpret_cast<AtomicWord *>(&src[i])->load(ORDER), ORDER);
        }
        // 2. copy by references if any
        stop = ((src.Size() - i) / OBJECT_POINTER_SIZE) * OBJECT_POINTER_SIZE;
        for (; i < stop; i += OBJECT_POINTER_SIZE) {
            // Atomic with parameterized order reason: memory order defined as constexpr
            reinterpret_cast<AtomicRef *>(&dst[i])->store(reinterpret_cast<AtomicRef *>(&src[i])->load(ORDER), ORDER);
        }
    }
};

using EtsObjectArray = EtsTypedObjectArray<EtsObject>;

template <class ClassType, EtsClassRoot ETS_CLASS_ROOT>
class EtsPrimitiveArray : public EtsArray {
public:
    using ValueType = ClassType;

    static EtsPrimitiveArray *Create(uint32_t length, SpaceType spaceType = SpaceType::SPACE_TYPE_OBJECT,
                                     bool pinned = false)
    {
        ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
        return EtsArray::Create<EtsPrimitiveArray>(GetComponentClass(), length, spaceType, pinned);
    }
    void Set(uint32_t index, ClassType element)
    {
        SetImpl(index, element);
    }
    ClassType Get(uint32_t index)
    {
        return GetImpl<ClassType>(index);
    }
    static EtsClass *GetComponentClass()
    {
        return PandaEtsVM::GetCurrent()->GetClassLinker()->GetClassRoot(ETS_CLASS_ROOT);
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
using EtsUintArray = EtsPrimitiveArray<EtsUint, EtsClassRoot::UINT_ARRAY>;
using EtsIntArray = EtsPrimitiveArray<EtsInt, EtsClassRoot::INT_ARRAY>;
using EtsUlongArray = EtsPrimitiveArray<EtsUlong, EtsClassRoot::ULONG_ARRAY>;
using EtsLongArray = EtsPrimitiveArray<EtsLong, EtsClassRoot::LONG_ARRAY>;
using EtsFloatArray = EtsPrimitiveArray<EtsFloat, EtsClassRoot::FLOAT_ARRAY>;
using EtsDoubleArray = EtsPrimitiveArray<EtsDouble, EtsClassRoot::DOUBLE_ARRAY>;

namespace test {
template <class ElementType>
class EtsArrayObjectMembers;
}  // namespace test

// Mirror class for Array<T> from ETS stdlib
template <class ElementType>
class EtsArrayObject : public EtsObject {
public:
    EtsArrayObject() = delete;
    ~EtsArrayObject() = delete;

    NO_COPY_SEMANTIC(EtsArrayObject);
    NO_MOVE_SEMANTIC(EtsArrayObject);

    static EtsArrayObject *FromEtsObject(EtsObject *etsObj)
    {
        return reinterpret_cast<EtsArrayObject *>(etsObj);
    }

    EtsTypedObjectArray<ElementType> *GetData()
    {
        return reinterpret_cast<EtsTypedObjectArray<ElementType> *>(GetFieldObject(GetBufferOffset()));
    }

    uint32_t GetActualLength()
    {
        return helpers::ToUnsigned(GetFieldPrimitive<EtsInt>(GetActualLengthOffset()));
    }

    static constexpr size_t GetBufferOffset()
    {
        return MEMBER_OFFSET(EtsArrayObject, buffer_);
    }

    static constexpr size_t GetActualLengthOffset()
    {
        return MEMBER_OFFSET(EtsArrayObject, actualLength_);
    }

private:
    ObjectPointer<EtsTypedObjectArray<ElementType>> buffer_;
    EtsInt actualLength_;

    friend class test::EtsArrayObjectMembers<ElementType>;
};

using EtsBoxedBooleanArray = EtsArrayObject<EtsBoxPrimitive<EtsBoolean>>;
using EtsBoxedByteArray = EtsArrayObject<EtsBoxPrimitive<EtsByte>>;
using EtsBoxedCharArray = EtsArrayObject<EtsBoxPrimitive<EtsChar>>;
using EtsBoxedShortArray = EtsArrayObject<EtsBoxPrimitive<EtsShort>>;
using EtsBoxedIntArray = EtsArrayObject<EtsBoxPrimitive<EtsInt>>;
using EtsBoxedLongArray = EtsArrayObject<EtsBoxPrimitive<EtsLong>>;
using EtsBoxedFloatArray = EtsArrayObject<EtsBoxPrimitive<EtsFloat>>;
using EtsBoxedDoubleArray = EtsArrayObject<EtsBoxPrimitive<EtsDouble>>;
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_ARRAY_H_
