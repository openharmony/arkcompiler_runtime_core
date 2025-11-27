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

{% yaml "branching_on_typeof_v_strict_equal_byte_common.yaml" as common_test_data %}

/*---
desc: >-
    Checking branching on "typeof v === "byte""
    in function call and with smart type compatibility check
---*/

{% for c in common_test_data.cases %}
{%- set c_id = loop.index %}

namespace testNS{{c_id}} {
  {{c.check_variant_decl_data|indent(width=2)}}

  // Function for check that variable of computed smart type after branching on
  // "typeof v === "byte"" is byte type on the positive branch
  function check_as_type_byte(v: byte, v_id: int): void {
    arktest.assertEQ(v, check_values[v_id][0])
  }

  // Function for check that variable of computed smart type after branching on
  // "typeof v === "byte"" is assignable to type A - byte on the negative branch
  function check_as_type_A_diff_ByteType(v: A_diff_ByteType,
                                          v_id: int): void {
    arktest.assertEQ(v, check_values[v_id][0])
  }

  // Function for check values of variables of type A with branching on "typeof v === "byte""
  function check(v: A,
                  v_type: VarType,
                  v_id: int): void {
    if (typeof v === "byte") {
      arktest.assertEQ(v_type, VarType.ByteType)
      {{c.check_value_by_expression_on_the_positive_branch}}
      check_as_type_byte(v, v_id)
      arktest.assertTrue(v instanceof byte)
      arktest.assertEQ(v, check_values[v_id][0])
    }
    else {
      arktest.assertEQ(v_type, VarType.A_diff_ByteType)
      {{c.check_value_by_expression_on_the_negative_branch}}
      check_as_type_A_diff_ByteType(v, v_id)
    }
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
  let testSuite = new arktest.ArkTestsuite('branching_on_typeof_v_strict_equal_byte_in_fn');
  {%- for c in common_test_data.cases %}
  {%- set c_id = loop.index %}
  testSuite.addTest('testNS{{c_id}}.runTest()', testNS{{c_id}}.runTest)
  {%- endfor %}
  let res = testSuite.run()
  return res
}
