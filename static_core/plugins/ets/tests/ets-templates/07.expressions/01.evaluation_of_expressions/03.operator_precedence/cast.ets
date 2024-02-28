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
desc: cast operator with other lower precedence operators
---*/

function main(): int {
    let res: int, t1 = 1, t2 = 2, t3 = 3, t4 = 4, t255 = 255, t257 = 257, t258 = 258

    // shifts
    res = t255 << t257 as byte
    if (res != 510) return 1;

    res = t257 >> t257 as byte
    if (res != 128) return 1;

    res = - t255 >>> t257 as byte
    if (res != 2147483520) return 1;

    // relational
    if (t2 < t258 as byte) return 1;
    if (t3 <= t258 as byte) return 1;
    if (t2 > t258 as byte) return 1;
    if (t1 >= t258 as byte) return 1;
    if (t1 == t258 as byte) return 1;
    if (t2 != t258 as byte) return 1;

    // bitwise
    res = t255 & t258 as byte
    if (res != 2) return 1;

    res = t3 | t258 as byte
    if (res != 3) return 1;

    res = t255 ^ t258 as byte
    if (res != 253) return 1;

    // ternary
    res = t3 <= t4 ? t257 : t258 as byte
    if (res != 257) return 1;

    // assignments
    res = t258 += t257 as byte
    if (res != 259) return 1; else t258 = 258;

    res = t258 -= t257 as byte
    if (res != 257) return 1; else t258 = 258;

    res = t258 *= t257 as byte
    if (res != 258) return 1; else t258 = 258;

    res = t258 /= t257 as byte
    if (res != 258) return 1; else t258 = 258;

    res = t255 %= t258 as byte
    if (res != 1) return 1; else t255 = 255;

    res = t255 &= t258 as byte
    if (res != 2) return 1; else t255 = 255;

    res = t3 |= t258 as byte
    if (res != 3) return 1; else t3 = 3;

    res = t255 ^= t258 as byte
    if (res != 253) return 1; else t255 = 255;

    res = t255 <<= t257 as byte
    if (res != 510) return 1; else t255 = 255;

    res = t257 >>= t257 as byte
    if (res != 128) return 1; else t257 = 257;

    res = t255 >>>= t257 as byte
    if (res != 127) return 1; else t255 = 255;

    return 0;
}
