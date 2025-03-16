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
desc: ternary operator with other lower precedence operators
---*/

function main(): int {
    let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255  = 255

    // assignment
    res = t2 += t2 < t3 ? t4 : t8
    if (res != 6) return 1; else t2 = 2;

    res = t2 -= t2 < t3 ? t4 : t8
    if (res != -2) return 1; else t2 = 2;

    res = t2 *= t2 < t3 ? t4 : t3
    if (res != 8) return 1; else t2 = 2;

    res = t2 /= t2 < t3 ? t4 : t3
    if (res != 0) return 1; else t2 = 2;

    res = t2 %= t2 < t3 ? t4 : t3
    if (res != 2) return 1; else t2 = 2;

    res = t2 &= t2 < t3 ? t4 : t3
    if (res != 0) return 1; else t2 = 2;

    res = t2 |= t2 < t3 ? t4 : t3
    if (res != 6) return 1; else t2 = 2;

    res = t2 ^= t2 < t3 ? t4 : t3
    if (res != 6) return 1; else t2 = 2;

    res = t2 <<= t2 < t3 ? t4 : t3
    if (res != 32) return 1; else t2 = 2;

    res = t2 >>= t2 < t3 ? t4 : t3
    if (res != 0) return 1; else t2 = 2;

    res = t2 >>>= t2 < t3 ? t4 : t3
    if (res != 0) return 1; else t2 = 2;

    return 0;
}
