/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "libpandabase/utils/utils.h"
#include "libpandabase/utils/utf.h"
#include "runtime/arch/memory_helpers.h"
#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_string_builder.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include <cstdint>
#include <cmath>

namespace ark::ets {

/// StringBuilder fields offsets
static constexpr uint32_t SB_BUFFER_OFFSET = ark::ObjectHeader::ObjectHeaderSize();
static constexpr uint32_t SB_INDEX_OFFSET = SB_BUFFER_OFFSET + ark::OBJECT_POINTER_SIZE;
static constexpr uint32_t SB_LENGTH_OFFSET = SB_INDEX_OFFSET + sizeof(int32_t);
static constexpr uint32_t SB_COMPRESS_OFFSET = SB_LENGTH_OFFSET + sizeof(int32_t);

/// "null", "true" and "false" packed to integral types
static constexpr uint64_t NULL_CODE = 0x006C006C0075006E;
static constexpr uint64_t TRUE_CODE = 0x0065007500720074;
static constexpr uint64_t FALS_CODE = 0x0073006c00610066;
static constexpr uint16_t E_CODE = 0x0065;

static_assert(std::is_same_v<EtsBoolean, uint8_t>);
static_assert(std::is_same_v<EtsChar, uint16_t> &&
              std::is_same_v<EtsCharArray, EtsPrimitiveArray<EtsChar, EtsClassRoot::CHAR_ARRAY>>);

// The following implementation is based on ObjectHeader::ShallowCopy
static EtsObjectArray *ReallocateBuffer(EtsHandle<EtsObjectArray> &bufHandle)
{
    uint32_t bufLen = bufHandle->GetLength();
    ASSERT(bufLen < (UINT_MAX >> 1U));
    // Allocate the new buffer - may trigger GC
    auto *newBuf = EtsObjectArray::Create(bufHandle->GetClass(), 2 * bufLen);
    // Copy the old buffer data
    bufHandle->CopyDataTo(newBuf);
    EVENT_SB_BUFFER_REALLOC(ManagedThread::GetCurrent()->GetId(), newBuf, newBuf->GetLength(), newBuf->GetElementSize(),
                            newBuf->ObjectSize());
    return newBuf;
}

// A string representations of nullptr, bool, short, int, long, float and double
// do not contain uncompressable chars. So we may skip 'compress' check in these cases.
template <bool CHECK_IF_COMPRESSABLE = true>
ObjectHeader *AppendCharArrayToBuffer(VMHandle<EtsObject> &sbHandle, EtsCharArray *arr)
{
    auto *sb = sbHandle.GetPtr();
    auto length = sb->GetFieldPrimitive<uint32_t>(SB_LENGTH_OFFSET);
    auto index = sb->GetFieldPrimitive<uint32_t>(SB_INDEX_OFFSET);
    auto *buf = reinterpret_cast<EtsObjectArray *>(sb->GetFieldObject(SB_BUFFER_OFFSET));

    // Check the case of the buf overflow
    uint32_t bufLen = buf->GetLength();
    if (index >= bufLen) {
        auto *coroutine = EtsCoroutine::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
        EtsHandle<EtsCharArray> arrHandle(coroutine, arr);
        EtsHandle<EtsObjectArray> bufHandle(coroutine, buf);
        // May trigger GC
        buf = ReallocateBuffer(bufHandle);
        // Update sb and arr as corresponding objects might be moved by GC
        sb = sbHandle.GetPtr();
        arr = arrHandle.GetPtr();
        // Remember the new buffer
        sb->SetFieldObject(SB_BUFFER_OFFSET, reinterpret_cast<EtsObject *>(buf));
    }

    // Append array to the buf
    buf->Set(index, reinterpret_cast<EtsObject *>(arr));
    // Increment the index
    sb->SetFieldPrimitive<uint32_t>(SB_INDEX_OFFSET, index + 1U);
    // Increase the length
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    sb->SetFieldPrimitive<uint32_t>(SB_LENGTH_OFFSET, length + arr->GetLength());
    // If string compression is disabled in the runtime, then set 'StringBuilder.compress' to 'false',
    // as by default 'StringBuilder.compress' is 'true'.
    if (!Runtime::GetCurrent()->GetOptions().IsRuntimeCompressedStringsEnabled()) {
        if (sb->GetFieldPrimitive<bool>(SB_COMPRESS_OFFSET)) {
            sb->SetFieldPrimitive<bool>(SB_COMPRESS_OFFSET, false);
        }
    } else if (CHECK_IF_COMPRESSABLE && sb->GetFieldPrimitive<bool>(SB_COMPRESS_OFFSET)) {
        // Set the compress field to false if the array contains not compressable chars
        auto n = arr->GetLength();
        for (uint32_t i = 0; i < n; ++i) {
            if (!ark::coretypes::String::IsASCIICharacter(arr->Get(i))) {
                sb->SetFieldPrimitive<bool>(SB_COMPRESS_OFFSET, false);
                break;
            }
        }
    }
    return sb->GetCoreType();
}

static void ReconstructStringAsMUtf8(EtsString *dstString, EtsObjectArray *buffer, uint32_t index, uint32_t length,
                                     EtsClass *stringKlass)
{
    // All strings in the buf are MUtf8
    uint8_t *dstData = dstString->GetDataMUtf8();
    for (uint32_t i = 0; i < index; ++i) {
        EtsObject *obj = buffer->Get(i);
        if (obj->IsInstanceOf(stringKlass)) {
            coretypes::String *srcString = reinterpret_cast<EtsString *>(obj)->GetCoreType();
            uint32_t n = srcString->CopyDataRegionMUtf8(dstData, 0, srcString->GetLength(), length);
            dstData += n;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            length -= n;
        } else {
            // obj is an array of chars
            coretypes::Array *srcArray = reinterpret_cast<EtsArray *>(obj)->GetCoreType();
            uint32_t n = srcArray->GetLength();
            for (uint32_t j = 0; j < n; ++j) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                dstData[j] = srcArray->GetPrimitive<uint16_t>(sizeof(uint16_t) * j);
            }
            dstData += n;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            length -= n;
        }
    }
}

