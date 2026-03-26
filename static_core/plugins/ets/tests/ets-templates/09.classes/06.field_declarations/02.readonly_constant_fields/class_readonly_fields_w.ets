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

{% for timing in ["compile", "runtime"] %}

{% for c in cases %}

{% for decl in c.decl_variants %}
{% set cv_id = loop.index0 %}

{% if 'check_variants' in c %}
  {% set check_variants=c.check_variants %}
{% else %}
  {% set check_variants=[{}] %}
{% endif %}
{% for cv in check_variants %}

{% set ns = namespace() %}

{% set ns.decl = decl %}
{% for k, v in cv.items() %}
  {% set ns.decl = ns.decl|replace("[" ~ k ~ "]", v) %}
{% endfor %}
{% set decl=ns.decl %}

/*---
desc: {{c.desc}}
assert: {{c.assert}}
{% if 'compile' in timing %}
tags: [compile-only]
expected_out: {{c.expected_out[cv_id]}}
{% endif %}
---*/

// #2345 after runner is made to support this properly, remove compile-only tag and "timing" loop

{{decl}}

function main(): void {
}

{% endfor %}
{% endfor %}
{% endfor %}
{% endfor %}
