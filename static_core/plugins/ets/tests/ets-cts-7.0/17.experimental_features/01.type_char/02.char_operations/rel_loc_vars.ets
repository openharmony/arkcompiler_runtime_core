/*
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

/*---
desc: |-
    Relational operators can be used if:
    - both operands are of char type; or
    - one operand is of char type and other is of a numeric type
---*/

function main() {
    let c1: char = c'a'
    let c2: char = c'b'
    let c3: char = c'c'

    let b1: byte = 97
    let b2: byte = 98
    let b3: byte = 99

    let s1: short = 97
    let s2: short = 98
    let s3: short = 99

    let i1: int = 97
    let i2: int = 98
    let i3: int = 99

    let l1: long = 97
    let l2: long = 98
    let l3: long = 99

    let f1: float = 97.2f
    let f2: float = 98f
    let f3: float = 99.2f

    let d1: double = 97.1
    let d2: double = 98.0
    let d3: double = 99.1

    // lt
    arktest.assertTrue(c1 < c3)
    arktest.assertFalse(c2 < c2)
    arktest.assertFalse(c3 < c1)

    arktest.assertTrue(c1 < b3)
    arktest.assertFalse(c2 < b2)
    arktest.assertFalse(c3 < b1)

    arktest.assertTrue(b1 < c3)
    arktest.assertFalse(b2 < c2)
    arktest.assertFalse(b3 < c1)

    arktest.assertTrue(c1 < s3)
    arktest.assertFalse(c2 < s2)
    arktest.assertFalse(c3 < s1)

    arktest.assertTrue(s1 < c3)
    arktest.assertFalse(s2 < c2)
    arktest.assertFalse(s3 < c1)

    arktest.assertTrue(c1 < i3)
    arktest.assertFalse(c2 < i2)
    arktest.assertFalse(c3 < i1)

    arktest.assertTrue(i1 < c3)
    arktest.assertFalse(i2 < c2)
    arktest.assertFalse(i3 < c1)

    arktest.assertTrue(c1 < l3)
    arktest.assertFalse(c2 < l2)
    arktest.assertFalse(c3 < l1)

    arktest.assertTrue(l1 < c3)
    arktest.assertFalse(l2 < c2)
    arktest.assertFalse(l3 < c1)

    arktest.assertTrue(c1 < f3)
    arktest.assertFalse(c2 < f2)
    arktest.assertFalse(c3 < f1)

    arktest.assertTrue(f1 < c3)
    arktest.assertFalse(f2 < c2)
    arktest.assertFalse(f3 < c1)

    arktest.assertTrue(c1 < d3)
    arktest.assertFalse(c2 < d2)
    arktest.assertFalse(c3 < d1)

    arktest.assertTrue(d1 < c3)
    arktest.assertFalse(d2 < c2)
    arktest.assertFalse(d3 < c1)

    // le
    arktest.assertTrue(c1 <= c3)
    arktest.assertTrue(c2 <= c2)
    arktest.assertFalse(c3 <= c1)

    arktest.assertTrue(c1 <= b3)
    arktest.assertTrue(c2 <= b2)
    arktest.assertFalse(c3 <= b1)

    arktest.assertTrue(b1 <= c3)
    arktest.assertTrue(b2 <= c2)
    arktest.assertFalse(b3 <= c1)

    arktest.assertTrue(c1 <= s3)
    arktest.assertTrue(c2 <= s2)
    arktest.assertFalse(c3 <= s1)

    arktest.assertTrue(s1 <= c3)
    arktest.assertTrue(s2 <= c2)
    arktest.assertFalse(s3 <= c1)

    arktest.assertTrue(c1 <= i3)
    arktest.assertTrue(c2 <= i2)
    arktest.assertFalse(c3 <= i1)

    arktest.assertTrue(i1 <= c3)
    arktest.assertTrue(i2 <= c2)
    arktest.assertFalse(i3 <= c1)

    arktest.assertTrue(c1 <= l3)
    arktest.assertTrue(c2 <= l2)
    arktest.assertFalse(c3 <= l1)

    arktest.assertTrue(l1 <= c3)
    arktest.assertTrue(l2 <= c2)
    arktest.assertFalse(l3 <= c1)

    arktest.assertTrue(c1 <= f3)
    arktest.assertTrue(c2 <= f2)
    arktest.assertFalse(c3 <= f1)

    arktest.assertTrue(f1 <= c3)
    arktest.assertTrue(f2 <= c2)
    arktest.assertFalse(f3 <= c1)

    arktest.assertTrue(c1 <= d3)
    arktest.assertTrue(c2 <= d2)
    arktest.assertFalse(c3 <= d1)

    arktest.assertTrue(d1 <= c3)
    arktest.assertTrue(d2 <= c2)
    arktest.assertFalse(d3 <= c1)

    // gt
    arktest.assertFalse(c1 > c3)
    arktest.assertFalse(c2 > c2)
    arktest.assertTrue(c3 > c1)

    arktest.assertFalse(c1 > b3)
    arktest.assertFalse(c2 > b2)
    arktest.assertTrue(c3 > b1)

    arktest.assertFalse(b1 > c3)
    arktest.assertFalse(b2 > c2)
    arktest.assertTrue(b3 > c1)

    arktest.assertFalse(c1 > s3)
    arktest.assertFalse(c2 > s2)
    arktest.assertTrue(c3 > s1)

    arktest.assertFalse(s1 > c3)
    arktest.assertFalse(s2 > c2)
    arktest.assertTrue(s3 > c1)

    arktest.assertFalse(c1 > i3)
    arktest.assertFalse(c2 > i2)
    arktest.assertTrue(c3 > i1)

    arktest.assertFalse(i1 > c3)
    arktest.assertFalse(i2 > c2)
    arktest.assertTrue(i3 > c1)

    arktest.assertFalse(c1 > l3)
    arktest.assertFalse(c2 > l2)
    arktest.assertTrue(c3 > l1)

    arktest.assertFalse(l1 > c3)
    arktest.assertFalse(l2 > c2)
    arktest.assertTrue(l3 > c1)

    arktest.assertFalse(c1 > f3)
    arktest.assertFalse(c2 > f2)
    arktest.assertTrue(c3 > f1)

    arktest.assertFalse(f1 > c3)
    arktest.assertFalse(f2 > c2)
    arktest.assertTrue(f3 > c1)

    arktest.assertFalse(c1 > d3)
    arktest.assertFalse(c2 > d2)
    arktest.assertTrue(c3 > d1)

    arktest.assertFalse(d1 > c3)
    arktest.assertFalse(d2 > c2)
    arktest.assertTrue(d3 > c1)

    // ge
    arktest.assertFalse(c1 >= c3)
    arktest.assertTrue(c2 >= c2)
    arktest.assertTrue(c3 >= c1)

    arktest.assertFalse(c1 >= b3)
    arktest.assertTrue(c2 >= b2)
    arktest.assertTrue(c3 >= b1)

    arktest.assertFalse(b1 >= c3)
    arktest.assertTrue(b2 >= c2)
    arktest.assertTrue(b3 >= c1)

    arktest.assertFalse(c1 >= s3)
    arktest.assertTrue(c2 >= s2)
    arktest.assertTrue(c3 >= s1)

    arktest.assertFalse(s1 >= c3)
    arktest.assertTrue(s2 >= c2)
    arktest.assertTrue(s3 >= c1)

    arktest.assertFalse(c1 >= i3)
    arktest.assertTrue(c2 >= i2)
    arktest.assertTrue(c3 >= i1)

    arktest.assertFalse(i1 >= c3)
    arktest.assertTrue(i2 >= c2)
    arktest.assertTrue(i3 >= c1)

    arktest.assertFalse(c1 >= l3)
    arktest.assertTrue(c2 >= l2)
    arktest.assertTrue(c3 >= l1)

    arktest.assertFalse(l1 >= c3)
    arktest.assertTrue(l2 >= c2)
    arktest.assertTrue(l3 >= c1)

    arktest.assertFalse(c1 >= f3)
    arktest.assertTrue(c2 >= f2)
    arktest.assertTrue(c3 >= f1)

    arktest.assertFalse(f1 >= c3)
    arktest.assertTrue(f2 >= c2)
    arktest.assertTrue(f3 >= c1)

    arktest.assertFalse(c1 >= d3)
    arktest.assertTrue(c2 >= d2)
    arktest.assertTrue(c3 >= d1)

    arktest.assertFalse(d1 >= c3)
    arktest.assertTrue(d2 >= c2)
    arktest.assertTrue(d3 >= c1)
}
