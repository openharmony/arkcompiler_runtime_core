/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    Each predefined value type has a corresponding predefined class type (called boxed type)
    that wraps the value of the predefined value type: Number, Byte, Short, Int, Long,
    Float, Double, Char, and Boolean.
tags: [{{c.tags}}]
---*/
let boxed: {{c.to_type}}

function main() {
{%- for t in c['values'] %}
    let prim{{loop.index}}: {{c.from_type}} = {{t.expr|safe}}
    boxed = new {{c.to_type}}(prim{{loop.index}})
    assertEQ( boxed.unboxed(), {{t.val}} )
{%- endfor %}
}
{%- endfor %}
