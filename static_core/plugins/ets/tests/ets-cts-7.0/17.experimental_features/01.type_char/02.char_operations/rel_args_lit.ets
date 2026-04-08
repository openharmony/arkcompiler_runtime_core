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

function lt(p1: char, p2: char) { return p1 < p2 }
function le(p1: char, p2: char) { return p1 <= p2 }
function gt(p1: char, p2: char) { return p1 > p2 }
function ge(p1: char, p2: char) { return p1 >= p2 }

function lt(p1: char, p2: int) { return p1 < p2 }
function le(p1: char, p2: int) { return p1 <= p2 }
function gt(p1: char, p2: int) { return p1 > p2 }
function ge(p1: char, p2: int) { return p1 >= p2 }

function lt(p1: int, p2: char) { return p1 < p2 }
function le(p1: int, p2: char) { return p1 <= p2 }
function gt(p1: int, p2: char) { return p1 > p2 }
function ge(p1: int, p2: char) { return p1 >= p2 }

function lt(p1: char, p2: float) { return p1 < p2 }
function le(p1: char, p2: float) { return p1 <= p2 }
function gt(p1: char, p2: float) { return p1 > p2 }
function ge(p1: char, p2: float) { return p1 >= p2 }

function lt(p1: float, p2: char) { return p1 < p2 }
function le(p1: float, p2: char) { return p1 <= p2 }
function gt(p1: float, p2: char) { return p1 > p2 }
function ge(p1: float, p2: char) { return p1 >= p2 }

function lt(p1: char, p2: double) { return p1 < p2 }
function le(p1: char, p2: double) { return p1 <= p2 }
function gt(p1: char, p2: double) { return p1 > p2 }
function ge(p1: char, p2: double) { return p1 >= p2 }

function lt(p1: double, p2: char) { return p1 < p2 }
function le(p1: double, p2: char) { return p1 <= p2 }
function gt(p1: double, p2: char) { return p1 > p2 }
function ge(p1: double, p2: char) { return p1 >= p2 }

function main() {
    // lt
    arktest.assertTrue(lt(c'a', c'c'))
    arktest.assertFalse(lt(c'b', c'b'))
    arktest.assertFalse(lt(c'c', c'a'))

    arktest.assertTrue(lt(c'a', 99))
    arktest.assertFalse(lt(c'b', 98))
    arktest.assertFalse(lt(c'c', 97))

    arktest.assertTrue(lt(97, c'c'))
    arktest.assertFalse(lt(98, c'b'))
    arktest.assertFalse(lt(99, c'a'))

    arktest.assertTrue(lt(c'a', 99.2f))
    arktest.assertFalse(lt(c'b', 98f))
    arktest.assertFalse(lt(c'c', 97.2f))

    arktest.assertTrue(lt(97.2f, c'c'))
    arktest.assertFalse(lt(98f, c'b'))
    arktest.assertFalse(lt(99.2f, c'a'))

    arktest.assertTrue(lt(c'a', 99.1))
    arktest.assertFalse(lt(c'b', 98.0))
    arktest.assertFalse(lt(c'c', 97.1))

    arktest.assertTrue(lt(97.1, c'c'))
    arktest.assertFalse(lt(98.0, c'b'))
    arktest.assertFalse(lt(99.1, c'a'))

    // le
    arktest.assertTrue(le(c'a', c'c'))
    arktest.assertTrue(le(c'b', c'b'))
    arktest.assertFalse(le(c'c', c'a'))

    arktest.assertTrue(le(c'a', 99))
    arktest.assertTrue(le(c'b', 98))
    arktest.assertFalse(le(c'c', 97))

    arktest.assertTrue(le(97, c'c'))
    arktest.assertTrue(le(98, c'b'))
    arktest.assertFalse(le(99, c'a'))

    arktest.assertTrue(le(c'a', 99.2f))
    arktest.assertTrue(le(c'b', 98f))
    arktest.assertFalse(le(c'c', 97.2f))

    arktest.assertTrue(le(97.2f, c'c'))
    arktest.assertTrue(le(98f, c'b'))
    arktest.assertFalse(le(99.2f, c'a'))

    arktest.assertTrue(le(c'a', 99.1))
    arktest.assertTrue(le(c'b', 98.0))
    arktest.assertFalse(le(c'c', 97.1))

    arktest.assertTrue(le(97.1, c'c'))
    arktest.assertTrue(le(98.0, c'b'))
    arktest.assertFalse(le(99.1, c'a'))

    // gt
    arktest.assertFalse(gt(c'a', c'c'))
    arktest.assertFalse(gt(c'b', c'b'))
    arktest.assertTrue(gt(c'c', c'a'))

    arktest.assertFalse(gt(c'a', 99))
    arktest.assertFalse(gt(c'b', 98))
    arktest.assertTrue(gt(c'c', 97))

    arktest.assertFalse(gt(97, c'c'))
    arktest.assertFalse(gt(98, c'b'))
    arktest.assertTrue(gt(99, c'a'))

    arktest.assertFalse(gt(c'a', 99.2f))
    arktest.assertFalse(gt(c'b', 98f))
    arktest.assertTrue(gt(c'c', 97.2f))

    arktest.assertFalse(gt(97.2f, c'c'))
    arktest.assertFalse(gt(98f, c'b'))
    arktest.assertTrue(gt(99.2f, c'a'))

    arktest.assertFalse(gt(c'a', 99.1))
    arktest.assertFalse(gt(c'b', 98.0))
    arktest.assertTrue(gt(c'c', 97.1))

    arktest.assertFalse(gt(97.1, c'c'))
    arktest.assertFalse(gt(98.0, c'b'))
    arktest.assertTrue(gt(99.1, c'a'))

    // ge
    arktest.assertFalse(ge(c'a', c'c'))
    arktest.assertTrue(ge(c'b', c'b'))
    arktest.assertTrue(ge(c'c', c'a'))

    arktest.assertFalse(ge(c'a', 99))
    arktest.assertTrue(ge(c'b', 98))
    arktest.assertTrue(ge(c'c', 97))

    arktest.assertFalse(ge(97, c'c'))
    arktest.assertTrue(ge(98, c'b'))
    arktest.assertTrue(ge(99, c'a'))

    arktest.assertFalse(ge(c'a', 99.2f))
    arktest.assertTrue(ge(c'b', 98f))
    arktest.assertTrue(ge(c'c', 97.2f))

    arktest.assertFalse(ge(97.2f, c'c'))
    arktest.assertTrue(ge(98f, c'b'))
    arktest.assertTrue(ge(99.2f, c'a'))

    arktest.assertFalse(ge(c'a', 99.1))
    arktest.assertTrue(ge(c'b', 98.0))
    arktest.assertTrue(ge(c'c', 97.1))

    arktest.assertFalse(ge(97.1, c'c'))
    arktest.assertTrue(ge(98.0, c'b'))
    arktest.assertTrue(ge(99.1, c'a'))
}
