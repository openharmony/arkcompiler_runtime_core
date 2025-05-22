/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: multiplicative operator with other operators
---*/

function main(): int {
    let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255  = 255

    // division
    res = t8 / t3 * t2
    if (res != 4) return 1;

    // modulo
    res = t8 % t3 * t2
    if (res != 4) return 1;

    // addition
    res = t2 + t4 * t3
    if (res != 14) return 1;

    // subtraction
    res = t2 - t3 * t4
    if (res != -10) return 1;

    // cast
    res = t2 * t255 as byte
    if (res != -2) return 1;

    res = t255 as byte * t3
    if (res != -3) return 1;

    // shifts
    res = t4 << t3 * t2
    if (res != 256) return 1;

    res = - t255 >> t2 * t3
    if (res != -4) return 1;

    res = - t255 >>> t2 * t3
    if (res != 67108860) return 1;

    // relational
    if (t255 < t8 * t8) return 1;
    if (t255 <= t2 * t8) return 1;
    if (t2 > t8 * t4) return 1;
    if (t4 >= t8 * t4) return 1;
    if (t2 == t2 * t4) return 1;
    if (t8 != t4 * t2) return 1;

    // bitwise
    res = t255 & t4 * t8
    if (res != 32) return 1;

    res = t3 | t4 * t2
    if (res != 11) return 1;

    res = t255 ^ t8 * t2
    if (res != 239) return 1;

    // ternary
    res = t3 <= t4 ? t4 : t3 * t4
    if (res != 4) return 1;

    res = t4 * t2 > t4 ? t4 : t3
    if (res != 4) return 1;

    // assignments
    res = t2 += t4 * t3
    if (res != 14) return 1;
    t2 = 2

    res = t2 -= t4 * t3
    if (res != -10) return 1;
    t2 = 2

    res = t8 /= t2 * t3
    if (res != 1) return 1;
    t8 = 8

    res = t8 %= t2 * t3
    if (res != 2) return 1;
    t8 = 8

    res = t255 &= t2 * t3
    if (res != 6) return 1;
    t255 = 255

    res = t2 |= t3 * t4
    if (res != 14) return 1;
    t2 = 2

    res = t255 ^= t4 * t8
    if (res != 223) return 1;
    t255 = 255

    res = t4 <<= t3 * t2
    if (res != 256) return 1;
    t4 = 4

    res = t255 >>= t2 * t3
    if (res != 3) return 1;
    t255 = 255

    res = t255 >>>= t3 * t2
    if (res != 3) return 1;
    t255 = 255

    return 0;
}
