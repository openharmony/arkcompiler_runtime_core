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
    // lt
    arktest.assertTrue(c'a' < c'c')
    arktest.assertFalse(c'b' < c'b')
    arktest.assertFalse(c'c' < c'a')

    arktest.assertTrue(c'a' < 99)
    arktest.assertFalse(c'b' < 98)
    arktest.assertFalse(c'c' < 97)

    arktest.assertTrue(97 < c'c')
    arktest.assertFalse(98 < c'b')
    arktest.assertFalse(99 < c'a')

    arktest.assertTrue(c'a' < 99f)
    arktest.assertFalse(c'b' < 98f)
    arktest.assertFalse(c'c' < 97f)

    arktest.assertTrue(97f < c'c')
    arktest.assertFalse(98f < c'b')
    arktest.assertFalse(99f < c'a')

    arktest.assertTrue(c'a' < 99.0)
    arktest.assertFalse(c'b' < 98.0)
    arktest.assertFalse(c'c' < 97.1)

    arktest.assertTrue(97.1 < c'c')
    arktest.assertFalse(98.0 < c'b')
    arktest.assertFalse(99.0 < c'a')

    // le
    arktest.assertTrue(c'a' <= c'c')
    arktest.assertTrue(c'b' <= c'b')
    arktest.assertFalse(c'c' <= c'a')

    arktest.assertTrue(c'a' <= 99)
    arktest.assertTrue(c'b' <= 98)
    arktest.assertFalse(c'c' <= 97)

    arktest.assertTrue(97 <= c'c')
    arktest.assertTrue(98 <= c'b')
    arktest.assertFalse(99 <= c'a')

    arktest.assertTrue(c'a' <= 99f)
    arktest.assertTrue(c'b' <= 98f)
    arktest.assertFalse(c'c' <= 97f)

    arktest.assertTrue(97f <= c'c')
    arktest.assertTrue(98f <= c'b')
    arktest.assertFalse(99f <= c'a')

    arktest.assertTrue(c'a' <= 99.0)
    arktest.assertTrue(c'b' <= 98.0)
    arktest.assertFalse(c'c' <= 97.1)

    arktest.assertTrue(97.1 <= c'c')
    arktest.assertTrue(98.0 <= c'b')
    arktest.assertFalse(99.0 <= c'a')

    // gt
    arktest.assertFalse(c'a' > c'c')
    arktest.assertFalse(c'b' > c'b')
    arktest.assertTrue(c'c' > c'a')

    arktest.assertFalse(c'a' > 99)
    arktest.assertFalse(c'b' > 98)
    arktest.assertTrue(c'c' > 97)

    arktest.assertFalse(97 > c'c')
    arktest.assertFalse(98 > c'b')
    arktest.assertTrue(99 > c'a')

    arktest.assertFalse(c'a' > 99f)
    arktest.assertFalse(c'b' > 98f)
    arktest.assertTrue(c'c' > 97f)

    arktest.assertFalse(97f > c'c')
    arktest.assertFalse(98f > c'b')
    arktest.assertTrue(99f > c'a')

    arktest.assertFalse(c'a' > 99.0)
    arktest.assertFalse(c'b' > 98.0)
    arktest.assertTrue(c'c' > 97.1)

    arktest.assertFalse(97.1 > c'c')
    arktest.assertFalse(98.0 > c'b')
    arktest.assertTrue(99.0 > c'a')

    // ge
    arktest.assertFalse(c'a' >= c'c')
    arktest.assertTrue(c'b' >= c'b')
    arktest.assertTrue(c'c' >= c'a')

    arktest.assertFalse(c'a' >= 99)
    arktest.assertTrue(c'b' >= 98)
    arktest.assertTrue(c'c' >= 97)

    arktest.assertFalse(97 >= c'c')
    arktest.assertTrue(98 >= c'b')
    arktest.assertTrue(99 >= c'a')

    arktest.assertFalse(c'a' >= 99f)
    arktest.assertTrue(c'b' >= 98f)
    arktest.assertTrue(c'c' >= 97f)

    arktest.assertFalse(97f >= c'c')
    arktest.assertTrue(98f >= c'b')
    arktest.assertTrue(99f >= c'a')

    arktest.assertFalse(c'a' >= 99.0)
    arktest.assertTrue(c'b' >= 98.0)
    arktest.assertTrue(c'c' >= 97.1)

    arktest.assertFalse(97.1 >= c'c')
    arktest.assertTrue(98.0 >= c'b')
    arktest.assertTrue(99.0 >= c'a')
}
