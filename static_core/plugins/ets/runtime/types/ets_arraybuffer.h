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

    /// Creates ArrayBuffer with managed buffer.
    static EtsEscompatArrayBuffer *Create(EtsCoroutine *coro, size_t length, void **resultData)
    {
        ASSERT_MANAGED_CODE();
        ASSERT(!coro->HasPendingException());

        [[maybe_unused]] EtsHandleScope scope(coro);
        auto *cls = coro->GetPandaVM()->GetClassLinker()->GetArrayBufferClass();
        EtsHandle<EtsEscompatArrayBuffer> handle(coro, EtsEscompatArrayBuffer::FromEtsObject(EtsObject::Create(cls)));

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
        auto *cls = coro->GetPandaVM()->GetClassLinker()->GetArrayBufferClass();
        EtsHandle<EtsEscompatArrayBuffer> handle(coro, EtsEscompatArrayBuffer::FromEtsObject(EtsObject::Create(cls)));

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

    void *GetData()
    {
        if (IsExternal()) {
            return reinterpret_cast<void *>(nativeData_);
        }
        return managedData_->GetData<void>();
    }

    void Detach()
    {
        ASSERT(IsDetachable());
        byteLength_ = 0;
        // Do not free memory, as the address was already passed into finalizer.
        // Memory will be freed after GC execution with object destruction
        nativeData_ = 0;
        wasDetached_ = ToEtsBoolean(true);
    }

    bool WasDetached()
    {
        return wasDetached_ == ToEtsBoolean(true);
    }

    bool IsExternal() const
    {
        return isExternal_ == ToEtsBoolean(true);
    }

    bool IsDetachable()
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
        ASSERT(!IsExternal());
        ASSERT(begin >= 0);
        auto byteLength = GetByteLength();
        ASSERT(begin + byteLength <= other->GetByteLength());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto *srcData = reinterpret_cast<int8_t *>(other->GetData()) + begin;
        auto *dstData = managedData_->GetData<int8_t>();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        errno_t res = memcpy_s(dstData, byteLength, srcData, byteLength);
        if (res != 0) {
            UNREACHABLE();
        }
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
        ASSERT(arg);
        auto *info = reinterpret_cast<FinalizationInfo *>(arg);
        ASSERT(info->function != nullptr);
        auto *allocator = static_cast<mem::Allocator *>(Runtime::GetCurrent()->GetInternalAllocator());

        info->function(EtsCoroutine::GetCurrent()->GetEtsNapiEnv(), info->data, info->hint);

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
        nativeData_ = 0;
        isExternal_ = ToEtsBoolean(false);
        wasDetached_ = ToEtsBoolean(false);
    }

    /// Initializes ArrayBuffer with externally provided buffer.
    void InitBufferByExternalData(EtsCoroutine *coro, const EtsHandle<EtsEscompatArrayBuffer> &arrayBufferHandle,
                                  void *data, EtsFinalize finalizerFunction, void *finalizerHint, size_t length)
    {
        managedData_ = nullptr;
        byteLength_ = length;
        nativeData_ = reinterpret_cast<EtsLong>(data);
        isExternal_ = ToEtsBoolean(true);
        wasDetached_ = ToEtsBoolean(false);

        RegisterFinalizationInfo(coro, arrayBufferHandle, finalizerFunction, finalizerHint);
    }

    /// Checks position is inside array, throws ets exception if not.
    bool DoBoundaryCheck(EtsInt pos)
    {
        if (pos < 0 || pos >= byteLength_) {
            ThrowEtsException(EtsCoroutine::GetCurrent(),
                              panda_file_items::class_descriptors::INDEX_OUT_OF_BOUNDS_ERROR,
                              "ArrayBuffer position is out of bounds");
            return false;
        }
        return true;
    }

private:
    ObjectPointer<EtsByteArray> managedData_;
    EtsInt byteLength_;
    EtsLong nativeData_;
    EtsBoolean isExternal_;
    EtsBoolean wasDetached_;

    friend class test::EtsArrayBufferTest;
    friend class test::EtsEscompatArrayBufferMembers;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ARRAYBUFFER_H