static void ReconstructStringAsUtf16(EtsString *dstString, EtsObjectArray *buffer, uint32_t index, uint32_t length,
                                     EtsClass *stringKlass)
{
    // Some strings in the buf are Utf16
    uint16_t *dstData = dstString->GetDataUtf16();
    for (uint32_t i = 0; i < index; ++i) {
        EtsObject *obj = buffer->Get(i);
        if (obj->IsInstanceOf(stringKlass)) {
            coretypes::String *srcString = reinterpret_cast<EtsString *>(obj)->GetCoreType();
            uint32_t n = srcString->CopyDataRegionUtf16(dstData, 0, srcString->GetLength(), length);
            dstData += n;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            length -= n;
        } else {
            // obj is an array of chars
            coretypes::Array *srcArray = reinterpret_cast<EtsCharArray *>(obj)->GetCoreType();
            auto *srcData = reinterpret_cast<EtsChar *>(srcArray->GetData());
            uint32_t n = srcArray->GetLength();
            switch (n) {
                case 1U:  // 2 bytes
                    *dstData = *reinterpret_cast<EtsChar *>(srcData);
                    break;
                case 2U:  // 4 bytes
                    *reinterpret_cast<uint32_t *>(dstData) = *reinterpret_cast<uint32_t *>(srcData);
                    break;
                case 4U:  // 8 bytes
                    *reinterpret_cast<uint64_t *>(dstData) = *reinterpret_cast<uint64_t *>(srcData);
                    break;
                default:
                    memcpy_s(static_cast<void *>(dstData), n << 1UL, static_cast<void *>(srcData), n << 1UL);
            }
            dstData += n;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            length -= n;
        }
    }
}

