/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "intrinsics.h"
#include "intrinsics/helpers/reflection_helpers.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_reflect_field.h"
#include "plugins/ets/runtime/types/ets_reflect_method.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "helpers/json_helper.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "runtime/common_interfaces/objects/string/base_string-inl.h"

#include <array>
#include <limits>
#include <vector>

namespace ark::ets::intrinsics {

static constexpr int32_t JSON_FVT_UNCLASSIFIED = -1;
static constexpr int32_t JSON_FVT_INT = 3;
static constexpr int32_t JSON_FVT_LONG = 4;
static constexpr int32_t JSON_FVT_DOUBLE = 5;

static constexpr uint16_t JSON_QUOTE = '"';
static constexpr uint16_t JSON_BACKSLASH = '\\';
static constexpr uint16_t JSON_BACKSPACE = '\b';
static constexpr uint16_t JSON_FORM_FEED = '\f';
static constexpr uint16_t JSON_NEW_LINE = '\n';
static constexpr uint16_t JSON_CARRIAGE_RETURN = '\r';
static constexpr uint16_t JSON_TAB = '\t';
static constexpr uint16_t JSON_DELETE = 0x7F;
static constexpr uint16_t JSON_HIGH_SURROGATE_START = 0xD800;
static constexpr uint16_t JSON_HIGH_SURROGATE_END = 0xDBFF;
static constexpr uint16_t JSON_LOW_SURROGATE_START = 0xDC00;
static constexpr uint16_t JSON_LOW_SURROGATE_END = 0xDFFF;
static constexpr uint16_t JSON_CONTROL_CHAR_MAX = 0x20;
static constexpr uint8_t JSON_HEX_NIBBLE_SHIFT = 4U;
static constexpr uint8_t JSON_HEXNIBBLE_MASK = 0xF;
// Chunk size for chunked-escape loops in StdCoreJSONStringifyStringFast.
// Aligned with std_core_Array.cpp CHUNK_SIZE. Each chunk takes a few us, well
// within the GC safepoint budget, and Safepoint() is called between chunks so
// that long inputs do not block cooperative STW GC.
// CC-OFFNXT(G.NAM.03) project code style
static constexpr uint32_t JSON_STRINGIFY_CHUNK_SIZE = 1U << 9U;

static constexpr std::array<uint16_t, 16> HEX_DIGITS = {'0', '1', '2', '3', '4', '5', '6', '7',
                                                        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

static bool IsHighSurrogate(uint16_t ch)
{
    return ch >= JSON_HIGH_SURROGATE_START && ch <= JSON_HIGH_SURROGATE_END;
}

static bool IsLowSurrogate(uint16_t ch)
{
    return ch >= JSON_LOW_SURROGATE_START && ch <= JSON_LOW_SURROGATE_END;
}

static bool IsSurrogate(uint16_t ch)
{
    return ch >= JSON_HIGH_SURROGATE_START && ch <= JSON_LOW_SURROGATE_END;
}

static bool NeedsUnicodeEscape(uint16_t ch)
{
    return ch < JSON_CONTROL_CHAR_MAX || ch == JSON_DELETE;
}

static bool NeedsShortEscape(uint16_t ch)
{
    return ch == JSON_QUOTE || ch == JSON_BACKSLASH || ch == JSON_BACKSPACE || ch == JSON_FORM_FEED ||
           ch == JSON_NEW_LINE || ch == JSON_CARRIAGE_RETURN || ch == JSON_TAB;
}

// Single-chunk scanner: accumulates escaped length for [chunkStart, chunkEnd)
// into the caller-provided counter. Does not touch the managed heap; the caller
// is responsible for safepoint polling and re-fetching the data pointer.
static void EscapedStringLengthUtf8Chunk(const uint8_t *data, uint32_t chunkStart, uint32_t chunkEnd, uint32_t *result)
{
    for (uint32_t i = chunkStart; i < chunkEnd; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const uint16_t ch = data[i];
        if (NeedsShortEscape(ch)) {
            *result += 2U;
        } else if (NeedsUnicodeEscape(ch)) {
            *result += 6U;
        } else {
            (*result)++;
        }
    }
}

// Single-chunk scanner for UTF-16. Surrogate-pair boundaries are guaranteed to
// never cross chunk edges (the caller adjusts chunkEnd so a trailing high
// surrogate is deferred to the next chunk), therefore the i + 1U >= chunkEnd
// check correctly detects dangling surrogates within a chunk.
static void EscapedStringLengthUtf16Chunk(const uint16_t *data, uint32_t chunkStart, uint32_t chunkEnd,
                                          uint32_t *result, bool *canBeCompressed)
{
    for (uint32_t i = chunkStart; i < chunkEnd;) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const uint16_t ch = data[i];
        if (NeedsShortEscape(ch)) {
            *result += 2U;
            i++;
        } else if (NeedsUnicodeEscape(ch) ||
                   (IsSurrogate(ch) && (!IsHighSurrogate(ch) || i + 1U >= chunkEnd ||
                                        !IsLowSurrogate(
                                            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                            data[i + 1U])))) {
            *result += 6U;
            i++;
        } else if (IsHighSurrogate(ch)) {
            *canBeCompressed = false;
            *result += 2U;
            i += 2U;
        } else {
            if (ch > std::numeric_limits<uint8_t>::max()) {
                *canBeCompressed = false;
            }
            (*result)++;
            i++;
        }
    }
}

template <class T>
static void WriteJsonChar(T *data, uint32_t *index, uint16_t ch)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    data[(*index)++] = static_cast<T>(ch);
}

