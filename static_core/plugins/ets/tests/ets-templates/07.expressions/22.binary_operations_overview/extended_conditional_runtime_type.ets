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

{% for case1 in cases %}

/*---
desc: |-
  Extended semantics for logical operators ‘A && B’, ‘A || B’ as per chapter 15.14.1.
  If the value of A is unknown at compile time, then the type of an expression is union A | B.
  Union in tests here is verified by assignability to all involved types.
test_suffix: "{{case1.type}}"
---*/

class Cls {}
enum EAB {A, B}
enum ES {a = "", b = "1"}
enum EN {a = 0, b = 1}

// type aliases for test suffix support
type union_example = boolean|string
type array_example = int[]
type func_example = () => void
function foo() {}
type tuple_example = [boolean, int]

function main(): int {
  let testSuite = new arktest.ArkTestsuite("extended_conditional_runtime_type_{{case1.type}}");

  {% for case2 in cases %}
    testSuite.addTest("case_extended_conditional_runtime_type_{{case1.type}}_{{case2.type}}", () => {
      let runtime_cond: boolean = Math.random() > 0.5
      let operand1true: {{case1.type}} = {{case1.init_t}}
      let operand1false: {{case1.type}} = {{case1.init_f}}
      let operand1runtime: {{case1.type}} = runtime_cond ? operand1true : operand1false

      let operand2: {{case2.type}} = {{case2.init_t}}

      // test using assignability to verify true union
      let rt_and = operand1runtime && operand2
      rt_and = operand1runtime
      rt_and = operand2

      let rt_or = operand1runtime || operand2
      rt_or = operand1runtime
      rt_or = operand2
    });
  {% endfor %}

    return testSuite.run();
}
{% endfor %}
