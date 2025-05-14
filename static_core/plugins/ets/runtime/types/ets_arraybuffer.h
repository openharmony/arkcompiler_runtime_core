/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ARRAYBUFFER_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ARRAYBUFFER_H

#include "include/mem/allocator.h"
#include "include/object_accessor.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "runtime/include/thread_scopes.h"

#include <cstdint>

namespace ark::ets {

namespace test {
class EtsArrayBufferTest;
class EtsEscompatArrayBufferMembers;
}  // namespace test

class EtsEscompatArrayBuffer : public EtsObject {
public:
    EtsEscompatArrayBuffer() = delete;
    ~EtsEscompatArrayBuffer() = delete;

    NO_COPY_SEMANTIC(EtsEscompatArrayBuffer);
    NO_MOVE_SEMANTIC(EtsEscompatArrayBuffer);

    static EtsEscompatArrayBuffer *FromEtsObject(EtsObject *arrayBuffer)
    {
        return reinterpret_cast<EtsEscompatArrayBuffer *>(arrayBuffer);
    }

    /**
     * Creates a byte array in non-movable space.
     * @param length of created array.
     * NOTE: non-movable creation ensures that native code can obtain raw pointer to buffer.
     */
    ALWAYS_INLINE static ObjectHeader *AllocateNonMovableArray(EtsInt length)
    {
        return EtsByteArray::Create(length, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT)->GetCoreType();
    }

    ALWAYS_INLINE static EtsLong GetAddress(EtsByteArray *array)
    {
        return reinterpret_cast<EtsLong>(array->GetData<void>());
    }

    /// Creates ArrayBuffer with managed buffer.
    static EtsEscompatArrayBuffer *Create(EtsCoroutine *coro, size_t length, void **resultData)
    {
        ASSERT_MANAGED_CODE();
        ASSERT(!coro->HasPendingException());

        [[maybe_unused]] EtsHandleScope scope(coro);
        auto *cls = PlatformTypes(coro)->escompatArrayBuffer;
        EtsHandle<EtsEscompatArrayBuffer> handle(coro, EtsEscompatArrayBuffer::FromEtsObject(EtsObject::Create(cls)));
        if (UNLIKELY(handle.GetPtr() == nullptr)) {
            ASSERT(coro->HasPendingException());
            return nullptr;
        }

        handle->InitializeByDefault(coro, length);
        *resultData = handle->GetData();
        return handle.GetPtr();
    }

    /// Creates ArrayBuffer with user-provided buffer and finalization function.
    static EtsEscompatArrayBuffer *Create(EtsCoroutine *coro, void *externalData, size_t length,
                                          EtsFinalize finalizerFunction, void *finalizerHint)
    {
        ASSERT_MANAGED_CODE();
        ASSERT(!coro->HasPendingException());

        [[maybe_unused]] EtsHandleScope scope(coro);
        auto *cls = PlatformTypes(coro)->escompatArrayBuffer;
        EtsHandle<EtsEscompatArrayBuffer> handle(coro, EtsEscompatArrayBuffer::FromEtsObject(EtsObject::Create(cls)));
        if (UNLIKELY(handle.GetPtr() == nullptr)) {
            ASSERT(coro->HasPendingException());
            return nullptr;
        }

        handle->InitBufferByExternalData(coro, handle, externalData, finalizerFunction, finalizerHint, length);
        return handle.GetPtr();
    }

    EtsObject *AsObject()
    {
        return this;
    }

    EtsInt GetByteLength() const
    {
        return byteLength_;
    }

    /// @brief Returns non-null data for a non-detached buffer
    void *GetData()
    {
        ASSERT(!WasDetached());
        return reinterpret_cast<void *>(nativeData_);
    }

    void Detach()
    {
        ASSERT(IsDetachable());
        byteLength_ = 0;
        // Do not free memory, as the address was already passed into finalizer.
        // Memory will be freed after GC execution with object destruction
        nativeData_ = 0;
        ASSERT(WasDetached());
    }

    /// NOTE: behavior of this method must repeat implementation of `detached` property in ArkTS `ArrayBuffer`
    bool WasDetached() const
    {
        return nativeData_ == 0;
    }

    bool IsExternal() const
    {
        return managedData_ == nullptr;
    }

    bool IsDetachable() const
    {
        return !WasDetached() && IsExternal();
    }