template <class T>
static void WriteUnicodeEscape(T *data, uint32_t *index, uint16_t ch)
{
    WriteJsonChar(data, index, JSON_BACKSLASH);
    WriteJsonChar(data, index, 'u');
    // NOLINTNEXTLINE(modernize-use-auto)
    const uint16_t chVal = static_cast<uint16_t>(ch);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    WriteJsonChar(data, index, HEX_DIGITS[(chVal >> (JSON_HEX_NIBBLE_SHIFT * 3U)) & JSON_HEXNIBBLE_MASK]);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    WriteJsonChar(data, index, HEX_DIGITS[(chVal >> (JSON_HEX_NIBBLE_SHIFT * 2U)) & JSON_HEXNIBBLE_MASK]);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    WriteJsonChar(data, index, HEX_DIGITS[(chVal >> JSON_HEX_NIBBLE_SHIFT) & JSON_HEXNIBBLE_MASK]);
    WriteJsonChar(data, index, HEX_DIGITS[chVal & JSON_HEXNIBBLE_MASK]);
}

template <class T>
static void WriteShortEscape(T *data, uint32_t *index, uint16_t ch)
{
    WriteJsonChar(data, index, JSON_BACKSLASH);
    switch (ch) {
        case JSON_QUOTE:
            WriteJsonChar(data, index, JSON_QUOTE);
            return;
        case JSON_BACKSLASH:
            WriteJsonChar(data, index, JSON_BACKSLASH);
            return;
        case JSON_BACKSPACE:
            WriteJsonChar(data, index, 'b');
            return;
        case JSON_FORM_FEED:
            WriteJsonChar(data, index, 'f');
            return;
        case JSON_NEW_LINE:
            WriteJsonChar(data, index, 'n');
            return;
        case JSON_CARRIAGE_RETURN:
            WriteJsonChar(data, index, 'r');
            return;
        case JSON_TAB:
            WriteJsonChar(data, index, 't');
            return;
        default:
            UNREACHABLE();
    }
}

// Single-chunk writer for the UTF-8 (MUtf8) path. Writes escaped chars for
// [chunkStart, chunkEnd) into dst starting at *index. Caller writes the
// surrounding JSON quotes and is responsible for safepoint polling.
template <class T>
static void WriteEscapedStringUtf8Chunk(T *dst, uint32_t *index, const uint8_t *src, uint32_t chunkStart,
                                        uint32_t chunkEnd)
{
    for (uint32_t i = chunkStart; i < chunkEnd; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const uint16_t ch = src[i];
        if (NeedsShortEscape(ch)) {
            WriteShortEscape(dst, index, ch);
        } else if (NeedsUnicodeEscape(ch)) {
            WriteUnicodeEscape(dst, index, ch);
        } else {
            WriteJsonChar(dst, index, ch);
        }
    }
}

