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

#include "ani_stringdecoder.h"
#include <codecvt>
#include <locale>
#include "tools/format_logger.h"
#include "unicode/unistr.h"

namespace ark::ets::sdk::util {
UConverter *UtilHelper::CreateConverter(const std::string &encStr, UErrorCode &codeflag)
{
    UConverter *conv = ucnv_open(encStr.c_str(), &codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("Unable to create a UConverter object: %s\n", u_errorName(codeflag));
        return nullptr;
    }
    ucnv_setFromUCallBack(conv, UCNV_FROM_U_CALLBACK_SUBSTITUTE, nullptr, nullptr, nullptr, &codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("Unable to set the from Unicode callback function");
        ucnv_close(conv);
        return nullptr;
    }

    ucnv_setToUCallBack(conv, UCNV_TO_U_CALLBACK_SUBSTITUTE, nullptr, nullptr, nullptr, &codeflag);
    if (U_FAILURE(codeflag) != 0) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("Unable to set the to Unicode callback function");
        ucnv_close(conv);
        return nullptr;
    }
    return conv;
}

std::string UtilHelper::ConvertToString(UChar *uchar)
{
    std::u16string tempStr16(uchar);
    std::string tepStr = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.to_bytes(tempStr16);
    return tepStr;
}

StringDecoder::StringDecoder(const std::string &encoding)
{
    UErrorCode codeflag = U_ZERO_ERROR;
    conv_ = UtilHelper::CreateConverter(encoding, codeflag);
}

ani_string StringDecoder::Write(ani_env *env, const char *source, int32_t byteOffset, uint32_t length, UBool flush)
{
    size_t limit = static_cast<size_t>(ucnv_getMinCharSize(conv_)) * length;
    size_t len = limit * sizeof(UChar);
    UChar *arr = nullptr;
    if (limit > 0) {
        arr = new UChar[limit + 1] {0};
    } else {
        ThrowError(env, "Error obtaining minimum number of input bytes");
        return nullptr;
    }
    UChar *target = arr;
    UErrorCode codeFlag = U_ZERO_ERROR;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    source += byteOffset;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ucnv_toUnicode(conv_, &target, target + len, &source, source + length, nullptr, flush, &codeFlag);
    if (U_FAILURE(codeFlag) != 0) {
        FreedMemory(arr);
        std::string err = "decoder error, ";
        err += u_errorName(codeFlag);
        ThrowError(env, err);
        return nullptr;
    }
    pendingLen_ = ucnv_toUCountPending(conv_, &codeFlag);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    pend_ = source + length - pendingLen_;

    ani_string resultStr {};
    size_t resultLen = target - arr;
    if (env->String_NewUTF16(reinterpret_cast<uint16_t *>(arr), resultLen, &resultStr) != ANI_OK) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("StringDecoder:: create string error!");
        FreedMemory(arr);
        return nullptr;
    }
    FreedMemory(arr);
    return resultStr;
}

ani_string StringDecoder::End(ani_env *env, const char *source, int32_t byteOffset, int32_t length)
{
    return Write(env, source, byteOffset, length, 1);
}

ani_string StringDecoder::End(ani_env *env)
{
    ani_string resultStr {};
    if (pendingLen_ == 0) {
        env->String_NewUTF8("", 0, &resultStr);
        return resultStr;
    }
    UErrorCode errorCode = U_ZERO_ERROR;
    auto *outputBuffer = new UChar[pendingLen_];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    UChar *targetEnd = outputBuffer + pendingLen_;
    UChar *target = outputBuffer;
    const char *src = pend_;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const char *sourceEnd = pend_ + pendingLen_;
    UBool flush = 1;
    ucnv_toUnicode(conv_, &target, targetEnd, &src, sourceEnd, nullptr, flush, &errorCode);
    if (U_FAILURE(errorCode) != 0) {
        std::string err = "decoder error, ";
        err += u_errorName(errorCode);
        ThrowError(env, err);
        return nullptr;
    }
    std::string tepStr = UtilHelper::ConvertToString(target);
    env->String_NewUTF8(tepStr.c_str(), tepStr.size(), &resultStr);
    delete[] outputBuffer;
    return resultStr;
}

void StringDecoder::FreedMemory(UChar *&pData)
{
    if (pData != nullptr) {
        delete[] pData;
        pData = nullptr;
    }
}

ani_object StringDecoder::ThrowError(ani_env *env, const std::string &message)
{
    ani_string errString;
    env->String_NewUTF8(message.c_str(), message.size(), &errString);
    static const char *className = "L@ohos/util/util/BusinessError;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("StringDecoder:: Not found %{public}s", className);
        return nullptr;
    }

    ani_method errorCtor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", "Lstd/core/String;:V", &errorCtor)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("StringDecoder:: Class_FindMethod <ctor> Failed");
        return nullptr;
    }

    ani_object errorObj;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (ANI_OK != env->Object_New(cls, errorCtor, &errorObj, errString)) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        LOG_ERROR_SDK("StringDecoder:: Object_New Error Faild");
    }
    return errorObj;
}
}  // namespace ark::ets::sdk::util