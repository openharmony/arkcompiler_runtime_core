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
{%- set ns = namespace(idx=0) %}
{%- for c in yaml_file.cases %}
{%- for v in c['values'] %}
{%- set ns.idx = ns.idx + 1 %}
{%- set vs = v|string %}
{%- if 'as byte' in vs %}
{%-   set ptype = 'byte' %}
{%-   set tname = 'Byte' %}
{%- elif 'as short' in vs %}
{%-   set ptype = 'short' %}
{%-   set tname = 'Short' %}
{%- elif 'as long' in vs %}
{%-   set ptype = 'long' %}
{%-   set tname = 'Long' %}
{%- elif vs.endswith('f') or 'Float.' in vs %}
{%-   set ptype = 'float' %}
{%-   set tname = 'Float' %}
{%- elif '.' in vs or 'Double.' in vs or 'e' in vs %}
{%-   set ptype = 'double' %}
{%-   set tname = 'Double' %}
{%- elif vs.replace('-', '').isdigit() and (vs|int > 2147483647 or vs|int < -2147483648) %}
{%-   set ptype = 'long' %}
{%-   set tname = 'Long' %}
{%- else %}
{%-   set ptype = 'int' %}
{%-   set tname = 'Int' %}
{%- endif %}

/*---
desc: CTE - BigInt <= comparison via function call with {{tname}}
tags: [compile-only, negative]
expected_error: "Semantic error ESE0105: Operator '<=' cannot be applied to types 'BigInt' and '{{tname}}'."
---*/
function le_1_{{ns.idx}}(a: bigint, b: {{ptype}}, diff: bigint) {
    let _ = a + diff <= b;
    let _ = a <= b;
    let _ = a - diff <= b;
}
function main() {
    le_1_{{ns.idx}}({{c.bigint}}, {{vs}}, {{c.diff or '1n'}})
}
{%- endfor %}
{%- endfor %}