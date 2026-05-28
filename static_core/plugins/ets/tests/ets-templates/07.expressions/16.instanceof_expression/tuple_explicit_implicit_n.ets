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

{% yaml "tuple_explicit_implicit.params.yaml" as data %}
{% for c in data.cases %}

{% set ctype = c.instanceof_type or c.type %}
/*---
desc: |-
    Tuple type erasure: instanceof with a tuple type checks only the number of elements (arity).
    Element types are erased at runtime. Only instanceof Tuple is valid
tags: [compile-only, negative]
expected_out: "Syntax error ESY18871: Type parameter is erased"
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

    pass_through(a1) instanceof {{ctype}}

    pass_through(a1) instanceof other_tuple_same_arity

    pass_through(a1) instanceof other_tuple_greater_arity

    pass_through(a1) instanceof other_tuple_lesser_arity

    let a2 = {{c.init}}
    pass_through(a2) instanceof {{ctype}}|Array
}
{% endfor %}
