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
{%- for c in cases %}

/*---
desc: >-
    Conversion from a union type to a wider union type.
    Call context, function, widening union conversion, implicit
params: to union type {{c.to_type}}
---*/
{{c.decl}}

{%- for t in c['values'] %}
let s{{loop.index}}: {{t.from_type}} = {{t.expr|safe}};
{%- endfor %}

function foo(p: {{c.to_type}}): {{c.to_type}} {
    return p
}

function main() {
{%- for t in c['values'] %}
    assertTrue( foo(s{{loop.index}}) {{'as ' + t.ref_type if t.ref_type}} {{t.eq}} {{t.expr|safe}} )
{%- endfor %}
}
{%- endfor %}
