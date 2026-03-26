/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

{% for decl in c.decl_variants %}
{% set decl_id = loop.index0 %}

{% for scl in c.sub_cases_logic %}
{% set scl_id = loop.index0 %}

/*---
desc: {{c.desc}}
params: {{c.params}}
---*/

//#4097 (urunner2) note: use this after declgen run support
//test suffix: "{{c.test_suffix}}_{{decl_id}}_{{scl_id}}"

{{decl}}

function main(): void {
  {{scl|indent(width=2)}}
}

{% endfor %}
{% endfor %}
{% endfor %}
