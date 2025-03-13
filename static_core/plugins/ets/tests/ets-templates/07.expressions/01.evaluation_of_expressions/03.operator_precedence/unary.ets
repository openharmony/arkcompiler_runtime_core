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
desc: unary minus, plus, and bitwise complement operators with other lower precedence operators
---*/

function main(): int {
    let i: int, b: boolean;
    let t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255 = 255, t258 = 258

    // multiplication
    i = + t2 * t4; if (i != 8) return 1;
    i = + + t2 * t4; if (i != 8) return 1;
    i = - t2 * t4; if (i != -8) return 1;
    i = - - t2 * t4; if (i != 8) return 1;
    i = ~ t2 * t4; if (i != -12) return 1;
    i = ~ ~ t2 * t4; if (i != 8) return 1;
    i = + - t2 * t4; if (i != -8) return 1;
    i = + ~ t2 * t4; if (i != -12) return 1;
    i = - + t2 * t4; if (i != -8) return 1;
    i = - ~ t2 * t4; if (i != 12) return 1;
    i = ~ + t2 * t4; if (i != -12) return 1;
    i = ~ - t2 * t4; if (i != 4) return 1;

    // division
    i = + t8 / t2; if (i != 4) return 1;
    i = + + t8 / t2; if (i != 4) return 1;
    i = - t8 / t2; if (i != -4) return 1;
    i = - - t8 / t2; if (i != 4) return 1;
    i = ~ t8 / t2; if (i != -4) return 1;
    i = ~ ~ t8 / t2; if (i != 4) return 1;
    i = + - t8 / t2; if (i != -4) return 1;
    i = + ~ t8 / t2; if (i != -4) return 1;
    i = - + t8 / t2; if (i != -4) return 1;
    i = - ~ t8 / t2; if (i != 4) return 1;
    i = ~ + t8 / t2; if (i != -4) return 1;
    i = ~ - t8 / t2; if (i != 3) return 1;

    // modulo
    i = + t8 % t3; if (i != 2) return 1;
    i = + + t8 % t3; if (i != 2) return 1;
    i = - t8 % t3; if (i != -2) return 1;
    i = - - t8 % t3; if (i != 2) return 1;
    i = ~ t8 % t2; if (i != -1) return 1;
    i = ~ ~ t8 % t3; if (i != 2) return 1;
    i = + - t8 % t3; if (i != -2) return 1;
    i = + ~ t8 % t2; if (i != -1) return 1;
    i = - + t8 % t3; if (i != -2) return 1;
    i = - ~ t8 % t2; if (i != 1) return 1;
    i = ~ + t8 % t2; if (i != -1) return 1;
    i = ~ - t8 % t2; if (i != 1) return 1;

    // addition
    i = + t2 + t4; if (i != 6) return 1;
    i = + + t2 + t4; if (i != 6) return 1;
    i =  - t2 + t4; if (i != 2) return 1;
    i = - - t2 + t4; if (i != 6) return 1;
    i = ~ t2 + t4; if (i != 1) return 1;
    i = ~ ~ t2 + t4; if (i != 6) return 1;
    i = + - t2 + t4; if (i != 2) return 1;
    i = + ~ t2 + t4; if (i != 1) return 1;
    i = - + t2 + t4; if (i != 2) return 1;
    i = - ~ t2 + t4; if (i != 7) return 1;
    i = ~ + t2 + t4; if (i != 1) return 1;
    i = ~ - t2 + t4; if (i != 5) return 1;

    // subtraction
    i = + t2 - t4; if (i != -2) return 1;
    i = + + t2 - t4; if (i != -2) return 1;
    i = - t2 - t4; if (i != -6) return 1;
    i = - - t2 - t4; if (i != -2) return 1;
    i = ~ t2 - t4; if (i != -7) return 1;
    i = ~ ~ t2 - t4; if (i != -2) return 1;
    i = + - t2 - t4; if (i != -6) return 1;
    i = + ~ t2 - t4; if (i != -7) return 1;
    i = - + t2 - t4; if (i != -6) return 1;
    i = - ~ t2 - t4; if (i != -1) return 1;
    i = ~ + t2 - t4; if (i != -7) return 1;
    i = ~ - t2 - t4; if (i != -3) return 1;

    // casting
    i = + t258 as byte; if (i != 2) return 1;
    i = + + t258 as byte; if (i != 2) return 1;
    i = - t258 as byte; if (i != -2) return 1;
    i = - - t258 as byte; if (i != 2) return 1;
    i = ~ t258 as byte; if (i != -3) return 1;
    i = ~ ~ t258 as byte; if (i != 2) return 1;
    i = + - t258 as byte; if (i != -2) return 1;
    i = + ~ t258 as byte; if (i != -3) return 1;
    i = - + t258 as byte; if (i != -2) return 1;
    i = - ~ t258 as byte; if (i != 3) return 1;
    i = ~ + t258 as byte; if (i != -3) return 1;
    i = ~ - t258 as byte; if (i != 1) return 1;

    // right shift
    i = + t255 >> t3; if (i != 31) return 1;
    i = + + t255 >> t3; if (i != 31) return 1;
    i = - t255 >> t3; if (i != -32) return 1;
    i = - - t255 >> t3; if (i != 31) return 1;
    i = ~ t255 >> t3; if (i != -32) return 1;
    i = ~ ~ t255 >> t3; if (i != 31) return 1;
    i = + - t255 >> t3; if (i != -32) return 1;
    i = + ~ t255 >> t3; if (i != -32) return 1;
    i = - + t255 >> t3; if (i != -32) return 1;
    i = - ~ t255 >> t3; if (i != 32) return 1;
    i = ~ + t255 >> t3; if (i != -32) return 1;
    i = ~ - t255 >> t3; if (i != 31) return 1;

    // right shift unsigned
    i = + t255 >>> t3; if (i != 31) return 1;
    i = + + t255 >>> t3; if (i != 31) return 1;
    i = - t255 >>> t3; if (i != 536870880) return 1;
    i = - - t255 >>> t3; if (i != 31) return 1;
    i = ~ t255 >>> t3; if (i != 536870880) return 1;
    i = ~ ~ t255 >>> t3; if (i != 31) return 1;
    i = + - t255 >>> t3; if (i != 536870880) return 1;
    i = + ~ t255 >>> t3; if (i != 536870880) return 1;
    i = - + t255 >>> t3; if (i != 536870880) return 1;
    i = - ~ t255 >>> t3; if (i != 32) return 1;
    i = ~ + t255 >>> t3; if (i != 536870880) return 1;
    i = ~ - t255 >>> t3; if (i != 31) return 1;

    // left shift
    i = + t255 << t3; if (i != 2040) return 1;
    i = + + t255 << t3; if (i != 2040) return 1;
    i = - t255 << t3; if (i != -2040) return 1;
    i = - - t255 << t3; if (i != 2040) return 1;
    i = ~ t255 << t3; if (i != -2048) return 1;
    i = ~ ~ t255 << t3; if (i != 2040) return 1;
    i = + - t255 << t3; if (i != -2040) return 1;
    i = + ~ t255 << t3; if (i != -2048) return 1;
    i = - + t255 << t3; if (i != -2040) return 1;
    i = - ~ t255 << t3; if (i != 2048) return 1;
    i = ~ + t255 << t3; if (i != -2048) return 1;
    i = ~ - t255 << t3; if (i != 2032) return 1;

    // equality
    b = + t2 == 2; if (!b) return 1;
    b = + + t2 == 2; if (!b) return 1;
    b = - t2 == -2; if (!b) return 1;
    b = - - t2 == 2; if (!b) return 1;
    b = ~ t2 == -3; if (!b) return 1;
    b = ~ ~ t2 == 2; if (!b) return 1;
    b = + - t2 == -2; if (!b) return 1;
    b = + ~ t2 == -3; if (!b) return 1;
    b = - + t2 == -2; if (!b) return 1;
    b = - ~ t2 == 3; if (!b) return 1;
    b = ~ + t2 == -3; if (!b) return 1;
    b = ~ - t2 == 1; if (!b) return 1;

    // not equality
    b = + t2 != 2; if (b) return 1;
    b = + + t2 != 2; if (b) return 1;
    b = - t2 != -2; if (b) return 1;
    b = - - t2 != 2; if (b) return 1;
    b = ~ t2 != -3; if (b) return 1;
    b = ~ ~ t2 != 2; if (b) return 1;
    b = + - t2 != -2; if (b) return 1;
    b = + ~ t2 != -3; if (b) return 1;
    b = - + t2 != -2; if (b) return 1;
    b = - ~ t2 != 3; if (b) return 1;
    b = ~ + t2 != -3; if (b) return 1;
    b = ~ - t2 != 1; if (b) return 1;

    // bitwise and
    i = + t255 & t4; if (i != 4) return 1;
    i = + + t255 & t4; if (i != 4) return 1;
    i = - t2 & t4; if (i != 4) return 1;
    i = - - t255 & t4; if (i != 4) return 1;
    i = ~ t2 & t4; if (i != 4) return 1;
    i = ~ ~ t255 & t4; if (i != 4) return 1;
    i = + - t2 & t4; if (i != 4) return 1;
    i = + ~ t2 & t4; if (i != 4) return 1;
    i = - + t2 & t4; if (i != 4) return 1;
    i = - ~ t3 & t4; if (i != 4) return 1;
    i = ~ + t3 & t4; if (i != 4) return 1;
    i = ~ - t255 & t4; if (i != 4) return 1;

    // bitwise or
    i = + t2 | t4; if (i != 6) return 1;
    i = + + t2 | t4; if (i != 6) return 1;
    i = - t255 | t4; if (i != -251) return 1;
    i = - - t2 | t4; if (i != 6) return 1;
    i = ~ t255 | t4; if (i != -252) return 1;
    i = ~ ~ t2 | t4; if (i != 6) return 1;
    i = + - t255 | t4; if (i != -251) return 1;
    i = + ~ t255 | t4; if (i != -252) return 1;
    i = - + t255 | t4; if (i != -251) return 1;
    i = - ~ t255 | t4; if (i != 260) return 1;
    i = ~ + t255 | t4; if (i != -252) return 1;
    i = ~ - t255 | t4; if (i != 254) return 1;

    // bitwise exclusive or
    i = + t255 ^ t4; if (i != 251) return 1;
    i = + + t255 ^ t4; if (i != 251) return 1;
    i = - t255 ^ t4; if (i != -251) return 1;
    i = - - t255 ^ t4; if (i != 251) return 1;
    i = ~ t255 ^ t4; if (i != -252) return 1;
    i = ~ ~ t255 ^ t4; if (i != 251) return 1;
    i = + - t255 ^ t3; if (i != -254) return 1;
    i = + ~ t3 ^ t2; if (i != -2) return 1;
    i = - + t255 ^ t3; if (i != -254) return 1;
    i = - ~ t255 ^ t4; if (i != 260) return 1;
    i = ~ + t3 ^ t2; if (i != -2) return 1;
    i = ~ - t4 ^ t255; if (i != 252) return 1;

    // multiplication assignment
    i = t2 *= + t4; if (i != 8 || t2 != 8) return 1; else t2 = 2;
    i = t2 *= + + t4; if (i != 8 || t2 != 8) return 1; else t2 = 2;
    i = t2 *= - t4; if (i != -8 || t2 != -8) return 1; else t2 = 2;
    i = t2 *= - - t4; if (i != 8 || t2 != 8) return 1; else t2 = 2;
    i = t2 *= ~ t4; if (i != -10 || t2 != -10) return 1; else t2 = 2;
    i = t2 *= ~ ~ t4; if (i != 8 || t2 != 8) return 1; else t2 = 2;
    i = t2 *= + - t4; if (i != -8 || t2 != -8) return 1; else t2 = 2;
    i = t2 *= + ~ t4; if (i != -10 || t2 != -10) return 1; else t2 = 2;
    i = t2 *= - + t4; if (i != -8 || t2 != -8) return 1; else t2 = 2;
    i = t2 *= - ~ t4; if (i != 10 || t2 != 10) return 1; else t2 = 2;
    i = t2 *= ~ + t4; if (i != -10 || t2 != -10) return 1; else t2 = 2;
    i = t2 *= ~ - t4; if (i != 6 || t2 != 6) return 1; else t2 = 2;

    // division assignment
    i = t8 /= + t2; if (i != 4 || t8 != 4) return 1; else t8 = 8;
    i = t8 /= + + t2; if (i != 4 || t8 != 4) return 1; else t8 = 8;
    i = t8 /= - t2; if (i != -4 || t8 != -4) return 1; else t8 = 8;
    i = t8 /= - - t2; if (i != 4 || t8 != 4) return 1; else t8 = 8;
    i = t8 /= ~ t2; if (i != -2 || t8 != -2) return 1; else t8 = 8;
    i = t8 /= ~ ~ t2; if (i != 4 || t8 != 4) return 1; else t8 = 8;
    i = t8 /= + - t2; if (i != -4 || t8 != -4) return 1; else t8 = 8;
    i = t8 /= + ~ t2; if (i != -2 || t8 != -2) return 1; else t8 = 8;
    i = t8 /= - + t2; if (i != -4 || t8 != -4) return 1; else t8 = 8;
    i = t8 /= - ~ t2; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 /= ~ + t2; if (i != -2 || t8 != -2) return 1; else t8 = 8;
    i = t8 /= ~ - t2; if (i != 8 || t8 != 8) return 1; else t8 = 8;

    // modulo assignment
    i = t8 %= + t3; if (i != 2 || t8 != 2) return 1;  else t8 = 8;
    i = t8 %= + + t3; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 %= - t3; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 %= - - t3; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 %= ~ t2; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 %= ~ ~ t3; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 %= + - t3; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 %= + ~ t2; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 %= - + t3; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 %= - ~ t2; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 %= ~ + t2; if (i != 2 || t8 != 2) return 1; else t8 = 8;
    i = t8 %= ~ - t255; if (i != 8 || t8 != 8) return 1; else t8 = 8;

    // addition assignment
    i = t2 += + t4; if (i != 6) return 1; else t2 = 2;
    i = t2 += + + t4; if (i != 6) return 1; else t2 = 2;
    i =  - t2 + t4; if (i != 2) return 1; else t2 = 2;
    i = - - t2 + t4; if (i != 6) return 1; else t2 = 2;
    i = ~ t2 + t4; if (i != 1) return 1; else t2 = 2;
    i = ~ ~ t2 + t4; if (i != 6) return 1; else t2 = 2;
    i = + - t2 + t4; if (i != 2) return 1; else t2 = 2;
    i = + ~ t2 + t4; if (i != 1) return 1; else t2 = 2;
    i = - + t2 + t4; if (i != 2) return 1; else t2 = 2;
    i = - ~ t2 + t4; if (i != 7) return 1; else t2 = 2;
    i = ~ + t2 + t4; if (i != 1) return 1; else t2 = 2;
    i = ~ - t2 + t4; if (i != 5) return 1; else t2 = 2;

    // subtraction assignment
    i = t2 -= + t4; if (i != -2 || t2 != -2) return 1; else t2 = 2;
    i = t2 -= + + t4; if (i != -2 || t2 != -2) return 1; else t2 = 2;
    i = t2 -= - t4; if (i != 6 || t2 != 6) return 1; else t2 = 2;
    i = t2 -= - - t4; if (i != -2 || t2 != -2) return 1; else t2 = 2;
    i = t2 -= ~ t4; if (i != 7 || t2 != 7) return 1; else t2 = 2;
    i = t2 -= ~ ~ t4; if (i != -2 || t2 != -2) return 1; else t2 = 2;
    i = t2 -= + - t4; if (i != 6 || t2 != 6) return 1; else t2 = 2;
    i = t2 -= + ~ t4; if (i != 7 ||  t2 != 7) return 1; else t2 = 2;
    i = t2 -= - + t4; if (i != 6 || t2 != 6) return 1; else t2 = 2;
    i = t2 -= - ~ t4; if (i != -3 || t2 != -3) return 1; else t2 = 2;
    i = t2 -= ~ + t4; if (i != 7 || t2 != 7) return 1; else t2 = 2;
    i = t2 -= ~ - t4; if (i != -1 || t2 != -1) return 1; else t2 = 2;

    // right shift assignment
    i = t255 >>= + t3; if (i != 31 || t255 != 31) return 1; else t255 = 255;
    i = t255 >>= + + t3; if (i != 31 || t255 != 31) return 1; else t255 = 255;
    i = t255 >>= - t255; if (i != 127 || t255 != 127) return 1; else t255 = 255;
    i = t255 >>= - - t3; if (i != 31 || t255 != 31) return 1; else t255 = 255;
    i = t255 >>= ~ t255; if (i != 255 || t255 != 255) return 1; else t255 = 255;
    i = t255 >>= ~ ~ t3; if (i != 31 || t255 != 31) return 1; else t255 = 255;
    i = t255 >>= + - t255; if (i != 127 || t255 != 127) return 1; else t255 = 255;
    i = t255 >>= + ~ t255; if (i != 255 || t255 != 255) return 1; else t255 = 255;
    i = t255 >>= - + t255; if (i != 127 || t255 != 127) return 1; else t255 = 255;
    i = t255 >>= - ~ t3; if (i != 15 || t255 != 15) return 1; else t255 = 255;
    i = t255 >>= ~ + t255; if (i != 255 || t255 != 255) return 1; else t255 = 255;
    i = t255 >>= ~ - t3; if (i != 63 || t255 != 63) return 1; else t255 = 255;

    // right shift unsigned assignment
    i = t255 >>>= + t3; if (i != 31 || t255 != 31) return 1; else t255 = 255;
    i = t255 >>>= + + t3; if (i != 31 || t255 != 31) return 1; else t255 = 255;
    i = t255 >>>= - t255; if (i != 127 || t255 != 127) return 1; else t255 = 255;
    i = t255 >>>= - - t3; if (i != 31 || t255 != 31) return 1; else t255 = 255;
    i = t255 >>>= ~ t255; if (i != 255 || t255 != 255) return 1; else t255 = 255;
    i = t255 >>>= ~ ~ t3; if (i != 31 || t255 != 31) return 1; else t255 = 255;
    i = t255 >>>= + - t255; if (i != 127 || t255 != 127) return 1; else t255 = 255;
    i = t255 >>>= + ~ t255; if (i != 255 || t255 != 255) return 1; else t255 = 255;
    i = t255 >>>= - + t255; if (i != 127 || t255 != 127) return 1; else t255 = 255;
    i = t255 >>>= - ~ t3; if (i != 15 || t255 != 15) return 1; else t255 = 255;
    i = t255 >>>= ~ + t255; if (i != 255 || t255 != 255) return 1; else t255 = 255;
    i = t255 >>>= ~ - t3; if (i != 63 || t255 != 63) return 1; else t255 = 255;

    // left shift assignment
    i = t255 <<= + t3; if (i != 2040 || t255 != 2040) return 1; else t255 = 255;
    i = t255 <<= + + t3; if (i != 2040 || t255 != 2040) return 1; else t255 = 255;
    i = t255 <<= - t3; if (i != -536870912 || t255 != -536870912) return 1; else t255 = 255;
    i = t255 <<= - - t3; if (i != 2040 || t255 != 2040) return 1; else t255 = 255;
    i = t255 <<= ~ t3; if (i != -268435456 || t255 != -268435456) return 1; else t255 = 255;
    i = t255 <<= ~ ~ t3; if (i != 2040 || t255 != 2040) return 1; else t255 = 255;
    i = t255 <<= + - t3; if (i != -536870912 || t255 != -536870912) return 1; else t255 = 255;
    i = t255 <<= + ~ t3; if (i != -268435456 || t255 != -268435456) return 1; else t255 = 255;
    i = t255 <<= - + t3; if (i != -536870912 || t255 != -536870912) return 1; else t255 = 255;
    i = t255 <<= - ~ t3; if (i != 4080 || t255 != 4080) return 1; else t255 = 255;
    i = t255 <<= ~ + t3; if (i != -268435456 || t255 != -268435456) return 1; else t255 = 255;
    i = t255 <<= ~ - t3; if (i != 1020 || t255 != 1020) return 1; else t255 = 255;

    // bitwise and assignment
    i = t255 &= + t4; if (i != 4 || t255 != 4) return 1; else t255 = 255;
    i = t255 &= + + t4; if (i != 4 || t255 != 4) return 1; else t255 = 255;
    i = t4 &= - t2; if (i != 4 || t4 != 4) return 1; else t4 = 4;
    i = t255 &= - - t4; if (i != 4 || t255 != 4) return 1; else t255 = 255;
    i = t2 &= ~ t4; if (i != 2 || t2 != 2) return 1; else t2 = 2;
    i = t255 &= ~ ~ t4; if (i != 4 || t255 != 4) return 1; else t255 = 255;
    i = t4 &= + - t2; if (i != 4 || t4 != 4) return 1; else t4 = 4;
    i = t2 &= + ~ t4; if (i != 2 || t2 != 2) return 1; else t2 = 2;
    i = t4 &= - + t2; if (i != 4 || t4 != 4) return 1; else t4 = 4;
    i = t3 &= - ~ t4; if (i != 1 || t3 != 1) return 1; else t3 = 3;
    i = t3 &= ~ + t4; if (i != 3 || t3 != 3) return 1; else t3 = 3;
    i = t255 &= ~ - t4; if (i != 3 || t255 != 3) return 1; else t255 = 255

    // bitwise or assignment
    i = t2 |= + t4; if (i != 6 || t2 != 6) return 1; else t2 = 2;
    i = t2 |= + + t4; if (i != 6 || t2 != 6) return 1; else t2 = 2;
    i = t255 |= - t4; if (i != -1 || t255 != -1) return 1; else t255 = 255;
    i = t2 |= - - t4; if (i != 6 || t2 != 6) return 1; else t2 = 2;
    i = t4 |= ~ t255; if (i != -252 || t4 != -252) return 1; else t4 = 4;
    i = t2 |= ~ ~ t4; if (i != 6 || t2 != 6) return 1; else t2 = 2;
    i = t255 |= + - t4; if (i != -1 || t255 != -1) return 1; else t255 = 255;
    i = t4 |= + ~ t255; if (i != -252 || t4 != -252) return 1; else t4 = 4;
    i = t255 |= - + t4; if (i != -1 || t255 != -1) return 1; else t255 = 255;
    i = t4 |= - ~ t255; if (i != 260 || t4 != 260) return 1; else t4 = 4;
    i = t4 |= ~ + t255; if (i != -252 || t4 != -252) return 1; else t4 = 4;
    i = t4 |= ~ - t255; if (i != 254 || t4 != 254) return 1; else t4 = 4;

    // bitwise exclusive or assignment
    i = t4 ^= + t255; if (i != 251 || t4 != 251) return 1; else t4 = 4;
    i = t4 ^= + + t255; if (i != 251 || t4 != 251) return 1; else t4 = 4;
    i = t4 ^= - t255; if (i != -251 || t4 != -251) return 1; else t4 = 4;
    i = t4 ^= - - t255; if (i != 251 || t4 != 251) return 1; else t4 = 4;
    i = t4 ^= ~ t255; if (i != -252 || t4 != -252) return 1; else t4 = 4;
    i = t4 ^= ~ ~ t255; if (i != 251 || t4 != 251) return 1; else t4 = 4;
    i = t3 ^= + - t255; if (i != -254 || t3 != -254) return 1; else t3 = 3;
    i = t2 ^= + ~ t3; if (i != -2 || t2 != -2) return 1; else t2 = 2;
    i = t3 ^= - + t255; if (i != -254 || t3 != -254) return 1; else t3 = 3;
    i = t4 ^= - ~ t255; if (i != 260 || t4 != 260) return 1; else t4 = 4;
    i = t2 ^= ~ + t3; if (i != -2 || t2 != -2) return 1; else t2 = 2;
    i = t255 ^= ~ - t4; if (i != 252 || t255 != 252) return 1; else t255 = 255;

    return 0;
}
