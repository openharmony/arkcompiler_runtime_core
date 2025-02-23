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

#include "intrinsics.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_arraybuffer.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"

#include "plugins/ets/runtime/intrinsics/helpers/array_buffer_helper.h"

using namespace std::string_view_literals;
constexpr std::array UTF8_ENCODINGS = {"utf8"sv, "utf-8"sv, "ascii"sv};
constexpr std::array UTF16_ENCODINGS = {"utf16le"sv, "ucs2"sv, "ucs-2"sv};
constexpr std::array BASE64_ENCODINGS = {"base64"sv, "base64url"sv};

namespace ark::ets::intrinsics {

/// @brief Creates a new ArrayBuffer with specified size and optional initial data
static EtsHandle<EtsArrayBuffer> CreateArrayBuffer(EtsCoroutine *coro, EtsInt byteLength, const uint8_t *data = nullptr)
{
    EtsClass *arrayBufferClass = coro->GetPandaVM()->GetClassLinker()->GetArrayBufferClass();
    EtsHandle<EtsArrayBuffer> newBuffer(coro,
                                        reinterpret_cast<EtsArrayBuffer *>(EtsObject::Create(coro, arrayBufferClass)));
    if (UNLIKELY(newBuffer.GetPtr() == nullptr)) {
        return newBuffer;
    }
    newBuffer->SetByteLength(byteLength);
    auto *newData = EtsByteArray::Create(byteLength);
    newBuffer->SetData(coro, newData);
    if (newBuffer->GetData() == nullptr) {
        return newBuffer;
    }
    if (data != nullptr && byteLength > 0) {
        std::copy_n(data, byteLength, newBuffer->GetData()->GetData<EtsByte>());
    }
    return newBuffer;
}

/**
 * @brief Converts an ETS string to bytes using specified encoding
 * @details Supported encodings:
 *   - UTF-8/UTF-8
 *   - ASCII (7-bit)
 *   - UTF-16LE/UCS2
 *   - Base64/Base64URL
 *   - Latin1/Binary
 *   - Hex (must be even length)
 */
static PandaVector<uint8_t> ConvertEtsStringToBytes(EtsString *strObj, EtsString *encodingObj, EtsCoroutine *coro,
                                                    const LanguageContext &ctx)
{
    PandaVector<uint8_t> strBuf;
    PandaVector<uint8_t> encodingBuf;
    PandaString input(strObj->ConvertToStringView(&strBuf));
    PandaString encoding =
        encodingObj != nullptr ? PandaString(encodingObj->ConvertToStringView(&encodingBuf)) : "utf8";
    auto res = helpers::encoding::ConvertStringToBytes(input, encoding);
    if (std::holds_alternative<helpers::Err<PandaString>>(res)) {
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8(std::get<helpers::Err<PandaString>>(res).Message().c_str()));
        return PandaVector<uint8_t> {};
    }
    return std::get<PandaVector<uint8_t>>(res);
}

/// @brief Calculates byte length of string when encoded
extern "C" ets_int EtsStringBytesLength(EtsString *strObj, EtsString *encodingObj)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    auto ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    PandaVector<uint8_t> strBuf;
    PandaVector<uint8_t> encodingBuf;
    PandaString input(strObj->ConvertToStringView(&strBuf));
    PandaString encoding =
        encodingObj != nullptr ? PandaString(encodingObj->ConvertToStringView(&encodingBuf)) : "utf8";
    auto res = helpers::encoding::CalculateStringBytesLength(input, encoding);
    if (std::holds_alternative<helpers::Err<PandaString>>(res)) {
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8(std::get<helpers::Err<PandaString>>(res).Message().c_str()));
        return -1;
    }
    return std::get<int32_t>(res);
}

/// @brief Creates ArrayBuffer from encoded string
extern "C" ObjectHeader *EtsArrayBufferFromEncodedString(EtsString *strObj, EtsString *encodingObj, ets_int length)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    PandaVector<uint8_t> bytes = ConvertEtsStringToBytes(strObj, encodingObj, coro, ctx);
    auto byteLength = static_cast<EtsInt>(bytes.size());
    if (length < 0 || length > byteLength) {
        ThrowException(ctx, coro, ctx.GetIndexOutOfBoundsExceptionClassDescriptor(),
                       utf::CStringAsMutf8("Length is out of bounds"));
        return nullptr;
    }
    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<EtsArrayBuffer> newBuffer = CreateArrayBuffer(coro, length, length > 0 ? bytes.data() : nullptr);
    if (newBuffer.GetPtr() == nullptr || newBuffer->GetData() == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<ObjectHeader *>(newBuffer.GetPtr());
}

