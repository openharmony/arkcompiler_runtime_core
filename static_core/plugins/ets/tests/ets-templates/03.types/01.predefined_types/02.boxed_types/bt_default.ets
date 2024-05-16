/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
desc: >-
    Each predefined value type has a corresponding predefined class type (called boxed type)
    that wraps the value of the predefined value type: Number, Byte, Short, Int, Long,
    Float, Double, Char, and Boolean.
---*/

let b = new Byte()
let s = new Short()
let i = new Int()
let l = new Long()
let f = new Float()
let d = new Double()
let n = new Number()
let c = new Char()
let bool = new Boolean()

function main() {
    assert b.unboxed() == 0
    assert s.unboxed() == 0
    assert i.unboxed() == 0
    assert l.unboxed() == 0
    assert f.unboxed() == 0.0f
    assert d.unboxed() == 0.0
    assert n.unboxed() == 0.0
    assert c.unboxed() == c'\u0000'
    assert bool.unboxed() == false

    assert b == 0
    assert s == 0
    assert i == 0
    assert l == 0
    assert f == 0.0f
    assert d == 0.0
    assert n == 0.0
    assert c == c'\u0000'
    assert bool == false
}
