/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <cstring>
#include "runtime/tooling/bigint_helper.h"

namespace ark::tooling {

constexpr int DECIMAL_RADIX = 10;

std::vector<int> AddDecimalArrays(const std::vector<int> &lhs, const std::vector<int> &rhs)
{
    size_t maxLen = std::max(lhs.size(), rhs.size());
    std::vector<int> result;
    result.reserve(maxLen + 1);
    int carry = 0;
    for (size_t i = 0; i < maxLen; ++i) {
        int l = i < (lhs.size()) ? lhs[lhs.size() - i - 1] : 0;
        int r = i < (rhs.size()) ? rhs[rhs.size() - i - 1] : 0;
        int sum = l + r + carry;
        carry = sum / DECIMAL_RADIX;
        result.push_back(sum % DECIMAL_RADIX);
    }
    if (carry > 0) {
        result.push_back(carry);
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::vector<int> MultiplyDecimalArrayByBigInt(std::vector<int> lhs, const std::vector<int> &rhs)
{
    if (lhs.empty() || rhs.empty()) {
        return {0};
    }
    if (lhs.size() == 1 && lhs[0] == 0) {
        return {0};
    }
    if (rhs.size() == 1 && rhs[0] == 0) {
        return {0};
    }
    size_t len1 = lhs.size();
    size_t len2 = rhs.size();
    std::vector<int> product(len1 + len2, 0);
    for (int i = static_cast<int>(len1) - 1; i >= 0; --i) {
        long mulflag = 0;
        long addflag = 0;
        for (int j = static_cast<int>(len2) - 1; j >= 0; --j) {
            long l = lhs[i] & 0xFFFFFFFF;
            long r = rhs[j] & 0xFFFFFFFF;
            long temp1 = l * r + mulflag;
            mulflag = temp1 / DECIMAL_RADIX;
            temp1 = temp1 % DECIMAL_RADIX;
            long temp2 = product[i + j + 1] + temp1 + addflag;
            product[i + j + 1] = static_cast<int>(temp2 % DECIMAL_RADIX);
            addflag = temp2 / DECIMAL_RADIX;
        }
        product[i] = product[i] + static_cast<int>(mulflag + addflag);
    }
    while (product.size() > 1 && product[0] == 0) {
        product.erase(product.begin());
    }
    return product;
}

std::string DecimalArrayToString(const std::vector<int> &digits)
{
    std::string result;
    result.reserve(digits.size());
    for (int d : digits) {
        result.push_back('0' + d);
    }
    return result;
}

static void Uint32ToDigits(uint32_t word, std::vector<int> &wordDigits)
{
    if (word == 0) {
        wordDigits.clear();
        wordDigits.push_back(0);
        return;
    }
    std::string str = std::to_string(word);
    int len = static_cast<int>(str.size());
    wordDigits.resize(len);
    for (int i = 0; i < len; ++i) {
        wordDigits[i] = str[i] - '0';
    }
}

std::string BigIntBytesToDecimalString(const std::vector<uint32_t> &bytes, int sign)
{
    if (bytes.empty()) {
        return "0n";
    }
    static constexpr int MASK[] = {4, 2, 9, 4, 9, 6, 7, 2, 9, 6};
    static const std::vector<int> MASK_VEC(std::begin(MASK), std::end(MASK));

    std::vector<int> digits;
    std::vector<int> wordDigits;
    size_t length = bytes.size();
    for (size_t i = 0; i < length; ++i) {
        Uint32ToDigits(bytes[length - 1 - i], wordDigits);
        if (i == 0) {
            digits = wordDigits;
        } else {
            digits = MultiplyDecimalArrayByBigInt(digits, MASK_VEC);
            digits = AddDecimalArrays(digits, wordDigits);
        }
    }
    std::string result;
    if (sign < 0) {
        result = "-";
    }
    result += DecimalArrayToString(digits);
    result += "n";
    return result;
}
}  // namespace ark::tooling
