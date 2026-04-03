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

{% for line in table_lines %}
  {% for operator in line['operator'] %}

/*---
desc: Binary operations types per table in chapter 7.22 for non-numeric types.
test_suffix: "{{operator.name}}"
---*/

{{line.decl}}

function main(): int {
  let testSuite = new arktest.ArkTestsuite("non_numeric_type_{{operator.name}}");

  {% for type1 in line['type1'] %}
    {% for type2 in line['type2'] %}
        testSuite.addTest("non_numeric_type_{{operator.name}}_{{type1.t}}_{{type2.t}}", () => {
          let operand1: {{type1.t}} = {{type1.i}}
          let operand2: {{type2.t}} = {{type2.i}}

          let result1 = operand1 {{operator.sign}} operand2

          arktest.assertTrue(result1 instanceof {{line.result_type}},
            "At <{{type1.t}}> {{operator.sign}} <{{type2.t}}> expected result type <{{line.result_type}}>")

          let result2 = operand2 {{operator.sign}} operand1

          arktest.assertTrue(result2 instanceof {{line.result_type}},
            "At <{{type2.t}}> {{operator.sign}} <{{type1.t}}> expected result type <{{line.result_type}}>")
        });
    {% endfor %}
  {% endfor %}

  return testSuite.run()
}
  {% endfor %}
{% endfor %}
