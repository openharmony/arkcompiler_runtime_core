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

{% yaml "values_of_diff_type_on_user_defined_common.yaml" as common_test_data %}

/*---
desc: >-
    Checking that the values of the difference type A - B
    are all values that belong to type A but not to type B
    by checking values of type A | B
    (which includes values from types A & B, A - B, B - A)
    in case of usage instanceof on user-defined types
    with function call and smart type compatibility check
---*/

{% for c in common_test_data.cases %}
{%- set c_id = loop.index %}

namespace testNS{{c_id}} {
  {{c.check_variant_decl_data|indent(width=2)}}

  // Checks that value of computed smart type A - B is assignable to type A_diff_B
  function check_as_type_A_diff_B(v: A_diff_B,
                                  v_type: VarType,
                                  v_id: int): void {
    arktest.assertEQ(v_type, VarType.A_diff_B)
    arktest.assertTrue(v instanceof A)
    arktest.assertFalse(v instanceof B)
    check_values[v_id][1]!(v, v_id)
  }

  // Checks that value of computed smart type B - A is assignable to type B_diff_A
  function check_as_type_B_diff_A(v: B_diff_A,
                                  v_type: VarType,
                                  v_id: int): void {
    arktest.assertEQ(v_type, VarType.B_diff_A)
    arktest.assertFalse(v instanceof A)
    arktest.assertTrue(v instanceof B)
    check_values[v_id][2]!(v, v_id)
  }

  // Checks values of type A | B
  function check(v: A | B,
                  v_type: VarType,
                  v_id: int): void {
    if (v instanceof A) {
      if (v instanceof B) {
        arktest.assertEQ(v_type, VarType.A_inter_B)
        arktest.assertTrue(v instanceof A)
        arktest.assertTrue(v instanceof B)
        arktest.assertEQ(v, check_values[v_id][0])
      }
      else {
        arktest.assertEQ(v_type, VarType.A_diff_B)
        arktest.assertTrue(v instanceof A)
        arktest.assertFalse(v instanceof B)
        {{c.check_value_by_expression_as_type_A_diff_B}}
        check_as_type_A_diff_B(v, v_type, v_id)
      }
    }
    else {
      arktest.assertEQ(v_type, VarType.B_diff_A)
      arktest.assertFalse(v instanceof A)
      arktest.assertTrue(v instanceof B)
      {{c.check_value_by_expression_as_type_B_diff_A}}
      check_as_type_B_diff_A(v, v_type, v_id)
    }
  }

  export function runTest() {
    let checks_number: int = 0
    for (let v_id: int = 0; v_id < check_values.length; v_id++) {
      check(check_values[v_id][0],
        check_values[v_id][3],
        v_id)
      checks_number++
    }
    arktest.assertEQ(checks_number, check_values_number)
  }
}

{%- endfor %}

function main(): int {
  let testSuite = new arktest.ArkTestsuite('values_of_diff_type_on_user_defined_in_fn');
  {%- for c in common_test_data.cases %}
  {%- set c_id = loop.index %}
  testSuite.addTest('testNS{{c_id}}.runTest()', testNS{{c_id}}.runTest)
  {%- endfor %}
  let res = testSuite.run()
  return res
}