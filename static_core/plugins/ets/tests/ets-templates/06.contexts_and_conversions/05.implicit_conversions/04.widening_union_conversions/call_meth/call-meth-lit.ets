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
    Conversion from a union type that consists of literals only to a non-union type.
    Call context, instance method, widening union conversion, implicit
params: union of literals to {{c.to_type}}
---*/
{{c.decl}}

{%- for t in c['values'] %}
let s{{loop.index}}: {{t.from_type}} = {{t.expr|safe}};
{%- endfor %}

class A {
    meth(p: {{c.to_type}}): {{c.to_type}} {
        return p
    }
}

function main() {
    let a: A = new A()
{%- for t in c['values'] %}
    assertTrue( a.meth(s{{loop.index}}) == {{t.expr|safe}} )
{%- endfor %}
}
{%- endfor %}
