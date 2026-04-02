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

{% for modifier in ["let", "const"] %}
{% for v in vals %}
{% for op in operations %}
{% for order in ["forward", "backward"] %}

/*---
desc: >-
    Attempting to mix char values and numeric values as an operand in a
    relational expression causes a compile-time error
tags: [compile-only, negative]
test_suffix: "{{modifier}}_{{v.type}}_{{op.name}}_{{order}}"
expected_out: "Semantic error ESE0346: Wrong type of operands for binary expression"
---*/

function main() {
    {{modifier}} a0: char = c'a'
    {{modifier}} x: {{v.type}} = {{v.init}}

    {% if 'forward' in order %}
        let res = a0 {{op.sign}} x
    {% else %}
        let res = x {{op.sign}} a0
    {% endif %}
}
{%- endfor %}
{%- endfor %}
{%- endfor %}
{%- endfor %}