static inline EtsCharArray *NullToCharArray()
{
    EtsCharArray *arr = EtsCharArray::Create(std::char_traits<char>::length("null"));
    *reinterpret_cast<uint64_t *>(arr->GetData<EtsChar>()) = NULL_CODE;
    return arr;
}

static inline EtsCharArray *BoolToCharArray(EtsBoolean v)
{
    auto arrLen = v != 0U ? std::char_traits<char>::length("true") : std::char_traits<char>::length("false");
    EtsCharArray *arr = EtsCharArray::Create(arrLen);
    auto *data = reinterpret_cast<uint64_t *>(arr->GetData<EtsChar>());
    if (v != 0U) {
        *data = TRUE_CODE;
    } else {
        *data = FALS_CODE;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *reinterpret_cast<EtsChar *>(data + 1) = E_CODE;
    }
    return arr;
}

static inline EtsCharArray *CharToCharArray(EtsChar v)
{
    EtsCharArray *arr = EtsCharArray::Create(1U);
    *(reinterpret_cast<EtsChar *>(arr->GetData<EtsChar>())) = v;
    return arr;
}

static inline EtsCharArray *LongToCharArray(EtsLong v)
{
    auto sign = static_cast<uint32_t>(v < 0);
    auto nDigits = CountDigits(std::abs(v)) + sign;
    EtsCharArray *arr = EtsCharArray::Create(nDigits);
    auto *arrData = reinterpret_cast<EtsChar *>(arr->GetData<EtsChar>());
    utf::UInt64ToUtf16Array(std::abs(v), arrData, nDigits, sign != 0U);
    return arr;
}

ObjectHeader *StringBuilderAppendNullString(ObjectHeader *sb)
{
    ASSERT(sb != nullptr);
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> sbHandle(coroutine, sb);
    // May trigger GC
    EtsCharArray *arr = NullToCharArray();
    return AppendCharArrayToBuffer<false>(sbHandle, arr);
}

/**
 * Implementation of public native append(s: String): StringBuilder.
 * Inserts the string 's' into a free buffer slot:
 *
 *    buf[index] = s;
 *    index++;
 *    length += s.length
 *    compress &= s.IsMUtf8()
 *
 * In case of the buf overflow, we create a new buffer of a larger size
 * and copy the data from the old buffer.
 */
ObjectHeader *StringBuilderAppendString(ObjectHeader *sb, EtsString *str)
{
    ASSERT(sb != nullptr);

    if (str == nullptr) {
        return StringBuilderAppendNullString(sb);
    }
    if (str->GetLength() == 0) {
        return sb;
    }

    auto index = sb->GetFieldPrimitive<uint32_t>(SB_INDEX_OFFSET);
    auto *buf = reinterpret_cast<EtsObjectArray *>(sb->GetFieldObject(SB_BUFFER_OFFSET));
    // Check buf overflow
    if (index >= buf->GetLength()) {
        auto *coroutine = EtsCoroutine::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
        VMHandle<EtsObject> sbHandle(coroutine, sb);
        EtsHandle<EtsString> strHandle(coroutine, str);
        EtsHandle<EtsObjectArray> bufHandle(coroutine, buf);
        // May trigger GC
        buf = ReallocateBuffer(bufHandle);
        // Update sb and s as corresponding objects might be moved by GC
        sb = sbHandle->GetCoreType();
        str = strHandle.GetPtr();
        // Remember the new buffer
        sb->SetFieldObject(SB_BUFFER_OFFSET, reinterpret_cast<ObjectHeader *>(buf));
    }
    // Append string to the buf
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    buf->Set(index, reinterpret_cast<EtsObject *>(str));
    // Increment the index
    sb->SetFieldPrimitive<uint32_t>(SB_INDEX_OFFSET, index + 1U);
    // Increase the length
    auto length = sb->GetFieldPrimitive<uint32_t>(SB_LENGTH_OFFSET);
    sb->SetFieldPrimitive<uint32_t>(SB_LENGTH_OFFSET, length + str->GetLength());
    // Set the compress field to false if the string is not compressable
    if (sb->GetFieldPrimitive<bool>(SB_COMPRESS_OFFSET) && str->IsUtf16()) {
        sb->SetFieldPrimitive<bool>(SB_COMPRESS_OFFSET, false);
    }

    return sb;
}

