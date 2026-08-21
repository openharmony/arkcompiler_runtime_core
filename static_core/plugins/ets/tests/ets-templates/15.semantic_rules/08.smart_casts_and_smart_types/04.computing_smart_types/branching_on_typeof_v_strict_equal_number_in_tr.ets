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

{% yaml "branching_on_typeof_v_strict_equal_number_common.yaml" as common_test_data %}

/*---
desc: >-
    Checking branching on "typeof v === "number""
    with ternary expression usage in function call
    and with smart type compatibility check
---*/

{% for c in common_test_data.cases %}
{%- set c_id = loop.index %}

namespace testNS{{c_id}} {
  {{c.check_variant_decl_data|indent(width=2)}}

  // Function for check that variable of computed smart type after branching on
  // "typeof v === "number"" is number type on the positive branch
  function check_as_type_number(v: number, v_id: int): void {
    arktest.assertEQ(check_values[v_id][1], VarType.NumberType)
    arktest.assertEQ(v, check_values[v_id][0])
  }

  // Function for check that variable of computed smart type after branching on
  // "typeof v === "number"" is assignable to type A - number on the negative branch
  function check_as_type_A_diff_NumberType(v: A_diff_NumberType,
                                          v_id: int): void {
    arktest.assertEQ(check_values[v_id][1], VarType.A_diff_NumberType)
    arktest.assertEQ(v, check_values[v_id][0])
  }

  // Function for check values of variables of type A with branching on "typeof v === "number""
  function check(v: A,
                  v_type: VarType,
                  v_id: int): void {
    (typeof v === "number") ?
      {{c.check_value_by_expression_on_the_positive_branch}} : {{c.check_value_by_expression_on_the_negative_branch}};
    (typeof v === "number") ?
      check_as_type_number(v, v_id) : check_as_type_A_diff_NumberType(v, v_id);
  }

  export function runTest() {
    let checks_number: int = 0
    for (let v_id: int = 0; v_id < check_values.length; v_id++) {
      check(check_values[v_id][0],
        check_values[v_id][1],
        v_id)
      checks_number++
    }
    arktest.assertEQ(checks_number, check_values_number)
  }
}

{%- endfor %}

function main(): int {
  let testSuite = new arktest.ArkTestsuite('branching_on_typeof_v_strict_equal_number_in_tr');
  {%- for c in common_test_data.cases %}
  {%- set c_id = loop.index %}
  testSuite.addTest('testNS{{c_id}}.runTest()', testNS{{c_id}}.runTest)
  {%- endfor %}
  let res = testSuite.run()
  return res
}