// Single-chunk writer for the UTF-16 path. Surrogate pairs never cross chunk
// edges (caller-enforced invariant), so the dangling-surrogate check uses
// chunkEnd and behaves identically to the original whole-string scan.
template <class T>
static void WriteEscapedStringUtf16Chunk(T *dst, uint32_t *index, const uint16_t *src, uint32_t chunkStart,
                                         uint32_t chunkEnd)
{
    for (uint32_t i = chunkStart; i < chunkEnd;) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const uint16_t ch = src[i];
        if (NeedsShortEscape(ch)) {
            WriteShortEscape(dst, index, ch);
            i++;
        } else if (NeedsUnicodeEscape(ch) ||
                   (IsSurrogate(ch) && (!IsHighSurrogate(ch) || i + 1U >= chunkEnd ||
                                        !IsLowSurrogate(
                                            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                            src[i + 1U])))) {
            WriteUnicodeEscape(dst, index, ch);
            i++;
        } else if (IsHighSurrogate(ch)) {
            WriteJsonChar(dst, index, ch);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            WriteJsonChar(dst, index, src[i + 1U]);
            i += 2U;
        } else {
            WriteJsonChar(dst, index, ch);
            i++;
        }
    }
}

static EtsBoolean IsFieldHasAnnotation(EtsReflectField *reflectField, char const *annotClassDescriptor)
{
    EtsField *field = reflectField->GetEtsField();
    bool result = false;
    auto *runtimeClass = field->GetDeclaringClass()->GetRuntimeClass();
    const panda_file::File &pf = *runtimeClass->GetPandaFile();
    panda_file::FieldDataAccessor fda(pf, field->GetRuntimeField()->GetFileId());
    fda.EnumerateAnnotations([&pf, &result, &annotClassDescriptor](panda_file::File::EntityId annId) {
        panda_file::AnnotationDataAccessor ada(pf, annId);
        if (utf::IsEqual(utf::CStringAsMutf8(annotClassDescriptor), pf.GetStringData(ada.GetClassId()).data)) {
            result = true;
        }
    });
    return static_cast<EtsBoolean>(result);
}

EtsBoolean StdCoreJSONGetJSONStringifyIgnore(EtsReflectField *reflectField)
{
    return IsFieldHasAnnotation(reflectField, EtsPlatformTypes::DESCRIPTOR_coreJSONStringifyIgnore);
}

EtsBoolean StdCoreJSONGetJSONParseIgnore(EtsReflectField *reflectField)
{
    return IsFieldHasAnnotation(reflectField, EtsPlatformTypes::DESCRIPTOR_coreJSONParseIgnore);
}

EtsString *StdCoreJSONGetJSONRename(EtsReflectField *reflectField)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);

    EtsField *field = reflectField->GetEtsField();
    EtsHandle<EtsString> retStrHandle;
    auto *runtimeClass = field->GetDeclaringClass()->GetRuntimeClass();
    const panda_file::File &pf = *runtimeClass->GetPandaFile();
    panda_file::FieldDataAccessor fda(pf, field->GetRuntimeField()->GetFileId());
    fda.EnumerateAnnotations([&pf, &retStrHandle, &executionCtx](panda_file::File::EntityId annId) {
        panda_file::AnnotationDataAccessor ada(pf, annId);
        if (utf::IsEqual(utf::CStringAsMutf8(EtsPlatformTypes::DESCRIPTOR_coreJSONRename),
                         pf.GetStringData(ada.GetClassId()).data)) {
            const auto value = ada.GetElement(0).GetScalarValue();
            const auto id = value.Get<panda_file::File::EntityId>();
            auto stringData = pf.GetStringData(id);
            retStrHandle = EtsHandle<EtsString>(
                executionCtx, EtsString::CreateFromMUtf8(reinterpret_cast<const char *>(stringData.data)));
        }
    });
    return retStrHandle.GetPtr();
}

EtsBoolean StdCoreJSONGetJSONStringifyGetter(EtsReflectMethod *reflectMethod)
{
    EtsMethod *method = reflectMethod->GetEtsMethod();
    bool result = false;
    const panda_file::File &pf = *method->GetPandaMethod()->GetPandaFile();
    panda_file::MethodDataAccessor mda(pf, method->GetPandaMethod()->GetFileId());
    mda.EnumerateAnnotations([&pf, &result](panda_file::File::EntityId annId) {
        panda_file::AnnotationDataAccessor ada(pf, annId);
        if (utf::IsEqual(utf::CStringAsMutf8(EtsPlatformTypes::DESCRIPTOR_coreJSONStringifyGetter),
                         pf.GetStringData(ada.GetClassId()).data)) {
            result = true;
        }
    });
    return static_cast<EtsBoolean>(result);
}