ObjectHeader *StringBuilderAppendChar(ObjectHeader *sb, EtsChar v)
{
    ASSERT(sb != nullptr);

    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> sbHandle(coroutine, sb);

    // May trigger GC
    auto *arr = CharToCharArray(v);
    return AppendCharArrayToBuffer(sbHandle, arr);
}

ObjectHeader *StringBuilderAppendBool(ObjectHeader *sb, EtsBoolean v)
{
    ASSERT(sb != nullptr);

    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> sbHandle(coroutine, sb);

    // May trigger GC
    auto *arr = BoolToCharArray(v);
    return AppendCharArrayToBuffer<false>(sbHandle, arr);
}

ObjectHeader *StringBuilderAppendLong(ObjectHeader *sb, EtsLong v)
{
    ASSERT(sb != nullptr);

    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> sbHandle(coroutine, sb);

    // May trigger GC
    auto *arr = LongToCharArray(v);
    return AppendCharArrayToBuffer<false>(sbHandle, arr);
}

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
static inline EtsCharArray *FloatingPointToCharArray(FpType number)
{
    PandaString str;
    intrinsics::helpers::FpToStringDecimalRadix(number, str);
    auto len = str.length();
    EtsCharArray *arr = EtsCharArray::Create(len);
    Span<uint16_t> data(reinterpret_cast<uint16_t *>(arr->GetData<EtsChar>()), len);
    for (size_t i = 0; i < len; ++i) {
        ASSERT(ark::coretypes::String::IsASCIICharacter(str[i]));
        data[i] = static_cast<uint16_t>(str[i]);
    }
    return arr;
}

ObjectHeader *StringBuilderAppendFloat(ObjectHeader *sb, EtsFloat v)
{
    ASSERT(sb != nullptr);

    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> sbHandle(coroutine, sb);

    auto *arr = FloatingPointToCharArray(v);
    return AppendCharArrayToBuffer<false>(sbHandle, arr);
}

ObjectHeader *StringBuilderAppendDouble(ObjectHeader *sb, EtsDouble v)
{
    ASSERT(sb != nullptr);

    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> sbHandle(coroutine, sb);

    auto *arr = FloatingPointToCharArray(v);
    return AppendCharArrayToBuffer<false>(sbHandle, arr);
}

EtsString *StringBuilderToString(ObjectHeader *sb)
{
    ASSERT(sb != nullptr);

    auto length = sb->GetFieldPrimitive<uint32_t>(SB_LENGTH_OFFSET);
    if (length == 0) {
        return EtsString::CreateNewEmptyString();
    }

    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> sbHandle(coroutine, sb);

    auto index = sbHandle->GetFieldPrimitive<uint32_t>(SB_INDEX_OFFSET);
    auto compress = sbHandle->GetFieldPrimitive<bool>(SB_COMPRESS_OFFSET);
    EtsString *s = EtsString::AllocateNonInitializedString(length, compress);
    EtsClass *sKlass = EtsClass::FromRuntimeClass(s->GetCoreType()->ClassAddr<Class>());
    auto *buf = reinterpret_cast<EtsObjectArray *>(sbHandle->GetFieldObject(SB_BUFFER_OFFSET));
    if (compress) {
        ReconstructStringAsMUtf8(s, buf, index, length, sKlass);
    } else {
        ReconstructStringAsUtf16(s, buf, index, length, sKlass);
    }
    return s;
}

}  // namespace ark::ets
