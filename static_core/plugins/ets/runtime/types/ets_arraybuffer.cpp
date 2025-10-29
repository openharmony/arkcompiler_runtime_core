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

#include "include/mem/allocator.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_arraybuffer.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "runtime/arch/memory_helpers.h"

namespace ark::ets {

/*static*/
EtsStdCoreArrayBuffer *EtsStdCoreArrayBuffer::Create(EtsCoroutine *coro, size_t length)
{
    ASSERT_MANAGED_CODE();
    ASSERT(!coro->HasPendingException());

    [[maybe_unused]] EtsHandleScope scope(coro);
    auto *cls = PlatformTypes(coro)->coreArrayBuffer;
    EtsHandle<EtsStdCoreArrayBuffer> handle(coro, EtsStdCoreArrayBuffer::FromEtsObject(EtsObject::Create(cls)));
    if (UNLIKELY(handle.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    auto *buf = AllocateArray(length);
    if (UNLIKELY(buf == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    handle->Initialize(coro, length, buf);
    return handle.GetPtr();
}

/*static*/
EtsStdCoreArrayBuffer *EtsStdCoreArrayBuffer::CreateNonMovable(EtsCoroutine *coro, size_t length, void **resultData)
{
    ASSERT_MANAGED_CODE();
    ASSERT(!coro->HasPendingException());

    [[maybe_unused]] EtsHandleScope scope(coro);
    auto *cls = PlatformTypes(coro)->coreArrayBuffer;
    EtsHandle<EtsStdCoreArrayBuffer> handle(coro, EtsStdCoreArrayBuffer::FromEtsObject(EtsObject::Create(cls)));
    if (UNLIKELY(handle.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    auto *buf = AllocateNonMovableArray(length);
    if (UNLIKELY(buf == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    handle->Initialize(coro, length, buf);
    *resultData = handle->GetData();
    return handle.GetPtr();
}

/*static*/
bool EtsStdCoreArrayBuffer::IsNonMovableArray(EtsCoroutine *coro, EtsStdCoreArrayBuffer *self)
{
    ASSERT(self != nullptr);
    EtsHandle<EtsStdCoreArrayBuffer> handle(coro, self);
    if (LIKELY(IsNativeArray(handle.GetPtr()))) {
        return true;
    }
    auto objArray = handle->GetManagedDataImpl();
    return objArray != nullptr && coro->GetVM()->GetHeapManager()->IsNonMovable(objArray->AsObject()->GetCoreType());
}

/*static*/
bool EtsStdCoreArrayBuffer::IsNativeArray(EtsStdCoreArrayBuffer *self)
{
    ASSERT(self != nullptr);
    return self->GetNativeDataImpl() != nullptr;
}

/*static*/
void EtsStdCoreArrayBuffer::ReallocateNonMovableArray(EtsCoroutine *coro, EtsStdCoreArrayBuffer *self, EtsInt bytesLen)
{
    ASSERT(self != nullptr);
    EtsHandle<EtsStdCoreArrayBuffer> handle(coro, self);
    if (UNLIKELY(bytesLen <= 0 || handle.GetPtr() == nullptr)) {
        return;
    }
    ASSERT(!IsNativeArray(handle.GetPtr()));

    auto *nonMovArray = AllocateNonMovableArray(bytesLen);
    if (UNLIKELY(nonMovArray == nullptr)) {
        ASSERT(coro->HasPendingException());
        return;
    }
    auto srcLen = bytesLen > handle->GetByteLength() ? handle->GetByteLength() : bytesLen;

    auto inputManagedData = handle->GetManagedDataImpl();
    if (inputManagedData != nullptr) {
        // Pre read barrier is need, because an input managed buffer is movable.
        auto *readBarrierSet = ManagedThread::GetCurrent()->GetBarrierSet();
        auto field = ToVoidPtr(ToUintPtr(static_cast<void *>(inputManagedData)) + coretypes::Array::GetDataOffset());
        readBarrierSet->ReadBarrier(&field);
        auto *src = inputManagedData->GetData<void>();
        auto *dst = nonMovArray->GetData<void>();
        ASSERT(dst != nullptr);
        ASSERT(src != nullptr);
        [[maybe_unused]] auto err = memcpy_s(dst, bytesLen, src, srcLen);
        ASSERT(err == EOK);
    }
    ObjectAccessor::SetObject(coro, handle.GetPtr(), GetManagedDataOffset(), nonMovArray->GetCoreType());

    // Without full memory barrier it is possible that architectures with weak memory order can try fetching
    // managed data before it's set
    arch::FullMemoryBarrier();

    ASSERT(IsNonMovableArray(coro, handle.GetPtr()));
}

EtsInt EtsStdCoreArrayBuffer::GetByteLength() const
{
    return ObjectAccessor::GetPrimitive<EtsInt>(this, GetByteLengthOffset());
}

EtsByte EtsStdCoreArrayBuffer::At(EtsInt pos) const
{
    if (!DoBoundaryCheck(pos)) {
        return 0;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return reinterpret_cast<int8_t *>(GetData())[pos];
}

void EtsStdCoreArrayBuffer::Set(EtsInt pos, EtsByte val)
{
    if (!DoBoundaryCheck(pos)) {
        return;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    reinterpret_cast<int8_t *>(GetData())[pos] = val;
}

void EtsStdCoreArrayBuffer::Initialize(EtsCoroutine *coro, size_t length, EtsByteArray *array)
{
    ASSERT(array != nullptr);
    ObjectAccessor::SetObject(coro, this, GetManagedDataOffset(), array->GetCoreType());
    ObjectAccessor::SetPrimitive(this, GetByteLengthOffset(), static_cast<decltype(byteLength_)>(length));
    ObjectAccessor::SetPrimitive(this, GetNativeDataOffset(), 0);  // not needed in managed code
    ObjectAccessor::SetPrimitive(this, GetIsResizableOffset(), ToEtsBoolean(false));

    // Without full memory barrier it is possible that architectures with weak memory order can try fetching
    // managed data before it's set
    arch::FullMemoryBarrier();
}

void EtsStdCoreArrayBuffer::SetValues(EtsStdCoreArrayBuffer *other, EtsInt begin)
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

bool EtsStdCoreArrayBuffer::DoBoundaryCheck(EtsInt pos) const
{
    if (pos < 0 || pos >= GetByteLength()) {
        PandaString message = "ArrayBuffer position ";
        message.append(ToPandaString(pos)).append(" is out of bounds");
        ThrowEtsException(EtsCoroutine::GetCurrent(), PlatformTypes()->coreIndexOutOfBoundsError, message.c_str());
        return false;
    }
    return true;
}

}  // namespace ark::ets
