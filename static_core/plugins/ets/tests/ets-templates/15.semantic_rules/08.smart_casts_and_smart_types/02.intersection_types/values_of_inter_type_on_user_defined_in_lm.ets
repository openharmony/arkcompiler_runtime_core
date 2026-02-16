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

{% yaml "values_of_inter_type_on_user_defined_common.yaml" as common_test_data %}

/*---
desc: >-
    Checking that the values of intersection type A & B
    are all values that belong to both A and B
    by checking values of type A | B
    (which includes values from types A & B, A - B, B - A)
    in case of usage instanceof on user-defined types
    with lambda function call and smart type compatibility check
---*/

{% for c in common_test_data.cases %}
{%- set c_id = loop.index %}

namespace testNS{{c_id}} {
  {{c.check_variant_decl_data|indent(width=2)}}

  export function runTest() {
    // Checks that value of computed smart type A & B is assignable to TypeA
    let check_type_A_inter_B_as_TypeA = (v: TypeA,
                                          v_type: VarType,
                                          v_id: int) => {
      arktest.assertEQ(v_type, VarType.A_inter_B)
      arktest.assertTrue(v instanceof TypeA)
      arktest.assertTrue(v instanceof TypeB)
      check_values[v_id][1]!(v, v_id)
    }

    // Checks that value of computed smart type A & B is assignable to TypeB
    let check_type_A_inter_B_as_TypeB = (v: TypeB,
                                          v_type: VarType,
                                          v_id: int) => {
      arktest.assertEQ(v_type, VarType.A_inter_B)
      arktest.assertTrue(v instanceof TypeA)
      arktest.assertTrue(v instanceof TypeB)
      check_values[v_id][2]!(v, v_id)
    }

    // Checks values of type A | B
    let check = (v: TypeA | TypeB,
                  v_type: VarType,
                  v_id: int) => {
      if (v instanceof TypeA) {
        if (v instanceof TypeB) {
          arktest.assertEQ(v_type, VarType.A_inter_B)
          arktest.assertTrue(v instanceof TypeA)
          arktest.assertTrue(v instanceof TypeB)
          {{c.check_value_type_A_inter_B_by_expression_as_TypeA}}
          {{c.check_value_type_A_inter_B_by_expression_as_TypeB}}
          check_type_A_inter_B_as_TypeA(v, v_type, v_id)
          check_type_A_inter_B_as_TypeB(v, v_type, v_id)
        }
        else {
          arktest.assertEQ(v_type, VarType.A_diff_B)
          arktest.assertTrue(v instanceof TypeA)
          arktest.assertFalse(v instanceof TypeB)
          arktest.assertEQ(v, check_values[v_id][0])
        }
      }
      else {
        arktest.assertTrue(v instanceof TypeB)
        if (v instanceof TypeB) {
          arktest.assertEQ(v_type, VarType.B_diff_A)
          arktest.assertFalse(v instanceof TypeA)
          arktest.assertTrue(v instanceof TypeB)
          arktest.assertEQ(v, check_values[v_id][0])
        }
        else {
          arktest.assertTrue(false,
            'v should be instance of TypeB')
        }
      }
    }

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
  let testSuite = new arktest.ArkTestsuite('values_of_inter_type_on_user_defined_in_lm');
  {%- for c in common_test_data.cases %}
  {%- set c_id = loop.index %}
  testSuite.addTest('testNS{{c_id}}.runTest()', testNS{{c_id}}.runTest)
  {%- endfor %}
  let res = testSuite.run()
  return res
}