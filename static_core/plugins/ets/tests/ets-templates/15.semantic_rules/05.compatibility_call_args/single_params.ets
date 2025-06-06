/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Type of any argument (except arguments of a rest parameter) must be compatible
      with the type of the corresponding parameter for any function, method, or constructor call 
---*/

{{ c.defs }}

class A {
    field: {{ c.arg }};
    constructor(a: {{ c.arg }}) {
        this.field = a;
    }

    bar(callee: {{ c.arg }}): {{ c.arg }} {
        return callee;
    }
}

function foo(callee: {{ c.arg }}): {{ c.arg }} {
    return callee;
}

function main() {
    {%- for v in c['values'] %}
    let a{{loop.index}}: A = new A({{ v.val }});
    assertEQ(a{{loop.index}}.field, {{ v.result }})
    assertEQ(a{{loop.index}}.bar({{ v.val }}), {{ v.result }})
    assertEQ(foo({{ v.val }}), {{ v.result }})
    {%- endfor %}
}

{% endfor %}