/// @brief Creates new ArrayBuffer from slice of existing buffer
extern "C" ObjectHeader *EtsArrayBufferFromBufferSlice(ark::ObjectHeader *obj, ets_int offset, ets_int length)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    EtsClass *arrayBufferClass = coro->GetPandaVM()->GetClassLinker()->GetArrayBufferClass();
    auto etsObj = reinterpret_cast<EtsObject *>(obj);
    if (etsObj->GetClass() != arrayBufferClass) {
        ThrowException(ctx, coro, ctx.GetClassCastExceptionClassDescriptor(),
                       utf::CStringAsMutf8("Object is not an ArrayBuffer"));
        return nullptr;
    }

    [[maybe_unused]] EtsHandleScope s(coro);
    EtsHandle<EtsArrayBuffer> original(coro, reinterpret_cast<EtsArrayBuffer *>(etsObj));
    EtsInt origByteLength = original->GetByteLength();
    if (offset < 0 || offset > origByteLength) {
        ThrowException(ctx, coro, ctx.GetIndexOutOfBoundsExceptionClassDescriptor(),
                       utf::CStringAsMutf8("Offset is out of bounds"));
        return nullptr;
    }
    if (length < 0 || length > origByteLength - offset) {
        ThrowException(ctx, coro, ctx.GetIndexOutOfBoundsExceptionClassDescriptor(),
                       utf::CStringAsMutf8("Length is out of bounds"));
        return nullptr;
    }

    EtsHandle<EtsArrayBuffer> newBuffer = CreateArrayBuffer(coro, length);
    if (newBuffer.GetPtr() == nullptr || newBuffer->GetData() == nullptr) {
        return nullptr;
    }
    if (original->GetData() != nullptr) {
        std::copy_n(std::next(original->GetData()->GetData<EtsByte>(), offset), length,
                    newBuffer->GetData()->GetData<EtsByte>());
    }
    return newBuffer.GetPtr();
}

/**
 * @brief Converts ArrayBuffer content to string using specified encoding.
 *
 * This function validates the input buffer and indices, extracts the requested bytes,
 * determines the encoding, and then converts the byte sequence into the desired string.
 */
extern "C" EtsString *EtsArrayBufferToString(ark::ObjectHeader *buffer, EtsString *encodingObj, ets_int start,
                                             ets_int end)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    auto ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);

    auto vb = helpers::encoding::ValidateBuffer(buffer);
    if (std::holds_alternative<helpers::Err<PandaString>>(vb)) {
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8(std::get<helpers::Err<PandaString>>(vb).Message().c_str()));
        return nullptr;
    }

    auto etsObj = reinterpret_cast<EtsObject *>(buffer);
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsArrayBuffer> buf(coro, reinterpret_cast<EtsArrayBuffer *>(etsObj));
    EtsInt byteLength = buf->GetByteLength();

    auto vi = helpers::encoding::ValidateIndices(byteLength, start, end);
    if (std::holds_alternative<helpers::Err<PandaString>>(vi)) {
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8(std::get<helpers::Err<PandaString>>(vi).Message().c_str()));
        return nullptr;
    }

    auto length = static_cast<size_t>(end - start);
    PandaVector<uint8_t> bytes(length);
    if (buf->GetData() != nullptr) {
        std::copy_n(std::next(buf->GetData()->GetData<EtsByte>(), start), length, bytes.data());
    }

    PandaVector<uint8_t> encodingBuf;
    PandaString encoding =
        encodingObj != nullptr ? PandaString(encodingObj->ConvertToStringView(&encodingBuf)) : "utf8";

    PandaString output;
    if (std::find(UTF8_ENCODINGS.begin(), UTF8_ENCODINGS.end(), encoding) != UTF8_ENCODINGS.end()) {
        output = helpers::encoding::ConvertUtf8Encoding(bytes);
    } else if (std::find(UTF16_ENCODINGS.begin(), UTF16_ENCODINGS.end(), encoding) != UTF16_ENCODINGS.end()) {
        output = helpers::encoding::ConvertUtf16Encoding(bytes);
    } else if (std::find(BASE64_ENCODINGS.begin(), BASE64_ENCODINGS.end(), encoding) != BASE64_ENCODINGS.end()) {
        output = helpers::encoding::ConvertBase64Encoding(bytes, encoding);
    } else if (encoding == "hex") {
        output = helpers::encoding::ConvertHexEncoding(bytes);
    } else {
        PandaString errMsg = "Unsupported encoding: " + encoding;
        ThrowException(ctx, coro, ctx.GetIllegalArgumentExceptionClassDescriptor(),
                       utf::CStringAsMutf8(errMsg.c_str()));
        return nullptr;
    }
    return EtsString::CreateFromUtf8(output.data(), output.size());
}

}  // namespace ark::ets::intrinsics
