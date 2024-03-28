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
desc: postfix operators with other lower precedence operators
---*/

function main(): int {
    let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255  = 255

    // unary plus
    res = + t2++
    if (res != 2 || t2 != 3) return 1;
    t2 = 2

    res = + t2--
    if (res != 2 || t2 != 1) return 1;
    t2 = 2

    // unary minus
    res = - t2++
    if (res != -2 || t2 != 3) return 1;
    t2 = 2

    res = - t2--
    if (res != -2 || t2 != 1) return 1;
    t2 = 2

    // bitwise complement
    res = ~ t2++
    if (res != -3 || t2 != 3) return 1;
    t2 = 2

    res = ~ t2--
    if (res != -3 || t2 != 1) return 1;
    t2 = 2

    // multiplicative
    res = t4-- * t2++
    if (res != 8 || t2 != 3 || t4 != 3) return 1;
    t2 = 2
    t4 = 4

    res = t4++ * t2--
    if (res != 8 || t2 != 1 || t4 != 5) return 1;
    t2 = 2
    t4 = 4

    // division
    res = t4-- / t2++
    if (res != 2 || t2 != 3 || t4 != 3) return 1;
    t2 = 2
    t4 = 4

    res = t4++ / t2--
    if (res != 2 || t2 != 1 || t4 != 5) return 1;
    t2 = 2
    t4 = 4

    // modulo
    res = t255-- % t4++
    if (res != 3 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255++ % t4--
    if (res != 3 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // addition
    res = t255-- + t4++
    if (res != 259 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255++ + t4--
    if (res != 259 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // subtraction
    res = t255-- - t4++
    if (res != 251 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255++ - t4--
    if (res != 251 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // casting
    res = t255-- as byte - t4++ as byte
    if (res != -5 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255++ as byte - t4-- as byte
    if (res != -5 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // left shift
    res = t255-- << t4++
    if (res != 4080 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255++ << t4--
    if (res != 4080 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // right shift
    res = t255-- >> t4++
    if (res != 15 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255++ >> t4--
    if (res != 15 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // right no-sign shift
    res = - t255-- >>> t4++
    if (res != 268435440 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = - t255++ >>> t4--
    if (res != 268435440 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // relational
    if (t3-- < t2++) return 1;
    t3 = 3
    t2 = 2

    if (t8-- <= t4++) return 1;
    t8 = 8
    t4 = 4

    if (t2++ > t3--) return 1;
    t3 = 3
    t2 = 2

    if (t4++ >= t8--) return 1;
    t8 = 8
    t4 = 4

    // equality
    if (t3-- == t2) return 1;
    t3 = 3

    if (t3++ == t4) return 1;
    t3 = 3

    if (t3-- != 3) return 1;
    t3 = 3

    if (t3++ != 3) return 1;
    t3 = 3

    // bitwise AND
    res = t255-- & t4++
    if (res != 4 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255++ & t4--
    if (res != 4 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // bitwise OR
    res = t255-- | t4++
    if (res != 255 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255++ | t4--
    if (res != 255 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // bitwise XOR
    res = t255-- ^ t4++
    if (res != 251 || t255 != 254 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255++ ^ t4--
    if (res != 251 || t255 != 256 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    // ternary
    res = t8 > t4 ? t255-- : t4++
    if (res != 255 || t255 != 254 || t4 != 4) return 1;
    t255 = 255

    res = t8 > t4 ? t4-- : t255++
    if (res != 4 || t255 != 255 || t4 != 3) return 1;
    t4 = 4

    // assignment
    res = t255 = t3--
    if (res != 3 || t255 != 3 || t3 != 2) return 1;
    t255 = 255
    t3 = 3

    res = t255 = t3++
    if (res != 3 || t255 != 3 || t3 != 4) return 1;
    t255 = 255
    t3 = 3

    res = t255 += t3--
    if (res != 258 || t255 != 258 || t3 != 2) return 1;
    t255 = 255
    t3 = 3

    res = t255 += t3++
    if (res != 258 || t255 != 258 || t3 != 4) return 1;
    t255 = 255
    t3 = 3

    res = t255 -= t3--
    if (res != 252 || t255 != 252 || t3 != 2) return 1;
    t255 = 255
    t3 = 3

    res = t255 -= t3++
    if (res != 252 || t255 != 252 || t3 != 4) return 1;
    t255 = 255
    t3 = 3

    res = t255 *= t3--
    if (res != 765 || t255 != 765 || t3 != 2) return 1;
    t255 = 255
    t3 = 3

    res = t255 *= t3++
    if (res != 765 || t255 != 765 || t3 != 4) return 1;
    t255 = 255
    t3 = 3

    res = t255 /= t3--
    if (res != 85 || t255 != 85 || t3 != 2) return 1;
    t255 = 255
    t3 = 3

    res = t255 /= t3++
    if (res != 85 || t255 != 85 || t3 != 4) return 1;
    t255 = 255
    t3 = 3

    res = t255 %= t4--
    if (res != 3 || t255 != 3 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 %= t4++
    if (res != 3 || t255 != 3 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 <<= t4--
    if (res != 4080 || t255 != 4080 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 <<= t4++
    if (res != 4080 || t255 != 4080 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 >>= t4--
    if (res != 15 || t255 != 15 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 >>= t4++
    if (res != 15 || t255 != 15 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 >>>= t4--
    if (res != 15 || t255 != 15 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 >>>= t4++
    if (res != 15 || t255 != 15 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 &= t4--
    if (res != 4 || t255 != 4 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 &= t4++
    if (res != 4 || t255 != 4 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 |= t4--
    if (res != 255 || t255 != 255 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 |= t4++
    if (res != 255 || t255 != 255 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    res = t255 ^= t4--
    if (res != 251 || t255 != 251 || t4 != 3) return 1;
    t255 = 255
    t4 = 4

    res = t255 ^= t4++
    if (res != 251 || t255 != 251 || t4 != 5) return 1;
    t255 = 255
    t4 = 4

    return 0;
}
