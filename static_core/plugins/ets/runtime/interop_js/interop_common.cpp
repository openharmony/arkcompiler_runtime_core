/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

void InteropTrace(const char *func, const char *file, int line)
{
    INTEROP_LOG(DEBUG) << "trace: " << func << ":" << file << ":" << line;
}

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

static inline size_t GetBigIntJSArraySize(size_t etsArraySize, uint64_t lastElem)
{
    size_t bitsNum = (etsArraySize - 1) * BIGINT_BITS_NUM;
    size_t tailBits = 0;

    while (lastElem > 0) {
        lastElem >>= 1U;
        ++tailBits;
    }
    bitsNum += tailBits;
    return (bitsNum + BIT_64 - 1) / BIT_64;
}

SmallVector<uint64_t, 4U> ConvertBigIntArrayFromEtsToJs(const std::vector<uint32_t> &etsArray)
{
    // BigInt in ArkTS is stored in EtsInt array but only 30 bits have meaning. Put these bits into int64_t array
    size_t jsArraySize = GetBigIntJSArraySize(etsArray.size(), etsArray.back());
    SmallVector<uint64_t, 4U> jsArray;
    jsArray.resize(jsArraySize, 0);

    size_t curJSArrayElemPos = 0;
    size_t curBitPos = 0;
    for (auto etsElem : etsArray) {
        jsArray[curJSArrayElemPos] |= static_cast<uint64_t>(etsElem) << curBitPos;
        if (curBitPos + BIGINT_BITS_NUM <= BIT_64) {
            curBitPos = curBitPos + BIGINT_BITS_NUM;
        } else {
            if (++curJSArrayElemPos != jsArray.size()) {
                size_t leftoverBitNums = BIT_64 - curBitPos;
                jsArray[curJSArrayElemPos] |= static_cast<uint32_t>(etsElem) >> leftoverBitNums;
                curBitPos = BIGINT_BITS_NUM - leftoverBitNums;
            }
        }
    }

    return jsArray;
}

static inline size_t GetBigIntEtsArraySize(size_t jsArraySize, uint64_t lastElem)
{
    size_t bitsNum = (jsArraySize - 1) * BIT_64;
    size_t tailBits = 0;

    while (lastElem > 0) {
        lastElem >>= 1U;
        ++tailBits;
    }
    bitsNum += tailBits;
    return (bitsNum + BIGINT_BITS_NUM - 1) / BIGINT_BITS_NUM;
}

static inline void ConvertToTwosComplement(std::vector<EtsInt> &array)
{
    uint8_t extraBit = 1;
    for (auto &elem : array) {
        elem = (~static_cast<uint32_t>(elem) + extraBit) & BIGINT_BITMASK_30;
        if (elem != 0) {
            extraBit = 0;
        }
    }
}

std::vector<EtsInt> ConvertBigIntArrayFromJsToEts(SmallVector<uint64_t, 4U> &jsArray, int signBit)
{
    size_t etsArraySize = GetBigIntEtsArraySize(jsArray.size(), jsArray.back());
    std::vector<EtsInt> etsArray(etsArraySize + 1, 0);

    size_t curJSArrayElemPos = 0;
    size_t curBitPos = 0;
    for (size_t i = 0; i < etsArraySize; ++i) {
        auto etsElem = static_cast<uint32_t>(jsArray[curJSArrayElemPos] >> curBitPos);

        if (curBitPos + BIGINT_BITS_NUM <= BIT_64) {
            curBitPos = curBitPos + BIGINT_BITS_NUM;
        } else {
            if (++curJSArrayElemPos != jsArray.size()) {
                size_t leftoverBitNums = BIT_64 - curBitPos;
                etsElem |= static_cast<uint32_t>(jsArray[curJSArrayElemPos]) << leftoverBitNums;
                curBitPos = BIGINT_BITS_NUM - leftoverBitNums;
            }
        }

        etsArray[i] = etsElem & BIGINT_BITMASK_30;
    }

    if (signBit == 1) {
        ConvertToTwosComplement(etsArray);
    }
    etsArray.back() = (signBit == 1) ? BIGINT_BITMASK_30 : 0;

    return etsArray;
}

}  // namespace ark::ets::interop::js

namespace ark::ets::ts2ets::GlobalCtx {  // NOLINT(readability-identifier-naming)

void Init()
{
    interop::js::InteropCtx::Init(EtsCoroutine::GetCurrent(), nullptr);
}

}  // namespace ark::ets::ts2ets::GlobalCtx
