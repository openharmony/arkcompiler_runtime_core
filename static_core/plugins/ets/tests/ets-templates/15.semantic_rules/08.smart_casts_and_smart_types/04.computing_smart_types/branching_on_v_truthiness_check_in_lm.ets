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

{% yaml "branching_on_v_truthiness_check_common.yaml" as common_test_data %}

{% for c in common_test_data.cases %}

/*---
desc: >-
    Checking branching on "v (truthiness check)"
    in lambda function call and with smart type compatibility check
---*/

{{c.check_variant_decl_data}}

function main(): void {
  // Function for check that variable of computed smart type after branching on
  // "v (truthiness check)" is assignable to type A - U on the positive branch
  let check_as_type_A_diff_U = (v: A_diff_U,
                                  v_id: int) => {
    check_values[v_id][1]!(v, v_id)
  }
  // Function for check that variable of computed smart type after branching on
  // "v (truthiness check)" is assignable to type A & T on the negative branch
  let check_as_type_A_inter_T = (v: A_inter_T,
                                v_id: int) => {
    check_values[v_id][2]!(v, v_id)
  }
  // Function for check values of variables of type A with branching on "v (truthiness check)"
  let check = (v: A,
                v_type: VarType,
                v_id: int) => {
    if (v) {
      arktest.assertEQ(v_type, VarType.A_diff_U)
      {{c.check_value_by_expression_on_the_positive_branch}}
      check_as_type_A_diff_U(v, v_id)
    }
    else {
      arktest.assertEQ(v_type, VarType.A_inter_T)
      {{c.check_value_by_expression_on_the_negative_branch}}
      check_as_type_A_inter_T(v, v_id)
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

{% endfor %}
