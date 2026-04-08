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

{% for type in [
   ["int", "0"],
   ["bigint", "0n"],
   ["Any", "0"],
   ["null", "null"],
   ["undefined", "undefined"],
   ["Object", "1"],
   ]%}
{% for decl_mod in ["let", "const"] %}

/*---
desc: Verify CTE for instanceof via reflection in the form 'value instanceof value_other'.
tags: [compile-only, negative]
expected_out: "Semantic error ESE0371: Cannot find type 'value_other'."
test_suffix: "{{type.0}}_{{decl_mod}}"
---*/

function main() {
    {{decl_mod}} value: {{type.0}} = {{type.1}}
    {{decl_mod}} value_other: {{type.0}} = {{type.1}}

    arktest.assertTrue(value_let instanceof value_other, "expect CTE here")
}
{% endfor %}
{% endfor %}
