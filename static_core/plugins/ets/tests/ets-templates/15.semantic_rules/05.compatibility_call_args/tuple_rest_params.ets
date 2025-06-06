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
desc: If a single argument corresponding to the rest parameter has the spread operator,
      then the expression that follows the operator must refer to a tuple of types compatible
    with the proper types of the tuple rest parameter
---*/

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

    let tuple: {{ c.to_type }} = {{ c.value }};
    let a: A = new A(...tuple);
    let method_result = a.bar(...tuple);
    let func_result = foo(...tuple);

    {%- for res in c['result'] %}
    assertEQ(method_result[{{ loop.index0 }}], {{ res }})
    assertEQ(func_result[{{ loop.index0 }}], {{ res }})
    assertEQ(a.field[{{ loop.index0 }}], {{ res }})
    {%- endfor %}
}

{% endfor %}
