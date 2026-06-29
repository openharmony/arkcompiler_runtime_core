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

#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "libarkbase/os/mutex.h"

#include <cctype>

#ifdef OHOS_PANDA_TRACE_ENABLE
#include "syspara/parameters.h"
#endif

namespace ark::ets::interop::js {

[[noreturn]] void InteropFatal(const char *message)
{
    InteropCtx::Fatal(message);
    UNREACHABLE();
}

[[noreturn]] void InteropFatal(const std::string &message)
{
    InteropCtx::Fatal(message.c_str());
    UNREACHABLE();
}

[[noreturn]] void InteropFatal(const char *message, napi_status status)
{
    InteropCtx::Fatal(std::string(message) + " status=" + std::to_string(status));
    UNREACHABLE();
}

#if defined(OHOS_PANDA_TRACE_ENABLE)
void InteropTrace(const char *func, const char *file, int line)
{
    static std::once_flag initFlag;
    static bool isInteropTraceEnabled = false;

    std::call_once(initFlag,
                   []() { isInteropTraceEnabled = OHOS::system::GetBoolParameter(INTEROP_TRACE_ENABLE, false); });

    if (isInteropTraceEnabled) {
#ifndef NDEBUG
        INTEROP_LOG(INFO) << "interoptrace: " << func << ":" << file << ":" << line;
#else
        INTEROP_LOG(INFO) << "interoptrace: " << func;
#endif
    }
}
#endif

std::pair<SmallVector<uint64_t, 4U>, int> GetBigInt(napi_env env, napi_value jsVal)
{
    size_t wordCount;
    NAPI_ASSERT_OK(napi_get_value_bigint_words(env, jsVal, nullptr, &wordCount, nullptr));

    int signBit;
    SmallVector<uint64_t, 4U> words;
    if (wordCount == 0) {
        bool lossless;
        signBit = 0;
        words.resize(1);
        NAPI_ASSERT_OK(napi_get_value_bigint_uint64(env, jsVal, &words[0], &lossless));
    } else {
        words.resize(wordCount);
        NAPI_ASSERT_OK(napi_get_value_bigint_words(env, jsVal, &signBit, &wordCount, words.data()));
    }

    return {words, signBit};
}

SmallVector<uint64_t, 4U> ConvertBigIntArrayFromEtsToJs(const std::vector<uint32_t> &etsArray)
{
    ASSERT(BIT_64 % BIGINT_BITS_NUM == 0);
    // BigInt in ArkTS is stored in EtsInt array. Put these bits into int64_t array
    size_t jsArraySize = etsArray.size() / 2 + etsArray.size() % 2;
    SmallVector<uint64_t, 4U> jsArray;
    jsArray.resize(jsArraySize, 0);

    size_t curJSArrayElemPos = 0;
    size_t curBitPos = 0;
    for (auto etsElem : etsArray) {
        jsArray[curJSArrayElemPos] |= static_cast<uint64_t>(etsElem) << curBitPos;
        curBitPos = curBitPos + BIGINT_BITS_NUM;
        if (curBitPos == BIT_64) {
            curBitPos = 0;
            ++curJSArrayElemPos;
        }
    }

    return jsArray;
}

static inline size_t GetBigIntEtsArraySize(size_t jsArraySize, uint64_t lastElem)
{
    if (jsArraySize == 1 && lastElem == 0) {
        return 0;
    }

    size_t etsSize = jsArraySize * 2 - 1;

    if ((lastElem >> BIGINT_BITS_NUM) > 0) {
        ++etsSize;
    }

    return etsSize;
}

std::vector<EtsInt> ConvertBigIntArrayFromJsToEts(SmallVector<uint64_t, 4U> &jsArray)
{
    size_t etsArraySize = GetBigIntEtsArraySize(jsArray.size(), jsArray.back());
    std::vector<EtsInt> etsArray(etsArraySize, 0);

    size_t curJSArrayElemPos = 0;
    size_t curBitPos = 0;
    for (size_t i = 0; i < etsArraySize; ++i) {
        etsArray[i] = static_cast<uint32_t>(jsArray[curJSArrayElemPos] >> curBitPos);

        curBitPos = curBitPos + BIGINT_BITS_NUM;
        if (curBitPos == BIT_64) {
            curBitPos = 0;
            ++curJSArrayElemPos;
        }
    }

    return etsArray;
}

void ThrowNoInteropContextException()
{
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    auto ctx = thread->GetVM()->GetLanguageContext();
    auto descriptor = PlatformTypes()->interopNoInteropContextError->GetDescriptor();
    PandaString msg = "Interop call may be done only from _main_ or exclusive worker";
    ThrowException(ctx, thread, utf::CStringAsMutf8(descriptor), utf::CStringAsMutf8(msg.c_str()));
}

void ThrowJSErrorNotAssignable(napi_env env, const EtsClass *fromKlass, EtsClass *toKlass)
{
    const char *from = fromKlass->GetDescriptor();
    const char *to = toKlass->GetDescriptor();
    InteropCtx::ThrowJSTypeError(env, std::string(from) + " is not assignable to " + to);
}

static bool GetPropertyStatusHandling([[maybe_unused]] napi_env env, napi_status rc)
{
#if !defined(PANDA_TARGET_OHOS) && !defined(PANDA_JS_ETS_HYBRID_MODE)
    if (UNLIKELY(rc == napi_object_expected || NapiThrownGeneric(rc))) {
        ASSERT(NapiIsExceptionPending(env));
        return false;
    }
#else
    if (UNLIKELY(rc == napi_object_expected && !NapiIsExceptionPending(env))) {
        InteropCtx::ThrowJSTypeError(env, "Cannot convert undefined or null to object");
        return false;
    }

    if (UNLIKELY(rc == napi_pending_exception || NapiThrownGeneric(rc))) {
        ASSERT(NapiIsExceptionPending(env));
        return false;
    }
#endif
    INTEROP_FATAL_IF(rc != napi_ok);
    return true;
}

bool NapiGetProperty(napi_env env, napi_value object, napi_value key, napi_value *result)
{
    napi_status rc = napi_get_property(env, object, key, result);
    return GetPropertyStatusHandling(env, rc);
}

bool NapiGetNamedProperty(napi_env env, napi_value object, const char *utf8name, napi_value *result)
{
    napi_status rc = napi_get_named_property(env, object, utf8name, result);
    return GetPropertyStatusHandling(env, rc);
}

// ---- BigInt JSON preprocessing helpers ----

// Number.MAX_SAFE_INTEGER = 2^53 - 1 = 9007199254740991 (16 decimal digits)
constexpr size_t MAX_SAFE_INTEGER_DIGITS = 16U;
constexpr const char *MAX_SAFE_INTEGER_STR = "9007199254740991";

bool IsExceedingSafeInteger(const PandaString &numStr)
{
    // numStr contains only digits (no sign, no leading zeros after '0')
    if (numStr.length() > MAX_SAFE_INTEGER_DIGITS) {
        return true;
    }
    if (numStr.length() < MAX_SAFE_INTEGER_DIGITS) {
        return false;
    }
    return numStr > MAX_SAFE_INTEGER_STR;
}

namespace {

// MUTF-8 encodes U+0000 as overlong two-byte sequence
constexpr unsigned char MUTF8_NULL_HIGH = 0xC0U;
constexpr unsigned char MUTF8_NULL_LOW = 0x80U;
constexpr size_t MUTF8_NULL_LEN = 2U;

// JSON control characters U+0000..U+001F MUST be escaped (RFC 8259 §7)
constexpr unsigned char JSON_CONTROL_MAX = 0x1FU;
constexpr unsigned char HEX_SHIFT = 4U;
constexpr unsigned char HEX_MASK = 0x0FU;

constexpr std::string_view CONTROL_ESCAPE_PREFIX = "\\u00";
constexpr std::string_view NULL_ESCAPE = "\\u0000";

// Reserve headroom for quoting large integer literals in JSON string
constexpr size_t RESERVE_HEADROOM = 32U;

constexpr std::array<char, 16U> HEX_DIGITS = {'0', '1', '2', '3', '4', '5', '6', '7',
                                              '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

// Consume consecutive ASCII digits starting at |i|, advancing past them.
void ConsumeDigits(const PandaString &jsonStr, size_t &i)
{
    while (i < jsonStr.size() && std::isdigit(static_cast<unsigned char>(jsonStr[i])) != 0) {
        i++;
    }
}

// Copy a JSON string literal (including quotes), handling \" escape sequences
// and escaping control characters (U+0000..U+001F) that ETS char serialization
// may emit as raw bytes — invalid in JSON but repairable here as \u00XX.
// Advances |i| past the closing quote on return.
void CopyJsonStringLiteral(const PandaString &jsonStr, size_t &i, PandaString &result)
{
    result += '"';
    i++;  // skip opening quote
    while (i < jsonStr.size()) {
        auto ch = static_cast<unsigned char>(jsonStr[i]);
        if (ch == MUTF8_NULL_HIGH && i + 1 < jsonStr.size() &&
            static_cast<unsigned char>(jsonStr[i + 1]) == MUTF8_NULL_LOW) {
            result += NULL_ESCAPE;
            i += MUTF8_NULL_LEN;
            continue;
        }
        if (ch <= JSON_CONTROL_MAX) {
            result += CONTROL_ESCAPE_PREFIX;
            result += HEX_DIGITS[ch >> HEX_SHIFT];
            result += HEX_DIGITS[ch & HEX_MASK];
            i++;
            continue;
        }
        result += jsonStr[i];
        if (ch == '\\' && i + 1 < jsonStr.size()) {
            result += jsonStr[++i];  // escaped char
        } else if (ch == '"') {
            i++;
            return;
        }
        i++;
    }
}

// Consume fractional part and/or exponent of a JSON number.
// |i| must point to '.' or 'e'/'E' on entry; advances past the suffix.
void ConsumeFloatSuffix(const PandaString &jsonStr, size_t &i)
{
    if (jsonStr[i] == '.') {
        ConsumeDigits(jsonStr, ++i);
    }
    if (i < jsonStr.size() && (jsonStr[i] == 'e' || jsonStr[i] == 'E')) {
        i++;
        if (i < jsonStr.size() && (jsonStr[i] == '+' || jsonStr[i] == '-')) {
            i++;
        }
        ConsumeDigits(jsonStr, i);
    }
}

// Append an integer number token scanned from [start, end) to |result|,
// wrapping it in quotes if it exceeds Number.MAX_SAFE_INTEGER.
void AppendNumberToken(const PandaString &jsonStr, size_t start, size_t end, bool isNegative, PandaString &result)
{
    PandaString numStr = jsonStr.substr(start, end - start);
    PandaString absStr = isNegative ? numStr.substr(1) : numStr;
    if (UNLIKELY(IsExceedingSafeInteger(absStr))) {
        result += '"';
        result += numStr;
        result += '"';
        return;
    }
    result += numStr;
}

// Return true if |i| points to the fractional or exponent part of a JSON number.
bool IsFloatSuffix(const PandaString &jsonStr, size_t i)
{
    return i < jsonStr.size() && (jsonStr[i] == '.' || jsonStr[i] == 'e' || jsonStr[i] == 'E');
}

// If |i| points to the start of a JSON number token ('-' or digit), scan it,
// optionally preprocess large integers, append to |result|, and return true.
// Otherwise return false and leave |i| unchanged.
bool TryHandleNumber(const PandaString &jsonStr, size_t &i, PandaString &result)
{
    auto ch = static_cast<unsigned char>(jsonStr[i]);
    if (ch != '-' && std::isdigit(ch) == 0U) {
        return false;
    }
    size_t start = i;
    bool isNegative = (ch == '-');
    if (isNegative) {
        i++;
    }
    ConsumeDigits(jsonStr, i);
    if (IsFloatSuffix(jsonStr, i)) {
        ConsumeFloatSuffix(jsonStr, i);
        result += jsonStr.substr(start, i - start);
        return true;
    }
    AppendNumberToken(jsonStr, start, i, isNegative, result);
    return true;
}

}  // anonymous namespace

PandaString PreprocessBigIntInJson(const PandaString &jsonStr)
{
    PandaString result;
    result.reserve(jsonStr.size() + RESERVE_HEADROOM);

    for (size_t i = 0U; i < jsonStr.size();) {
        char ch = jsonStr[i];

        if (ch == '"') {
            CopyJsonStringLiteral(jsonStr, i, result);
            continue;
        }
        if (TryHandleNumber(jsonStr, i, result)) {
            continue;
        }
        result += ch;
        i++;
    }

    return result;
}

}  // namespace ark::ets::interop::js
