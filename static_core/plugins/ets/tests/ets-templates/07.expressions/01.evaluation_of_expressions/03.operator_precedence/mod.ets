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
desc: remainder operator with other operators
---*/

function main(): int {
    let res: int, t2 = 2, t3 = 3, t4 = 4, t7 = 7, t8 = 8, t255  = 255, t518 = 518;

    // multiplication
    res = t3 * t7 % t3
    if (res != 0) return 1;

    // division
    res = t8 / t3 % t2
    if (res != 0) return 1;

    // addition
    res = t2 + t7 % t2
    if (res != 3) return 1;

    // subtraction
    res = t8 - t7 % t4
    if (res != 5) return 1;

    // modulo
    res = t518 % t255 as byte
    if (res != 8) return 1;

    // cast
    res = t518 as byte % t255
    if (res != 6) return 1;

    // shifts
    res = t4 << t3 % t2
    if (res != 8) return 1;

    res = t255 >> t3 % t2
    if (res != 127) return 1;

    res = - t255 >>> t7 % t4
    if (res != 536870880) return 1;

    // relational
    if (t8 < t518 % t255) return 1;
    if (t255 <= t518 % t255) return 1;
    if (t3 > t7 % t4) return 1;
    if (t2 >= t7 % t4) return 1;
    if (t4 == t8 % t3) return 1;
    if (t2 != t8 % t3) return 1;

    // bitwise
    res = t7 & t255 % t7
    if (res != 3) return 1;

    res = t8 | t3 % t2
    if (res != 9) return 1;

    res = t255 ^ t7 % t2
    if (res != 254) return 1;

    // ternary
    res = t8 > t2 ? t4 : t8 % t4
    if (res != 4) return 1;

    res = t7 % t4 > t2 ? t2 : t8
    if (res != 2) return 1;

    // assignments
    res = t2 += t7 % t3
    if (res != 3) return 1;
    t2 = 2

    res = t2 -= t7 % t3
    if (res != 1) return 1;
    t2 = 2

    res = t8 *= t7 % t3
    if (res != 8) return 1;
    t8 = 8

    res = t8 /= t7 % t2
    if (res != 8) return 1;
    t8 = 8

    res = t3 &= t7 % t3
    if (res != 1) return 1;
    t3 = 3

    res = t2 |= t7 % t3
    if (res != 3) return 1;
    t2 = 2

    res = t255 ^= t7 % t4
    if (res != 252) return 1;
    t255 = 255

    res = t4 <<= t3 % t2
    if (res != 8) return 1;
    t4 = 4

    res = t255 >>= t3 % t2
    if (res != 127) return 1;
    t255 = 255

    res = t255 >>>= t3 % t2
    if (res != 127) return 1;
    t255 = 255

    return 0;
}
