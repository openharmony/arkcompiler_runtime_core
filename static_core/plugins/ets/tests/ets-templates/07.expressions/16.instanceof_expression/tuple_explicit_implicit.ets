/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

{% for c in cases %}
/*---
desc: |-
    Tuple type erasure: instanceof with a tuple type checks only the number of elements (arity).
    Element types are erased at runtime. A tuple of arity N matches any tuple type of arity N
    regardless of element types.
---*/

class OtherType1 {}
class OtherType2 {}
class OtherType3 {}
type other_tuple_same_arity = [OtherType1, OtherType2]
type other_tuple_greater_arity = [OtherType1, OtherType2, OtherType3]
type other_tuple_lesser_arity = [OtherType1]

{{c.decl}}

function pass_through(value: Any): Any { return value }

function main() {
    let a1: {{c.type}} = {{c.init}}

    // Positive: explicit tuple matches its own type (arity check)
    arktest.assertTrue(pass_through(a1) instanceof ({{c.instanceof_type or c.type}}),
        "failed instanceof own type for {{c.type}}")

    // Positive: explicit tuple matches Tuple supertype
    arktest.assertTrue(pass_through(a1) instanceof Tuple,
        "failed instanceof Tuple for {{c.type}}")

    // Positive (type erasure): same arity, different element types -> still matches
    arktest.assertTrue(pass_through(a1) instanceof (other_tuple_same_arity),
        "type erasure: {{c.type}} should match other 2-element tuple")

    // Negative: greater arity -> does not match
    arktest.assertFalse(pass_through(a1) instanceof (other_tuple_greater_arity),
        "failed negative (greater arity) for {{c.type}}")

    // Negative: lesser arity -> does not match
    arktest.assertTrue(pass_through(a1) instanceof (other_tuple_lesser_arity),
        "failed instanceof (lesser arity)")

    let a2 = {{c.init}}
    // Implicit type: without explicit type annotation this syntax can produce a tuple or an array of unions
    arktest.assertTrue(pass_through(a2) instanceof ({{c.instanceof_type or c.type}}|Array),
        "failed instanceof for implicit type {{c.type}}")
}
{% endfor %}
