/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_BASE64_H
#define PANDA_TOOLING_INSPECTOR_BASE64_H

#include <string>

namespace ark::tooling::inspector {
// TODO(dslynko): borrowed from arkcompiler_toolchain, deside the future fate of this class.
class PtBase64 {
public:
    PtBase64() = default;
    ~PtBase64() = default;

    static std::size_t constexpr DecodedSize(std::size_t n)
    {
        return n / ENCODED_GROUP_BYTES * UNENCODED_GROUP_BYTES;
    }

    static std::size_t constexpr EncodedSize(std::size_t n)
    {
        return ENCODED_GROUP_BYTES * ((n + 2) / UNENCODED_GROUP_BYTES);  // 2: byte filling
    }

    static std::pair<std::size_t, bool> Decode(void *output, const char *input, std::size_t len)
    {
        if (len < ENCODED_GROUP_BYTES || len % ENCODED_GROUP_BYTES != 0) {
            return {0, false};
        }

        auto *src = reinterpret_cast<unsigned char const *>(input);
        char *dest = static_cast<char *>(output);
        char base64Chars[ENCODED_GROUP_BYTES];
        int8_t i = 0;
        while (len-- && *src != '=') {
            auto v = DECODE_TABLE[*src];
            if (v == INVAILD_VALUE) {
                break;
            }
            base64Chars[i++] = v;

            if (i == ENCODED_GROUP_BYTES) {
                *dest++ = (base64Chars[0] << 2) | ((base64Chars[1] & 0x30) >> 4);  // 2: shift 2bits, 4: shift 4bits
                *dest++ = (base64Chars[1] << 4) | ((base64Chars[2] & 0x3c) >> 2);  // 2: shift 2bits, 4: shift 4bits
                *dest++ =
                    (base64Chars[2] << 6) | base64Chars[3];  // 6: shift 6bits, 2: the second char, 3: the third char
                i = 0;
            }
            src++;
        }

        if (i != 0) {
            char tmp[UNENCODED_GROUP_BYTES];
            tmp[0] = (base64Chars[0] << 2) | ((base64Chars[1] & 0x30) >> 4);  // 2: shift 2bits, 4: shift 4bits
            tmp[1] = (base64Chars[1] << 4) | ((base64Chars[2] & 0x3c) >> 2);  // 2: shift 2bits, 4: shift 4bits
            tmp[2] = (base64Chars[2] << 6) | base64Chars[3];  // 6: shift 6bits, 2: the second char, 3: the third char
            for (int8_t j = 0; j < i - 1; j++) {
                *dest++ = tmp[j];
            }
        }

        len++;  // get the remaining length and also avoid underflow
        size_t decodedLen = dest - static_cast<char *>(output);
        bool decodedDone = false;
        // 0: decodedDone, 1: =, 2: ==
        if (len == 0 || (len == 1 && *src == '=') || (len == 2 && *src == '=' && *(src + 1) == '=')) {
            decodedDone = true;
        }

        return {decodedLen, decodedDone};
    }

    static size_t Encode(char *output, const void *input, std::size_t len)
    {
        if (len == 0) {
            return 0;
        }

        auto *src = static_cast<const unsigned char *>(input);
        char *dest = output;
        for (auto n = len / UNENCODED_GROUP_BYTES; n--;) {
            *dest++ = ENCODE_TABLE[src[0] >> 2];                             // 2: shift 2bits
            *dest++ = ENCODE_TABLE[((src[0] & 0x03) << 4) | (src[1] >> 4)];  // 4: shift 4bits
            *dest++ = ENCODE_TABLE[((src[1] & 0x0f) << 2) | (src[2] >> 6)];  // 2: shift 2bits, 6: shift 6bits
            *dest++ = ENCODE_TABLE[src[2] & 0x3f];                           // 2: the second char

            src += UNENCODED_GROUP_BYTES;
        }

        int8_t remainder = static_cast<int8_t>(len % UNENCODED_GROUP_BYTES);
        int8_t paddingCnt = (UNENCODED_GROUP_BYTES - remainder) % UNENCODED_GROUP_BYTES;
        switch (paddingCnt) {
            case 1:
                *dest++ = ENCODE_TABLE[src[0] >> 2];                             // 2: shift 2bits
                *dest++ = ENCODE_TABLE[((src[0] & 0x03) << 4) | (src[1] >> 4)];  // 4: shift 4bits
                *dest++ = ENCODE_TABLE[((src[1] & 0x0f) << 2)];                  // 2: shift 2bits
                *dest++ = '=';
                break;
            case 2:                                              // 2: "=="
                *dest++ = ENCODE_TABLE[src[0] >> 2];             // 2: shift 2bits
                *dest++ = ENCODE_TABLE[((src[0] & 0x03) << 4)];  // 4: shift 4bits
                *dest++ = '=';
                *dest++ = '=';
                break;
            default:
                break;
        }
        return dest - output;
    }

private:
    static constexpr uint8_t UNENCODED_GROUP_BYTES = 3;
    static constexpr uint8_t ENCODED_GROUP_BYTES = 4;
    static constexpr uint8_t INVAILD_VALUE = 255;

    static constexpr const unsigned char DECODE_TABLE[] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,
        255, 255, 255, 63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255, 255, 255, 255, 255, 255, 0,
        1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,
        23,  24,  25,  255, 255, 255, 255, 255, 255, 26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
        39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};

    static constexpr const char ENCODE_TABLE[] = {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/"};
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_BASE64_H
