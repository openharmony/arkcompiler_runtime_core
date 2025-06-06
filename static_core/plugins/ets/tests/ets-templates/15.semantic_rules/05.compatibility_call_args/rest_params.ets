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
desc: Type of each argument corresponding to the rest parameter without the spread operator
      must be compatible with the element type of the array rest type parameter.
---*/

{{ c.defs }}

class A {
    field: {{ c.to_type }};
    constructor(...a: {{ c.to_type }}) {
        this.field = a;
    }

    bar(...callee: {{ c.to_type }}): {{ c.to_type }} {
        return callee;
    }
}

function foo(...callee: {{ c.to_type }}): {{ c.to_type }} {
    return callee;
}

function main() {

    let a: A = new A({{ c.value }});
    let method_result = a.bar({{ c.value }});
    let func_result = foo({{ c.value }});

    {%- for res in c['result'] %}
    assertEQ(method_result[{{ loop.index0 }}], {{ res }})
    assertEQ(func_result[{{ loop.index0 }}], {{ res }})
    assertEQ(a.field[{{ loop.index0 }}], {{ res }})
    {%- endfor %}
}

{% endfor %}