EtsInt StdCoreJSONGetFieldDeclaredFastType(EtsReflectField *reflectField)
{
    auto *field = reflectField->GetEtsField();
    ASSERT(field != nullptr);
    switch (field->GetEtsType()) {
        case EtsType::INT:
            return JSON_FVT_INT;
        case EtsType::LONG:
            return JSON_FVT_LONG;
        case EtsType::DOUBLE:
            return JSON_FVT_DOUBLE;
        default:
            return JSON_FVT_UNCLASSIFIED;
    }
}

EtsInt StdCoreJSONGetIntFieldFast(EtsReflectField *reflectField, EtsObject *obj)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObject> objHandle(executionCtx, obj);
    auto *field = reflectField->GetEtsField();
    ASSERT(field != nullptr);
    if (!field->IsStatic() && !helpers::ValidateInstanceField(executionCtx, objHandle.GetPtr(), field)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return 0;
    }
    return helpers::ReflectFieldGetPrimitive<EtsInt>(objHandle.GetPtr(), field).GetAs<EtsInt>();
}

EtsLong StdCoreJSONGetLongFieldFast(EtsReflectField *reflectField, EtsObject *obj)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObject> objHandle(executionCtx, obj);
    auto *field = reflectField->GetEtsField();
    ASSERT(field != nullptr);
    if (!field->IsStatic() && !helpers::ValidateInstanceField(executionCtx, objHandle.GetPtr(), field)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return 0;
    }
    return helpers::ReflectFieldGetPrimitive<EtsLong>(objHandle.GetPtr(), field).GetAs<EtsLong>();
}

EtsDouble StdCoreJSONGetDoubleFieldFast(EtsReflectField *reflectField, EtsObject *obj)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObject> objHandle(executionCtx, obj);
    auto *field = reflectField->GetEtsField();
    ASSERT(field != nullptr);
    if (!field->IsStatic() && !helpers::ValidateInstanceField(executionCtx, objHandle.GetPtr(), field)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return 0.0;
    }
    return helpers::ReflectFieldGetPrimitive<EtsDouble>(objHandle.GetPtr(), field).GetAs<EtsDouble>();
}

