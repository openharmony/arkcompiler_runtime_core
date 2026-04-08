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

function lt(p1: char, p2: byte) { return p1 < p2 }
function le(p1: char, p2: byte) { return p1 <= p2 }
function gt(p1: char, p2: byte) { return p1 > p2 }
function ge(p1: char, p2: byte) { return p1 >= p2 }

function lt(p1: byte, p2: char) { return p1 < p2 }
function le(p1: byte, p2: char) { return p1 <= p2 }
function gt(p1: byte, p2: char) { return p1 > p2 }
function ge(p1: byte, p2: char) { return p1 >= p2 }

function lt(p1: char, p2: short) { return p1 < p2 }
function le(p1: char, p2: short) { return p1 <= p2 }
function gt(p1: char, p2: short) { return p1 > p2 }
function ge(p1: char, p2: short) { return p1 >= p2 }

function lt(p1: short, p2: char) { return p1 < p2 }
function le(p1: short, p2: char) { return p1 <= p2 }
function gt(p1: short, p2: char) { return p1 > p2 }
function ge(p1: short, p2: char) { return p1 >= p2 }

function lt(p1: char, p2: int) { return p1 < p2 }
function le(p1: char, p2: int) { return p1 <= p2 }
function gt(p1: char, p2: int) { return p1 > p2 }
function ge(p1: char, p2: int) { return p1 >= p2 }

function lt(p1: int, p2: char) { return p1 < p2 }
function le(p1: int, p2: char) { return p1 <= p2 }
function gt(p1: int, p2: char) { return p1 > p2 }
function ge(p1: int, p2: char) { return p1 >= p2 }

function lt(p1: char, p2: long) { return p1 < p2 }
function le(p1: char, p2: long) { return p1 <= p2 }
function gt(p1: char, p2: long) { return p1 > p2 }
function ge(p1: char, p2: long) { return p1 >= p2 }

function lt(p1: long, p2: char) { return p1 < p2 }
function le(p1: long, p2: char) { return p1 <= p2 }
function gt(p1: long, p2: char) { return p1 > p2 }
function ge(p1: long, p2: char) { return p1 >= p2 }

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

