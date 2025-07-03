/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "plugins/ets/runtime/types/ets_type.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"

#include "runtime/handle_scope.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/thread.h"

namespace ark::ets::intrinsics {

/* make sure the class the intrinsic being called from belongs to the boot context */
static bool EnsureBootContext()
{
    auto *coro = EtsCoroutine::GetCurrent();
    auto ctx = StackWalker::Create(coro).GetMethod()->GetClass()->GetLoadContext();
    if (!ctx->IsBootContext()) {
        auto e = panda_file_items::class_descriptors::ILLEGAL_STATE_EXCEPTION;
        auto msg = "Unsafe intrinsics cannot be called outside of the boot context";
        ThrowEtsException(coro, e, msg);
        return false;
    }
    return true;
}

template <typename T>
static T UnsafeMemoryRead(void *addr)
{
    if (EnsureBootContext()) {
        return *reinterpret_cast<T *>(addr);
    }
    return static_cast<T>(-1);
}

template <typename T>
static void UnsafeMemoryWrite(void *addr, T val)
{
    if (EnsureBootContext()) {
        *reinterpret_cast<T *>(addr) = val;
    }
}

extern "C" EtsByte UnsafeMemoryReadBoolean(void *addr)
{
    return UnsafeMemoryRead<EtsByte>(addr);
}

extern "C" EtsByte UnsafeMemoryReadInt8(void *addr)
{
    return UnsafeMemoryRead<EtsByte>(addr);
}

extern "C" EtsShort UnsafeMemoryReadInt16(void *addr)
{
    return UnsafeMemoryRead<EtsShort>(addr);
}

extern "C" EtsInt UnsafeMemoryReadInt32(void *addr)
{
    return UnsafeMemoryRead<EtsInt>(addr);
}

extern "C" EtsLong UnsafeMemoryReadInt64(void *addr)
{
    return UnsafeMemoryRead<EtsLong>(addr);
}

extern "C" EtsFloat UnsafeMemoryReadFloat32(void *addr)
{
    return UnsafeMemoryRead<EtsFloat>(addr);
}

extern "C" EtsDouble UnsafeMemoryReadFloat64(void *addr)
{
    return UnsafeMemoryRead<EtsDouble>(addr);
}

extern "C" EtsDouble UnsafeMemoryReadNumber(void *addr)
{
    return UnsafeMemoryRead<EtsDouble>(addr);
}

extern "C" void UnsafeMemoryWriteBoolean(void *addr, EtsBoolean val)
{
    UnsafeMemoryWrite<EtsByte>(addr, val);
}

extern "C" void UnsafeMemoryWriteInt8(void *addr, EtsByte val)
{
    UnsafeMemoryWrite<EtsByte>(addr, val);
}

extern "C" void UnsafeMemoryWriteInt16(void *addr, EtsShort val)
{
    UnsafeMemoryWrite<EtsShort>(addr, val);
}

extern "C" void UnsafeMemoryWriteInt32(void *addr, EtsInt val)
{
    UnsafeMemoryWrite<EtsInt>(addr, val);
}

extern "C" void UnsafeMemoryWriteInt64(void *addr, EtsLong val)
{
    UnsafeMemoryWrite<EtsLong>(addr, val);
}

extern "C" void UnsafeMemoryWriteFloat32(void *addr, EtsFloat val)
{
    UnsafeMemoryWrite<EtsFloat>(addr, val);
}

extern "C" void UnsafeMemoryWriteFloat64(void *addr, EtsDouble val)
{
    UnsafeMemoryWrite<EtsDouble>(addr, val);
}

extern "C" void UnsafeMemoryWriteNumber(void *addr, EtsDouble val)
{
    UnsafeMemoryWrite<EtsDouble>(addr, val);
}

/* get the size of the buffer to hold the string content add additional
   2 bytes for the leading byte-order mark which is used in the UTF-16
   case */
extern "C" int UnsafeMemoryStringGetSizeInBytes(EtsString *str)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    EtsHandle<EtsString> handle(coroutine, str);

    if (!EnsureBootContext()) {
        return -1;
    }

    str = handle.GetPtr();

    uint32_t shift = str->IsUtf16() ? 1 : 0;
    uint32_t addend = str->IsUtf16() ? 2 : 0;
    auto length = static_cast<uint32_t>(str->GetLength());
    int size = (length << shift) + addend;
    return size;
}

static constexpr uint16_t UTF16_BOM = 0xFEFF;

extern "C" EtsString *UnsafeMemoryReadString(void *buf, int len)
{
    if (!EnsureBootContext()) {
        return nullptr;
    }

    ObjectHeader *res = nullptr;
    auto vm = ManagedThread::GetCurrent()->GetVM();
    auto ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    auto mark = reinterpret_cast<uint16_t *>(buf);
    auto size = static_cast<uint32_t>(len);

    if (*mark == UTF16_BOM) {
        auto addr = ++mark;
        /* size is in bytes but CreateFromUtf16 takes it as chars, so,
           we divide it by half and subtract the length of the BOM */
        res = coretypes::String::CreateFromUtf16(addr, (size / 2U) - 1, false, ctx, vm);
    } else {
        auto addr = reinterpret_cast<const uint8_t *>(buf);
        res = coretypes::String::CreateFromMUtf8(addr, size, size, true, ctx, vm);
    }

    return reinterpret_cast<EtsString *>(res);
}

extern "C" int UnsafeMemoryWriteString(void *addr, EtsString *str)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    EtsHandle<EtsString> handle(coroutine, str);

    if (!EnsureBootContext()) {
        return -1;
    }

    str = handle.GetPtr();

    auto size = static_cast<uint32_t>(str->GetLength());
    if (str->IsUtf16()) {
        *reinterpret_cast<uint16_t *>(addr) = UTF16_BOM;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        addr = reinterpret_cast<char *>(addr) + 2U;
        size <<= 1U;
        return memcpy_s(addr, size, str->GetDataUtf16(), size) == 0 ? size + 2U : -1;
    }
    return memcpy_s(addr, size, str->GetDataMUtf8(), size) == 0 ? size : -1;
}

}  // namespace ark::ets::intrinsics
