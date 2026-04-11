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

{% for op in operations %}
{% for pair in op.pairs %}

/*---
desc: Type combinations not listed in the table cause a compile-time error if a combination
      is detected at compile time, or a runtime error otherwise..
{% if pair.expect_cte is defined and not pair.expect_cte %}
tags: []
{% else %}
tags: [compile-only, negative]
{% endif %}
expected_out: {{pair.expected_out}}
test_suffix: "{{op.op.name}}_{{pair.type1.0}}_{{pair.type2.0}}"
---*/

{{op.decl}}

function main(): void {
  let x: {{pair.type1.0}} = {{pair.type1.1}};
  let y: {{pair.type2.0}} = {{pair.type2.1}};
  let z = x {{op.op.sign}} y;
}
{% endfor %}
{% endfor %}
