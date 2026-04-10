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

{% for c in cases %}

{% for decl in c.decl_variants %}

{% set check_variants=c.check_variants %}
{% for cv in check_variants %}

{% set ns = namespace() %}

{% set ns.decl = decl %}
{% for k, v in cv.items() %}
  {% set ns.decl = ns.decl|replace("[" ~ k ~ "]", v) %}
{% endfor %}
{% set decl=ns.decl %}

/*---
desc: {{c.desc}}
assert: >-
    If the instance field initializer expression uses
    this or super directly in any form,then a compile-time warning occurs.
expected_out: "Warning W0010: The instance field initializer expression cannot use the super."
---*/

{{decl}}

function main(): void {
  arktest.assertEQ(new A().field, {{cv.check_value}} )
}

{% endfor %}
{% endfor %}
{% endfor %}