extern "C" EtsString *StdCoreJSONStringifyStringFast(EtsString *str)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx->GetMT()->HasPendingException() == false);

    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsString> strHandle(executionCtx, str);

    std::vector<uint8_t> flat8Buf;
    std::vector<uint16_t> flat16Buf;
    auto readBarrier = [](void *obj, size_t offset) {
        return reinterpret_cast<ark::mem::BaseString *>(ObjectAccessor::GetObject(obj, offset));
    };
    // CC-OFFNXT(G.FMT.14)
    auto getUtf8Data = [&flat8Buf, &readBarrier](EtsString *value) -> const uint8_t * {
        if (value->IsLineString()) {
            return value->GetDataMUtf8();
        }
        return ark::mem::BaseString::GetUtf8DataFlat(readBarrier, value->GetCoreType()->ToStringConst(), flat8Buf);
    };
    // CC-OFFNXT(G.FMT.14)
    auto getUtf16Data = [&flat16Buf, &readBarrier](EtsString *value) -> const uint16_t * {
        if (value->IsLineString()) {
            return value->GetDataUtf16();
        }
        return ark::mem::BaseString::GetUtf16DataFlat(readBarrier, value->GetCoreType()->ToStringConst(), flat16Buf);
    };

    const uint32_t len = strHandle->GetLength();
    // result is GC-protected: AllocateNonInitializedString and Safepoint() can
    // both move it, so every dereference goes through the handle.
    EtsHandle<EtsString> resultHandle;

    if (strHandle->IsUtf16()) {
        // ---------- Pass 1: compute escaped length (chunked + safepoint) ----------
        uint32_t resultLength = 2U;  // reserve space for both surrounding quotes
        bool canBeCompressed = true;
        uint32_t i = 0U;
        while (i < len) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const uint16_t *srcData = getUtf16Data(strHandle.GetPtr());
            uint32_t chunkEnd = std::min(i + JSON_STRINGIFY_CHUNK_SIZE, len);
            // Keep surrogate pairs within one chunk: if the last position of this
            // chunk is a high surrogate and more data follows, defer it to the
            // next chunk so the pair is never split across a safepoint boundary.
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (chunkEnd < len && IsHighSurrogate(srcData[chunkEnd - 1U])) {
                chunkEnd--;
            }
            EscapedStringLengthUtf16Chunk(srcData, i, chunkEnd, &resultLength, &canBeCompressed);
            i = chunkEnd;
            executionCtx->GetMT()->Safepoint();
        }

        // ---------- Allocate result (may trigger GC, hence handled below) ----------
        resultHandle =
            EtsHandle<EtsString>(executionCtx, EtsString::AllocateNonInitializedString(resultLength, canBeCompressed));

        // ---------- Pass 2: fill buffer (chunked + safepoint) ----------
        flat16Buf.clear();
        uint32_t dstIndex = 0U;
        if (canBeCompressed) {
            WriteJsonChar(resultHandle->GetDataMUtf8(), &dstIndex, JSON_QUOTE);
        } else {
            WriteJsonChar(resultHandle->GetDataUtf16(), &dstIndex, JSON_QUOTE);
        }
        i = 0U;
        while (i < len) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const uint16_t *srcData = getUtf16Data(strHandle.GetPtr());
            uint32_t chunkEnd = std::min(i + JSON_STRINGIFY_CHUNK_SIZE, len);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (chunkEnd < len && IsHighSurrogate(srcData[chunkEnd - 1U])) {
                chunkEnd--;
            }
            // dst is re-fetched every chunk: a safepoint at the previous iteration
            // may have moved the result string in the managed heap.
            if (canBeCompressed) {
                WriteEscapedStringUtf16Chunk(resultHandle->GetDataMUtf8(), &dstIndex, srcData, i, chunkEnd);
            } else {
                WriteEscapedStringUtf16Chunk(resultHandle->GetDataUtf16(), &dstIndex, srcData, i, chunkEnd);
            }
            i = chunkEnd;
            executionCtx->GetMT()->Safepoint();
        }
        if (canBeCompressed) {
            WriteJsonChar(resultHandle->GetDataMUtf8(), &dstIndex, JSON_QUOTE);
        } else {
            WriteJsonChar(resultHandle->GetDataUtf16(), &dstIndex, JSON_QUOTE);
        }
    } else {
        // ---------- Pass 1: compute escaped length (chunked + safepoint) ----------
        uint32_t resultLength = 2U;
        uint32_t i = 0U;
        while (i < len) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const uint8_t *srcData = getUtf8Data(strHandle.GetPtr());
            uint32_t chunkEnd = std::min(i + JSON_STRINGIFY_CHUNK_SIZE, len);
            EscapedStringLengthUtf8Chunk(srcData, i, chunkEnd, &resultLength);
            i = chunkEnd;
            executionCtx->GetMT()->Safepoint();
        }

        // ---------- Allocate result (may trigger GC, hence handled below) ----------
        resultHandle = EtsHandle<EtsString>(executionCtx, EtsString::AllocateNonInitializedString(resultLength, true));

        // ---------- Pass 2: fill buffer (chunked + safepoint) ----------
        flat8Buf.clear();
        uint32_t dstIndex = 0U;
        WriteJsonChar(resultHandle->GetDataMUtf8(), &dstIndex, JSON_QUOTE);
        i = 0U;
        while (i < len) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            const uint8_t *srcData = getUtf8Data(strHandle.GetPtr());
            uint32_t chunkEnd = std::min(i + JSON_STRINGIFY_CHUNK_SIZE, len);
            // dst is re-fetched every chunk: a safepoint at the previous iteration
            // may have moved the result string in the managed heap.
            WriteEscapedStringUtf8Chunk(resultHandle->GetDataMUtf8(), &dstIndex, srcData, i, chunkEnd);
            i = chunkEnd;
            executionCtx->GetMT()->Safepoint();
        }
        WriteJsonChar(resultHandle->GetDataMUtf8(), &dstIndex, JSON_QUOTE);
    }
    return resultHandle.GetPtr();
}

extern "C" EtsString *StdCoreJSONStringifyFast(EtsObject *value)
{
    auto executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx->GetMT()->HasPendingException() == false);

    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObject> valueHandle(executionCtx, value);

    helpers::JSONStringifier stringifier;
    return stringifier.Stringify(valueHandle);
}

}  // namespace ark::ets::intrinsics