function main() {
    // lt
    arktest.assertTrue(lt(c1, c3))
    arktest.assertFalse(lt(c2, c2))
    arktest.assertFalse(lt(c3, c1))

    arktest.assertTrue(lt(c1, b3))
    arktest.assertFalse(lt(c2, b2))
    arktest.assertFalse(lt(c3, b1))

    arktest.assertTrue(lt(b1, c3))
    arktest.assertFalse(lt(b2, c2))
    arktest.assertFalse(lt(b3, c1))

    arktest.assertTrue(lt(c1, s3))
    arktest.assertFalse(lt(c2, s2))
    arktest.assertFalse(lt(c3, s1))

    arktest.assertTrue(lt(s1, c3))
    arktest.assertFalse(lt(s2, c2))
    arktest.assertFalse(lt(s3, c1))

    arktest.assertTrue(lt(c1, i3))
    arktest.assertFalse(lt(c2, i2))
    arktest.assertFalse(lt(c3, i1))

    arktest.assertTrue(lt(i1, c3))
    arktest.assertFalse(lt(i2, c2))
    arktest.assertFalse(lt(i3, c1))

    arktest.assertTrue(lt(c1, l3))
    arktest.assertFalse(lt(c2, l2))
    arktest.assertFalse(lt(c3, l1))

    arktest.assertTrue(lt(l1, c3))
    arktest.assertFalse(lt(l2, c2))
    arktest.assertFalse(lt(l3, c1))

    arktest.assertTrue(lt(c1, f3))
    arktest.assertFalse(lt(c2, f2))
    arktest.assertFalse(lt(c3, f1))

    arktest.assertTrue(lt(f1, c3))
    arktest.assertFalse(lt(f2, c2))
    arktest.assertFalse(lt(f3, c1))

    arktest.assertTrue(lt(c1, d3))
    arktest.assertFalse(lt(c2, d2))
    arktest.assertFalse(lt(c3, d1))

    arktest.assertTrue(lt(d1, c3))
    arktest.assertFalse(lt(d2, c2))
    arktest.assertFalse(lt(d3, c1))

    // le
    arktest.assertTrue(le(c1, c3))
    arktest.assertTrue(le(c2, c2))
    arktest.assertFalse(le(c3, c1))

    arktest.assertTrue(le(c1, b3))
    arktest.assertTrue(le(c2, b2))
    arktest.assertFalse(le(c3, b1))

    arktest.assertTrue(le(b1, c3))
    arktest.assertTrue(le(b2, c2))
    arktest.assertFalse(le(b3, c1))

    arktest.assertTrue(le(c1, s3))
    arktest.assertTrue(le(c2, s2))
    arktest.assertFalse(le(c3, s1))

    arktest.assertTrue(le(s1, c3))
    arktest.assertTrue(le(s2, c2))
    arktest.assertFalse(le(s3, c1))

    arktest.assertTrue(le(c1, i3))
    arktest.assertTrue(le(c2, i2))
    arktest.assertFalse(le(c3, i1))

    arktest.assertTrue(le(i1, c3))
    arktest.assertTrue(le(i2, c2))
    arktest.assertFalse(le(i3, c1))

    arktest.assertTrue(le(c1, l3))
    arktest.assertTrue(le(c2, l2))
    arktest.assertFalse(le(c3, l1))

    arktest.assertTrue(le(l1, c3))
    arktest.assertTrue(le(l2, c2))
    arktest.assertFalse(le(l3, c1))

    arktest.assertTrue(le(c1, f3))
    arktest.assertTrue(le(c2, f2))
    arktest.assertFalse(le(c3, f1))

    arktest.assertTrue(le(f1, c3))
    arktest.assertTrue(le(f2, c2))
    arktest.assertFalse(le(f3, c1))

    arktest.assertTrue(le(c1, d3))
    arktest.assertTrue(le(c2, d2))
    arktest.assertFalse(le(c3, d1))

    arktest.assertTrue(le(d1, c3))
    arktest.assertTrue(le(d2, c2))
    arktest.assertFalse(le(d3, c1))

    // gt
    arktest.assertFalse(gt(c1, c3))
    arktest.assertFalse(gt(c2, c2))
    arktest.assertTrue(gt(c3, c1))

    arktest.assertFalse(gt(c1, b3))
    arktest.assertFalse(gt(c2, b2))
    arktest.assertTrue(gt(c3, b1))

    arktest.assertFalse(gt(b1, c3))
    arktest.assertFalse(gt(b2, c2))
    arktest.assertTrue(gt(b3, c1))

    arktest.assertFalse(gt(c1, s3))
    arktest.assertFalse(gt(c2, s2))
    arktest.assertTrue(gt(c3, s1))

    arktest.assertFalse(gt(s1, c3))
    arktest.assertFalse(gt(s2, c2))
    arktest.assertTrue(gt(s3, c1))

    arktest.assertFalse(gt(c1, i3))
    arktest.assertFalse(gt(c2, i2))
    arktest.assertTrue(gt(c3, i1))

    arktest.assertFalse(gt(i1, c3))
    arktest.assertFalse(gt(i2, c2))
    arktest.assertTrue(gt(i3, c1))

    arktest.assertFalse(gt(c1, l3))
    arktest.assertFalse(gt(c2, l2))
    arktest.assertTrue(gt(c3, l1))

    arktest.assertFalse(gt(l1, c3))
    arktest.assertFalse(gt(l2, c2))
    arktest.assertTrue(gt(l3, c1))

    arktest.assertFalse(gt(c1, f3))
    arktest.assertFalse(gt(c2, f2))
    arktest.assertTrue(gt(c3, f1))

    arktest.assertFalse(gt(f1, c3))
    arktest.assertFalse(gt(f2, c2))
    arktest.assertTrue(gt(f3, c1))

    arktest.assertFalse(gt(c1, d3))
    arktest.assertFalse(gt(c2, d2))
    arktest.assertTrue(gt(c3, d1))

    arktest.assertFalse(gt(d1, c3))
    arktest.assertFalse(gt(d2, c2))
    arktest.assertTrue(gt(d3, c1))

    // ge
    arktest.assertFalse(ge(c1, c3))
    arktest.assertTrue(ge(c2, c2))
    arktest.assertTrue(ge(c3, c1))

    arktest.assertFalse(ge(c1, b3))
    arktest.assertTrue(ge(c2, b2))
    arktest.assertTrue(ge(c3, b1))

    arktest.assertFalse(ge(b1, c3))
    arktest.assertTrue(ge(b2, c2))
    arktest.assertTrue(ge(b3, c1))

    arktest.assertFalse(ge(c1, s3))
    arktest.assertTrue(ge(c2, s2))
    arktest.assertTrue(ge(c3, s1))

    arktest.assertFalse(ge(s1, c3))
    arktest.assertTrue(ge(s2, c2))
    arktest.assertTrue(ge(s3, c1))

    arktest.assertFalse(ge(c1, i3))
    arktest.assertTrue(ge(c2, i2))
    arktest.assertTrue(ge(c3, i1))

    arktest.assertFalse(ge(i1, c3))
    arktest.assertTrue(ge(i2, c2))
    arktest.assertTrue(ge(i3, c1))

    arktest.assertFalse(ge(c1, l3))
    arktest.assertTrue(ge(c2, l2))
    arktest.assertTrue(ge(c3, l1))

    arktest.assertFalse(ge(l1, c3))
    arktest.assertTrue(ge(l2, c2))
    arktest.assertTrue(ge(l3, c1))

    arktest.assertFalse(ge(c1, f3))
    arktest.assertTrue(ge(c2, f2))
    arktest.assertTrue(ge(c3, f1))

    arktest.assertFalse(ge(f1, c3))
    arktest.assertTrue(ge(f2, c2))
    arktest.assertTrue(ge(f3, c1))

    arktest.assertFalse(ge(c1, d3))
    arktest.assertTrue(ge(c2, d2))
    arktest.assertTrue(ge(c3, d1))

    arktest.assertFalse(ge(d1, c3))
    arktest.assertTrue(ge(d2, c2))
    arktest.assertTrue(ge(d3, c1))
}
