/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <cstdint>
#include <iostream>
#include "ani_textdecoder.h"
#include "ani.h"
#include "securec.h"
#include "ohos/init_data.h"

namespace ark::ets::sdk::util {
UConverter *CreateConverter(std::string &encStr, UErrorCode &codeflag)
{
    UConverter *conv = ucnv_open(encStr.c_str(), &codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Unable to create a UConverter object %{public}s", u_errorName(codeflag));
        return nullptr;
    }
    ucnv_setFromUCallBack(conv, UCNV_FROM_U_CALLBACK_SUBSTITUTE, nullptr, nullptr, nullptr, &codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Unable to set the from Unicode callback function");
        ucnv_close(conv);
        return nullptr;
    }

    ucnv_setToUCallBack(conv, UCNV_TO_U_CALLBACK_SUBSTITUTE, nullptr, nullptr, nullptr, &codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Unable to set the to Unicode callback function");
        ucnv_close(conv);
        return nullptr;
    }
    return conv;
}

TextDecoder::TextDecoder(std::string &buff, uint32_t flags) : encStr_(buff), tranTool_(nullptr, nullptr)
{
    SetHwIcuDirectory();
    label_ |= flags;
    bool fatal =
        (flags & static_cast<uint32_t>(ConverterFlags::FATAL_FLG)) == static_cast<uint32_t>(ConverterFlags::FATAL_FLG);
    UErrorCode codeflag = U_ZERO_ERROR;
    UConverter *conv = CreateConverter(encStr_, codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: ucnv_open failed !");
        return;
    }
    if (fatal) {
        codeflag = U_ZERO_ERROR;
        ucnv_setToUCallBack(conv, UCNV_TO_U_CALLBACK_STOP, nullptr, nullptr, nullptr, &codeflag);
    }
    TransformToolPointer tempTranTool(conv, ConverterClose);
    tranTool_ = std::move(tempTranTool);
}

ani_string TextDecoder::GetResultStr(ani_env *env, const UChar *arrDat, size_t length)
{
    ani_string resultStr = nullptr;
    ani_status status = env->String_NewUTF16(reinterpret_cast<const uint16_t *>(arrDat), length, &resultStr);
    if (status != ANI_OK) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: String creation failed");
    }
    return resultStr;
}

ani_object TextDecoder::CreateThrowErrorObject(ani_env *env, const std::string &message)
{
    ani_string errString;
    if (env->String_NewUTF8(message.c_str(), message.size(), &errString) != ANI_OK) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Data allocation failed");
        return nullptr;
    }
    static const char *className = "L@ohos/util/util/BusinessError;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Not found %{public}s", className);
        return nullptr;
    }

    ani_method errorCtor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "Lstd/core/String;:V", &errorCtor)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Class_FindMethod <ctor> Failed");
        return nullptr;
    }

    ani_object errorObj;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (ANI_OK != env->Object_New(cls, errorCtor, &errorObj, errString)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Object_New Array Faild");
        return nullptr;
    }
    return errorObj;
}

ani_string TextDecoder::DecodeToString(ani_env *env, const char *source, int32_t byteOffset, uint32_t length,
                                       bool iflag)
{
    const size_t minBytes = GetMinByteSize();
    const size_t ucharCount = minBytes * length;
    if (ucharCount == 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("TextDecoder:: Invalid buffer size");
        return nullptr;
    }
    thread_local std::vector<UChar> decodeBuffer;
    if (decodeBuffer.size() < ucharCount + 1) {
        decodeBuffer.resize(ucharCount + 1);
    }
    UChar *target = decodeBuffer.data();
    const UChar *const targetStart = target;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const UChar *const targetLimit = target + ucharCount;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    source += byteOffset;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const char *sourceLimit = source + length;
    const UBool flush = !iflag ? TRUE : FALSE;
    UErrorCode codeFlag = U_ZERO_ERROR;
    ucnv_toUnicode(GetConverterPtr(), &target, targetLimit, &source, sourceLimit, nullptr, flush, &codeFlag);
    if (codeFlag != U_ZERO_ERROR) {
        std::string message = "Parameter error. Please check if the decode data matches the encoding format.";
        if (auto errorObj = CreateThrowErrorObject(env, message)) {
            env->ThrowError(static_cast<ani_error>(errorObj));
        }
        return nullptr;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    size_t resultLen = target - targetStart;
    bool omitInitialBom = false;
    SetIgnoreBOM(targetStart, resultLen, omitInitialBom);
    const UChar *resultStart = targetStart;
    if (omitInitialBom && resultLen > 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        resultStart++;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        resultLen--;
    }
    ani_string result = GetResultStr(env, resultStart, resultLen);
    if (flush != 0) {
        label_ &= ~static_cast<uint32_t>(ConverterFlags::BOM_SEEN_FLG);
        Reset();
    }
    return result;
}

size_t TextDecoder::GetMinByteSize() const
{
    if (tranTool_ == nullptr) {
        return 0;
    }
    auto res = static_cast<size_t>(ucnv_getMinCharSize(tranTool_.get()));
    return res;
}

void TextDecoder::Reset() const
{
    if (tranTool_ == nullptr) {
        return;
    }
    ucnv_reset(tranTool_.get());
}

void TextDecoder::FreedMemory(UChar *&pData)
{
    delete[] pData;
    pData = nullptr;
}

void TextDecoder::SetIgnoreBOM(const UChar *arr, size_t resultLen, bool &bomFlag)
{
    switch (ucnv_getType(GetConverterPtr())) {
        case UCNV_UTF8:
        case UCNV_UTF16_BigEndian:
        case UCNV_UTF16_LittleEndian:
            label_ |= static_cast<uint32_t>(ConverterFlags::UNICODE_FLG);
            break;
        default:
            break;
    }
    if (resultLen > 0 && IsUnicode() && IsIgnoreBom()) {
        bomFlag = (*arr == static_cast<uint16_t>(ConverterFlags::BOM_FLAG));
    }
    label_ |= static_cast<uint32_t>(ConverterFlags::BOM_SEEN_FLG);
}
}  // namespace ark::ets::sdk::util