    EtsByte At(EtsInt pos)
    {
        if (!DoBoundaryCheck(pos)) {
            return 0;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<int8_t *>(GetData())[pos];
    }

    void Set(EtsInt pos, EtsByte val)
    {
        if (!DoBoundaryCheck(pos)) {
            return;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        reinterpret_cast<int8_t *>(GetData())[pos] = val;
    }

    void SetValues(EtsEscompatArrayBuffer *other, EtsInt begin)
    {
        ASSERT(!WasDetached());
        ASSERT(other != nullptr);
        ASSERT(!other->WasDetached());
        ASSERT(begin >= 0);
        auto thisByteLength = GetByteLength();
        ASSERT(begin + thisByteLength <= other->GetByteLength());

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *srcData = reinterpret_cast<int8_t *>(other->GetData()) + begin;
        auto *dstData = GetData();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        [[maybe_unused]] errno_t res = memcpy_s(dstData, thisByteLength, srcData, thisByteLength);
        ASSERT(res == 0);
    }

    static constexpr size_t GetManagedDataOffset()
    {
        return MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_);
    }

private:
    struct FinalizationInfo final {
        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        void *data;
        EtsFinalize function;
        void *hint;
        // NOLINTEND(misc-non-private-member-variables-in-classes)

        explicit FinalizationInfo(void *d, EtsFinalize f, void *h) : data(d), function(f), hint(h) {}
    };

private:
    /**
     * Creates `FinalizableWeakRef` for created ArrayBuffer.
     * @param coro in which the ArrayBuffer is created.
     * @param arrayBufferHandle handle for the created object.
     * @param finalizerFunction user-provided function to call upon finalization.
     * @param finalizerHint an additional argument for the finalizer.
     * NOTE: method must be called under `EtsHandleScope`.
     */
    static void RegisterFinalizationInfo(EtsCoroutine *coro, const EtsHandle<EtsEscompatArrayBuffer> &arrayBufferHandle,
                                         EtsFinalize finalizerFunction, void *finalizerHint)
    {
        if (finalizerFunction == nullptr) {
            return;
        }

        auto *allocator = static_cast<mem::Allocator *>(Runtime::GetCurrent()->GetInternalAllocator());
        auto *pandaVm = coro->GetPandaVM();

        auto *finalizationInfo =
            allocator->New<FinalizationInfo>(arrayBufferHandle.GetPtr()->GetData(), finalizerFunction, finalizerHint);
        EtsHandle<EtsObject> handle(arrayBufferHandle);
        pandaVm->RegisterFinalizerForObject(coro, handle, DoFinalization, finalizationInfo);

        ScopedNativeCodeThread s(coro);
        pandaVm->GetGC()->RegisterNativeAllocation(sizeof(FinalizationInfo));
    }

    static void DoFinalization(void *arg)
    {
        ASSERT(arg != nullptr);
        auto *info = reinterpret_cast<FinalizationInfo *>(arg);

        ASSERT(info->function != nullptr);
        auto *allocator = static_cast<mem::Allocator *>(Runtime::GetCurrent()->GetInternalAllocator());

        info->function(info->data, info->hint);

        auto *vm = Runtime::GetCurrent()->GetPandaVM();
        vm->GetGC()->RegisterNativeFree(sizeof(FinalizationInfo));

        allocator->Free(info);
    }

    /**
     * Initializes ArrayBuffer.
     * NOTE: behavior of this method must repeat initialization from managed constructor.
     */
    void InitializeByDefault(EtsCoroutine *coro, size_t length)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsEscompatArrayBuffer, managedData_),
                                  AllocateNonMovableArray(length));
        byteLength_ = length;
        nativeData_ = GetAddress(managedData_);
        ASSERT(nativeData_ != 0);
        isResizable_ = ToEtsBoolean(false);
    }

    /// Initializes ArrayBuffer with externally provided buffer.
    void InitBufferByExternalData(EtsCoroutine *coro, const EtsHandle<EtsEscompatArrayBuffer> &arrayBufferHandle,
                                  void *data, EtsFinalize finalizerFunction, void *finalizerHint, size_t length)
    {
        managedData_ = nullptr;
        byteLength_ = length;
        nativeData_ = reinterpret_cast<EtsLong>(data);
        ASSERT(nativeData_ != 0);
        isResizable_ = ToEtsBoolean(false);

        RegisterFinalizationInfo(coro, arrayBufferHandle, finalizerFunction, finalizerHint);
    }

    /**
     * @brief Checks position is inside array, throws ets exception if not.
     * NOTE: behavior of this method must repeat initialization from managed `doBoundaryCheck`.
     */
    bool DoBoundaryCheck(EtsInt pos)
    {
        if (pos < 0 || pos >= byteLength_) {
            PandaString message = "ArrayBuffer position ";
            message.append(std::to_string(pos)).append(" is out of bounds");
            ThrowEtsException(EtsCoroutine::GetCurrent(),
                              panda_file_items::class_descriptors::INDEX_OUT_OF_BOUNDS_ERROR, message.c_str());
            return false;
        }
        return true;
    }

private:
    // Managed array used in this `ArrayBuffer`, null if buffer is external
    ObjectPointer<EtsByteArray> managedData_;
    EtsInt byteLength_;
    // Contains pointer to either managed non-movable data or external data.
    // Null if `ArrayBuffer` was detached, non-null otherwise
    EtsLong nativeData_;
    EtsBoolean isResizable_;

    friend class test::EtsArrayBufferTest;
    friend class test::EtsEscompatArrayBufferMembers;
};

class EtsEscompatSharedMemory : public EtsObject {
public:
    EtsEscompatSharedMemory() = delete;
    ~EtsEscompatSharedMemory() = delete;

    NO_COPY_SEMANTIC(EtsEscompatSharedMemory);
    NO_MOVE_SEMANTIC(EtsEscompatSharedMemory);

    static constexpr size_t GetDataOffset()
    {
        return MEMBER_OFFSET(EtsEscompatSharedMemory, data_);
    }

    ObjectPointer<EtsByteArray> GetData()
    {
        return data_;
    }

    EtsLong GetWaiterPtr()
    {
        return waiterPtr_;
    }

private:
    ObjectPointer<EtsByteArray> data_;
    EtsLong waiterPtr_;
};

class EtsEscompatSharedArrayBuffer : public EtsObject {
public:
    EtsEscompatSharedArrayBuffer() = delete;
    ~EtsEscompatSharedArrayBuffer() = delete;

    NO_COPY_SEMANTIC(EtsEscompatSharedArrayBuffer);
    NO_MOVE_SEMANTIC(EtsEscompatSharedArrayBuffer);

    static constexpr size_t GetSharedMemoryOffset()
    {
        return MEMBER_OFFSET(EtsEscompatSharedArrayBuffer, sharedMemory_);
    }

    ObjectPointer<EtsEscompatSharedMemory> GetSharedMemory()
    {
        return sharedMemory_;
    }

private:
    ObjectPointer<EtsEscompatSharedMemory> sharedMemory_;
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ARRAYBUFFER_H
