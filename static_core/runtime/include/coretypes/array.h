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
#ifndef PANDA_RUNTIME_CORETYPES_ARRAY_H_
#define PANDA_RUNTIME_CORETYPES_ARRAY_H_

#include <securec.h>
#include <cstddef>
#include <cstdint>

#include "libpandabase/macros.h"
#include "libpandabase/mem/mem.h"
#include "libpandabase/mem/space.h"
#include "libpandabase/utils/span.h"
#include "libpandafile/bytecode_instruction-inl.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/language_context.h"
#include "runtime/include/object_header.h"
#include "runtime/mem/heap_manager.h"
#include "runtime/include/coretypes/tagged_value.h"

namespace panda {
class ManagedThread;
class PandaVM;
}  // namespace panda

namespace panda::interpreter {
template <BytecodeInstruction::Format FORMAT, bool IS_DYNAMIC = false>
class DimIterator;
}  // namespace panda::interpreter

namespace panda::coretypes {
class DynClass;
using ArraySizeT = panda::ArraySizeT;
using ArraySsizeT = panda::ArraySsizeT;

class Array : public ObjectHeader {
public:
    static constexpr ArraySizeT MAX_ARRAY_INDEX = std::numeric_limits<ArraySizeT>::max();

    static Array *Cast(ObjectHeader *object)
    {
        // NOTE(linxiang) to do assert
        return reinterpret_cast<Array *>(object);
    }

    PANDA_PUBLIC_API static Array *Create(panda::Class *array_class, const uint8_t *data, ArraySizeT length,
                                          panda::SpaceType space_type = panda::SpaceType::SPACE_TYPE_OBJECT);

    PANDA_PUBLIC_API static Array *Create(panda::Class *array_class, ArraySizeT length,
                                          panda::SpaceType space_type = panda::SpaceType::SPACE_TYPE_OBJECT);

    PANDA_PUBLIC_API static Array *Create(DynClass *dynarrayclass, ArraySizeT length,
                                          panda::SpaceType space_type = panda::SpaceType::SPACE_TYPE_OBJECT);

    static Array *CreateTagged(const PandaVM *vm, panda::BaseClass *array_class, ArraySizeT length,
                               panda::SpaceType space_type = panda::SpaceType::SPACE_TYPE_OBJECT,
                               TaggedValue init_value = TaggedValue::Undefined());

    static size_t ComputeSize(size_t elem_size, ArraySizeT length)
    {
        ASSERT(elem_size != 0);
        size_t size = sizeof(Array) + elem_size * length;
#ifdef PANDA_TARGET_32
        // NOLINTNEXTLINE(clang-analyzer-core.DivideZero)
        size_t size_limit = (std::numeric_limits<size_t>::max() - sizeof(Array)) / elem_size;
        if (UNLIKELY(size_limit < static_cast<size_t>(length))) {
            return 0;
        }
#endif
        return size;
    }

    ArraySizeT GetLength() const
    {
        // Atomic with relaxed order reason: data race with length_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        return length_.load(std::memory_order_relaxed);
    }

    uint32_t *GetData()
    {
        return data_;
    }

    const uint32_t *GetData() const
    {
        return data_;
    }

    template <class T, bool IS_VOLATILE = false>
    T GetPrimitive(size_t offset) const;

    template <class T, bool IS_VOLATILE = false>
    void SetPrimitive(size_t offset, T value);

    template <bool IS_VOLATILE = false, bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetObject(int offset) const;

    template <bool IS_VOLATILE = false, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetObject(size_t offset, ObjectHeader *value);

    template <class T>
    T GetPrimitive(size_t offset, std::memory_order memory_order) const;

    template <class T>
    void SetPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetObject(size_t offset, std::memory_order memory_order) const;

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetObject(size_t offset, ObjectHeader *value, std::memory_order memory_order);

    template <typename T>
    bool CompareAndSetPrimitive(size_t offset, T old_value, T new_value, std::memory_order memory_order, bool strong);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    bool CompareAndSetObject(size_t offset, ObjectHeader *old_value, ObjectHeader *new_value,
                             std::memory_order memory_order, bool strong);

    template <typename T>
    T CompareAndExchangePrimitive(size_t offset, T old_value, T new_value, std::memory_order memory_order, bool strong);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *CompareAndExchangeObject(size_t offset, ObjectHeader *old_value, ObjectHeader *new_value,
                                           std::memory_order memory_order, bool strong);

    template <typename T>
    T GetAndSetPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetAndSetObject(size_t offset, ObjectHeader *value, std::memory_order memory_order);

    template <typename T>
    T GetAndAddPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <typename T>
    T GetAndBitwiseOrPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <typename T>
    T GetAndBitwiseAndPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <typename T>
    T GetAndBitwiseXorPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <class T, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void Set(ArraySizeT idx, T elem);

    template <class T, bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    T Get(ArraySizeT idx) const;

    template <class T, bool IS_DYN>
    static constexpr size_t GetElementSize();

    template <class T>
    T GetBase();

    // Pass thread parameter to speed up interpreter
    template <class T, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void Set([[maybe_unused]] const ManagedThread *thread, ArraySizeT idx, T elem);

    template <class T, bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    T Get([[maybe_unused]] const ManagedThread *thread, ArraySizeT idx) const;

    size_t ObjectSize(uint32_t component_size) const
    {
        return ComputeSize(component_size, length_);
    }

    static constexpr uint32_t GetLengthOffset()
    {
        return MEMBER_OFFSET(Array, length_);
    }

    static constexpr uint32_t GetDataOffset()
    {
        return MEMBER_OFFSET(Array, data_);
    }

    template <bool IS_DYN>
    ArraySizeT GetElementOffset(ArraySizeT idx) const
    {
        size_t elem_size;
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (IS_DYN) {
            elem_size = TaggedValue::TaggedTypeSize();
        } else {  // NOLINT(readability-misleading-indentation)
            elem_size = ClassAddr<panda::Class>()->GetComponentSize();
        }
        return GetDataOffset() + idx * elem_size;
    }

    template <class DimIterator>
    static Array *CreateMultiDimensionalArray(ManagedThread *thread, panda::Class *klass, uint32_t nargs,
                                              const DimIterator &iter, size_t dim_idx = 0);

private:
    void SetLength(ArraySizeT length)
    {
        // Atomic with relaxed order reason: data race with length_ with no synchronization or ordering constraints
        // imposed on other reads or writes
        length_.store(length, std::memory_order_relaxed);
    }

    std::atomic<ArraySizeT> length_;
    // Align by 64bits, because dynamic language data is always 64bits
    __extension__ alignas(sizeof(uint64_t)) uint32_t data_[0];  // NOLINT(modernize-avoid-c-arrays)
};

static_assert(Array::GetLengthOffset() == sizeof(ObjectHeader));
static_assert(Array::GetDataOffset() == AlignUp(Array::GetLengthOffset() + sizeof(ArraySizeT), sizeof(uint64_t)));
static_assert(Array::GetDataOffset() % sizeof(uint64_t) == 0);

#ifdef PANDA_TARGET_64
constexpr uint32_t ARRAY_LENGTH_OFFSET = 8U;
static_assert(ARRAY_LENGTH_OFFSET == panda::coretypes::Array::GetLengthOffset());
constexpr uint32_t ARRAY_DATA_OFFSET = 16U;
static_assert(ARRAY_DATA_OFFSET == panda::coretypes::Array::GetDataOffset());
#endif
}  // namespace panda::coretypes

#endif  // PANDA_RUNTIME_CORETYPES_ARRAY_H_
