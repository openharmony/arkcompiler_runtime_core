/*
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

/*---
desc: bitwise operators with other operators
---*/

function main(): int {
    let res: int, t2 = 2, t3 = 3, t4 = 4, t7 = 7, t8 = 8, t255  = 255

    // bitwise
    res = t3 | t4 & t8
    if (res != 3) return 1;

    res = t3 ^ t4 & t2
    if (res != 3) return 1;

    res = t3 | t4 ^ t2
    if (res != 7) return 1;

    // ternary
    res = t3 <= t4 ? t7 : t2 & t3
    if (res != 7) return 1;

    res = t3 <= t4 ? t7 : t2 ^ t3
    if (res != 7) return 1;

    res = t3 <= t4 ? t7 : t2 | t255
    if (res != 7) return 1;

    // assignments
    res = t2 += t7 & t3
    if (res != 5) return 1; else t2 = 2;

    res = t2 += t7 ^ t3
    if (res != 6) return 1; else t2 = 2;

    res = t2 += t7 | t3
    if (res != 9) return 1; else t2 = 2;

    res = t2 -= t7 & t3
    if (res != -1) return 1; else t2 = 2;

    res = t2 -= t7 ^ t3
    if (res != -2) return 1; else t2 = 2;

    res = t2 -= t7 | t255
    if (res != -253) return 1; else t2 = 2;

    res = t2 *= t7 & t3
    if (res != 6) return 1; else t2 = 2;

    res = t2 *= t7 ^ t3
    if (res != 8) return 1; else t2 = 2;

    res = t2 *= t7 | t255
    if (res != 510) return 1; else t2 = 2;

    res = t255 /= t7 & t3
    if (res != 85) return 1; else t255 = 255;

    res = t255 /= t7 ^ t3
    if (res != 63) return 1; else t255 = 255;

    res = t255 /= t7 | t255
    if (res != 1) return 1; else t255 = 255;

    res = t255 %= t7 & t3
    if (res != 0) return 1; else t255 = 255;

    res = t255 %= t7 ^ t3
    if (res != 3) return 1; else t255 = 255;

    res = t255 %= t7 | t255
    if (res != 0) return 1; else t255 = 255;

    res = t255 &= t7 & t3
    if (res != 3) return 1; else t255 = 255;

    res = t255 &= t7 ^ t255
    if (res != 248) return 1; else t255 = 255;

    res = t255 &= t7 | t255
    if (res != 255) return 1; else t255 = 255;

    res = t255 ^= t7 & t3
    if (res != 252) return 1; else t255 = 255;

    res = t255 ^= t7 ^ t255
    if (res != 7) return 1; else t255 = 255;

    res = t255 ^= t7 | t255
    if (res != 0) return 1; else t255 = 255;

    res = t255 |= t7 & t3
    if (res != 255) return 1; else t255 = 255;

    res = t255 |= t7 ^ t255
    if (res != 255) return 1; else t255 = 255;

    res = t255 |= t7 | t255
    if (res != 255) return 1; else t255 = 255;

    res = t255 <<= t7 & t3
    if (res != 2040) return 1; else t255 = 255;

    res = t255 <<= t7 ^ t255
    if (res != -16777216) return 1; else t255 = 255;

    res = t255 <<= t7 | t255
    if (res != -2147483648) return 1; else t255 = 255;

    res = t255 >>= t7 & t3
    if (res != 31) return 1; else t255 = 255;

    res = t255 >>= t2 ^ t7
    if (res != 7) return 1; else t255 = 255;

    res = t255 >>= t7 | t7
    if (res != 1) return 1; else t255 = 255;

    res = t255 >>>= t7 & t3
    if (res != 31) return 1; else t255 = 255;

    res = t255 >>>= t2 ^ t7
    if (res != 7) return 1; else t255 = 255;

    res = t255 >>>= t7 | t7
    if (res != 1) return 1; else t255 = 255;

    return 0;
}
