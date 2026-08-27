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
{% yaml 'eq_groups.params_neg.yaml' as yaml_file %}
{%- for c in yaml_file.cases %}
{%- for v in c['values'] %}
{%- set vs = v|string %}
{%- if 'as byte' in vs %}
{%-   set tname = 'Byte' %}
{%- elif 'as short' in vs %}
{%-   set tname = 'Short' %}
{%- elif 'as long' in vs %}
{%-   set tname = 'Long' %}
{%- elif vs.endswith('f') or 'Float.' in vs %}
{%-   set tname = 'Float' %}
{%- elif '.' in vs or 'Double.' in vs or 'e' in vs %}
{%-   set tname = 'Double' %}
{%- elif vs.replace('-', '').isdigit() and (vs|int > 2147483647 or vs|int < -2147483648) %}
{%-   set tname = 'Long' %}
{%- else %}
{%-   set tname = 'Int' %}
{%- endif %}

/*---
desc: CTE - BigInt >= comparison with {{vs}}
tags: [compile-only, negative]
expected_error: "Semantic error ESE0105: Operator '>=' cannot be applied to types 'BigInt' and '{{tname}}'."
---*/
let a = {{c.bigint}}

function main() {
    let b = {{vs}};
    let _ = a + {{c.diff or '1n'}} >= b;
    let _ = a >= b;
    let _ = a - {{c.diff or '1n'}} >= b;
}
{%- endfor %}
{%- endfor %}
