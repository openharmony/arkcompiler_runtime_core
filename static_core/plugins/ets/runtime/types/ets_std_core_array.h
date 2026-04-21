/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_STD_CORE_ARRAY_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_STD_CORE_ARRAY_H

#include "libarkbase/mem/object_pointer.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::ets {

namespace test {
class EtsStdCoreArrayTest;
}  // namespace test

// Mirror class for Array<T> from ETS stdlib
class EtsStdCoreArray : public EtsObject {
public:
    EtsStdCoreArray() = delete;
    ~EtsStdCoreArray() = delete;

    NO_COPY_SEMANTIC(EtsStdCoreArray);
    NO_MOVE_SEMANTIC(EtsStdCoreArray);

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static EtsStdCoreArray *FromEtsObject(EtsObject *etsObj)
    {
        return reinterpret_cast<EtsStdCoreArray *>(etsObj);
    }

    static constexpr size_t GetBufferOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreArray, buffer_);
    }

    static constexpr size_t GetActualLengthOffset()
    {
        return MEMBER_OFFSET(EtsStdCoreArray, actualLength_);
    }

    /// @brief Creates instance of `std.core.Array` with given length
    static EtsStdCoreArray *Create(EtsExecutionContext *executionCtx, size_t length);

    /// @brief Checks whether the object's real type is `std.core.Array`
    static bool IsExactlyStdCoreArray(const EtsObject *obj, EtsExecutionContext *executionCtx)
    {
        return obj->GetClass() ==
               PlatformTypes(executionCtx)->coreArray;  // NOLINT (readability-implicit-bool-conversion)
    }

    /// @brief Checks whether the object's real type is `std.core.Array`
    bool IsExactlyStdCoreArray(EtsExecutionContext *executionCtx) const
    {
        return EtsStdCoreArray::IsExactlyStdCoreArray(this, executionCtx);
    }

    /// @brief Implementation of `std.core.Array` method `$_get`
    EtsObject *StdCoreArrayGet(EtsInt index);

    /// @brief Implementation of `std.core.Array` method `$_set`
    void StdCoreArraySet(EtsInt index, EtsObject *value);

    /// @brief Implementation of `std.core.Array` method `$_set_unsafe`
    EtsObject *StdCoreArrayGetUnsafe(EtsInt index);

    /// @brief Implementation of `std.core.Array` method `$_get_unsafe`
    void StdCoreArraySetUnsafe(EtsInt index, EtsObject *value);

    /**
     * @brief Implementation of `std.core.Array` method `pop`
     * Note that this method is valid only when `values` array is exactly `std.core.Array`.
     * Implementation of this method must be aligned with managed implementation of `pop` method
     */
    EtsObject *StdCoreArrayPop();

    /// @brief Returns underlying buffer, object must be exactly `std.core.Array`
    EtsObjectArray *GetDataFromStdCoreArray()
    {
        ASSERT(IsExactlyStdCoreArray(EtsExecutionContext::GetCurrent()));
        return GetDataFromStdCoreArrayImpl();
    }

    /// @brief Returns `actualLength` of array, object must be exactly `std.core.Array`
    EtsInt GetActualLengthFromStdCoreArray()
    {
        ASSERT(IsExactlyStdCoreArray(EtsExecutionContext::GetCurrent()));
        return GetActualLengthFromStdCoreArrayImpl();
    }

    /**
     * @brief Returns length of array according to possible inheritance of underlying array
     * Note that this method is potentially throwing
     */
    bool GetLength(EtsExecutionContext *executionCtx, EtsInt *result);

    /**
     * @brief Sets value at index according to possible inheritance of underlying array
     * Note that this method is potentially throwing
     */
    bool SetRef(EtsExecutionContext *executionCtx, size_t index, EtsObject *ref);

    /**
     * @brief Gets value at index according to possible inheritance of underlying array
     * Note that this method is potentially throwing
     */
    std::optional<EtsObject *> GetRef(EtsExecutionContext *executionCtx, size_t index);

    /**
     * @brief Pop from array according to possible inheritance of underlying array
     * Note that this method is potentially throwing
     */
    EtsObject *Pop(EtsExecutionContext *executionCtx);

private:
    EtsObjectArray *GetDataFromStdCoreArrayImpl()
    {
        auto *buff = ObjectAccessor::GetObject(this, GetBufferOffset());
        return EtsObjectArray::FromCoreType(buff);
    }

    EtsInt GetActualLengthFromStdCoreArrayImpl()
    {
        return ObjectAccessor::GetFieldPrimitive<EtsInt>(this, GetActualLengthOffset(), std::memory_order_relaxed);
    }

    void SetActualLengthFromStdCoreArrayImpl(EtsInt value)
    {
        ObjectAccessor::SetFieldPrimitive<EtsInt>(this, GetActualLengthOffset(), value, std::memory_order_relaxed);
    }

private:
    ObjectPointer<EtsObjectArray> buffer_;
    EtsInt actualLength_;

    friend class test::EtsStdCoreArrayTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_STD_CORE_ARRAY_H
