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

{% for c in cases %}

/*---
desc: >-
    Class, interface, and enumeration members can only be accessed 
    by applying the dot operator to an instance or to a type.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0143: Unresolved reference {{c.member}}"
---*/

class C {
    static x: int = 1
}

interface I {
    y: int
}

enum E { A, B }

function main(): void {
    let a = {{ c.member }}
}

{% endfor %}
