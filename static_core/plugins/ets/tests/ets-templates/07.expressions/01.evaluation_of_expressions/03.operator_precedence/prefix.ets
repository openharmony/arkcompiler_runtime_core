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
desc: prefix operators with other lower precedence operators
---*/

function main(): int {
    let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255  = 255

    // unary plus
    res = + ++t2
    if (res != 3 || t2 != 3) return 1;
    t2 = 2

    res = + --t2
    if (res != 1 || t2 != 1) return 1;
    t2 = 2

    // unary minus
    res = - ++t2
    if (res != -3 || t2 != 3) return 1;
    t2 = 2

    res = - --t2
    if (res != -1 || t2 != 1) return 1;
    t2 = 2

    // bitwise complement
    res = ~ ++t2
    if (res != -4 || t2 != 3) return 1;
    t2 = 2

    res = ~ --t2
    if (res != -2 || t2 != 1) return 1;
    t2 = 2

    // multiplicative
    res = --t4 * ++t2
    if (res != 9 || t2 != 3 || t4 != 3) return 1;
    t2 = 2
    t4 = 4

    res = ++t4 * --t2
    if (res != 5 || t2 != 1 || t4 != 5) return 1;
    t2 = 2
    t4 = 4

    // division
    res = --t4 / ++t2
    if (res != 1 || t2 != 3 || t4 != 3) return 1;
    t2 = 2
    t4 = 4

    res = ++t4 / --t2
    if (res != 5 || t2 != 1 || t4 != 5) return 1;
    t2 = 2
    t4 = 4

    // modulo
    res = --t255 % ++t4
    if (res != 4 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = ++t255 % --t4
    if (res != 1 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // addition
    res = --t255 + ++t4
    if (res != 259 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = ++t255 + --t4
    if (res != 259 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // subtraction
    res = --t255 - ++t4
    if (res != 249 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = ++t255 - --t4
    if (res != 253 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // casting
    res = --t255 as byte - ++t4 as byte
    if (res != -7 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = ++t255 as byte - --t4 as byte
    if (res != -3 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // left shift
    res = --t255 << ++t4
    if (res != 8128 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = ++t255 << --t4
    if (res != 2048 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // right shift
    res = --t255 >> ++t4
    if (res != 7 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = ++t255 >> --t4
    if (res != 32 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // right no-sign shift
    res = - --t255 >>> ++t4
    if (res != 134217720 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = - ++t255 >>> --t4
    if (res != 536870880 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // relational
    if (--t4 < ++t2) return 1;
    t4 = 4
    t2 = 2

    if (--t8 <= ++t4) return 1;
    t8 = 8
    t4 = 4

    if (++t2 > --t4) return 1;
    t4 = 4
    t2 = 2

    if (++t4 >= --t8) return 1;
    t8 = 8
    t4 = 4

    // equality
    if (--t3 == 3) return 1;
    t3 = 3

    if (++t3 == 3) return 1;
    t3 = 3

    if (--t3 != 2) return 1;
    t3 = 3

    if (++t3 != 4) return 1;
    t3 = 3

    // bitwise AND
    res = --t255 & ++t4
    if (res != 4 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = ++t255 & --t8
    if (res != 0 || t255 != 256 || t8 != 7) return 1;
    t255 = 255
    t8 = 8

    // bitwise OR
    res = --t255 | ++t4
    if (res != 255 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = ++t255 | --t4
    if (res != 259 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // bitwise XOR
    res = --t255 ^ ++t4
    if (res != 251 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = ++t255 ^ --t4
    if (res != 259 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // ternary
    res = t8 > t4 ? --t255 : ++t4
    if (res != 254 || t255 != 254 || t4 != 4) return 1;
    t255 = 255

    res = t8 > t4 ? --t4 : ++t255
    if (res != 3 || t255 != 255 || t4 != 3) return 1;
    t4 = 4

    // assignment
    res = t255 = --t3
    if (res != 2 || t255 != 2 || t3 != 2) return 1;
    t255 = 255
    t3 = 3

    res = t255 = ++t3
    if (res != 4 || t255 != 4 || t3 != 4) return 1;
    t255 = 255
    t3 = 3

    res = t255 += --t3
    if (res != 257 || t255 != 257 || t3 != 2) return 1;
    t255 = 255
    t3 = 3

    res = t255 += ++t3
    if (res != 259 || t255 != 259 || t3 != 4) return 1;
    t255 = 255
    t3 = 3

    res = t255 -= --t3
    if (res != 253 || t255 != 253 || t3 != 2) return 1;
    t255 = 255
    t3 = 3

    res = t255 -= ++t3
    if (res != 251 || t255 != 251 || t3 != 4) return 1;
    t255 = 255
    t3 = 3

    res = t255 *= --t3
    if (res != 510 || t255 != 510 || t3 != 2) return 1;
    t255 = 255
    t3 = 3

    res = t255 *= ++t3
    if (res != 1020 || t255 != 1020 || t3 != 4) return 1;
    t255 = 255
    t3 = 3

    res = t255 /= --t3
    if (res != 127 || t255 != 127 || t3 != 2) return 1;
    t255 = 255
    t3 = 3

    res = t255 /= ++t3
    if (res != 63 || t255 != 63 || t3 != 4) return 1;
    t255 = 255
    t3 = 3

    res = t255 %= --t4
    if (res != 0 || t255 != 0 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 %= ++t8
    if (res != 3 || t255 != 3 || t8 != 9) return 1;
    t255 = 255
    t8 = 8

    res = t255 <<= --t4
    if (res != 2040 || t255 != 2040 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 <<= ++t4
    if (res != 8160 || t255 != 8160 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 >>= --t4
    if (res != 31 || t255 != 31 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 >>= ++t4
    if (res != 7 || t255 != 7 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 >>>= --t4
    if (res != 31 || t255 != 31 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 >>>= ++t4
    if (res != 7 || t255 != 7 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 &= --t4
    if (res != 3 || t255 != 3 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 &= ++t4
    if (res != 5 || t255 != 5 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 |= --t4
    if (res != 255 || t255 != 255 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 |= ++t4
    if (res != 255 || t255 != 255 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 ^= --t4
    if (res != 252 || t255 != 252 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 ^= ++t4
    if (res != 250 || t255 != 250 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    return 0;
}
