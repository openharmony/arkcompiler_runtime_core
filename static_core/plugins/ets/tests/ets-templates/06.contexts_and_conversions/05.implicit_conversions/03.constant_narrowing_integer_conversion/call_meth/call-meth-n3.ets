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
desc: Call context, method, constant narrowing integer conversion, implicit
tags: [compile-only, negative]
params: from {{c.from_type}} to {{c.to_type}}
---*/
{%- for v in c['values'] %}
const c{{loop.index}}: {{c.from_type}} = {{v.expr|safe}}
{%- endfor %}

class A {
    meth(p: {{c.to_type}}): {{c.to_type}} { return p }
}

function main(): int {
    let a: A = new A()
{%- for v in c['values'] %}
    if (a.meth({{v.expr|safe}}) != {{v.val}}) {  // expression
        return 1
    }
    if (a.meth(c{{loop.index}}) != {{v.val}}) {  // constant
        return 1
    }
{%- endfor %}
    return 0
}
{%- endfor %}
