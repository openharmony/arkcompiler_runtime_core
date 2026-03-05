/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

{% for c in cases %}
{% set nsglob = namespace() %}

{% if 'check_variants' in c %}
  {% set check_variants=c.check_variants %}
{% else %}
  {% set check_variants=[{}] %}
{% endif %}

{% for cv in check_variants %}
{% set cv_id = loop.index0 %}

{% if 'sub_cases_logic' in c %}
  {% set sub_cases_logic=c.sub_cases_logic %}
{% else %}
  {% set sub_cases_logic=[""] %}
{% endif %}

{% for scl in sub_cases_logic %}
{% set scl_id = loop.index0 %}

{% set logic_data -%}
{% if 'logic' in c %}  {{c.logic|indent(width=2)}}{% endif -%}
{% if scl != "" %}  {{scl|indent(width=2)}}{% endif -%}
{% endset -%}

{% set ns = namespace() %}

{% set ns.logic_data = logic_data %}
{% set ns.expected_out = c.stdout[scl_id] %}
{% for k, v in cv.items() %}
  {% set ns.logic_data = ns.logic_data|replace("[" ~ k ~ "]", v) %}
  {% set ns.expected_out = ns.expected_out|replace("[" ~ k ~ "]", v) %}
{% endfor %}
{% set logic_data=ns.logic_data %}

{% set ns.decl = c.decl %}
{% for k, v in cv.items() %}
  {% set ns.decl = ns.decl|replace("[" ~ k ~ "]", v) %}
{% endfor %}
{% set decl=ns.decl %}
/*---
desc: {{c.desc}}
tags: [negative, compile-only]
expected_out: {{ns.expected_out}}
---*/

//#4097 (urunner2) note: use this after declgen run support
//test suffix: "{{c.test_suffix}}_{{cv_id}}_{{scl_id}}"

{{decl}}

function main(): void {
{{logic_data}}
}

{% endfor %}
{% endfor %}
{% endfor %}
