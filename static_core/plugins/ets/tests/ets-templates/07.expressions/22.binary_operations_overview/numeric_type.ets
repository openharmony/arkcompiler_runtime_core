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
desc: Binary operations types per table in chapter 7.22 for numeric types and bigint.
test_suffix: "{{operator.name}}"
---*/

function main(): int {
  let testSuite = new arktest.ArkTestsuite("numeric_type_{{operator.name}}");

  {% for type1 in line['type1'] %}
    {% for type2 in line['type2'] %}
        testSuite.addTest("numeric_type_{{operator.name}}_{{type1}}_{{type2}}", () => {
          let operand1: {{type1}} = 1{{line.bigint_suffix}}
          let operand2: {{type2}} = 1{{line.bigint_suffix}}

          let result1 = operand1 {{operator.sign}} operand2

          arktest.assertTrue(result1 instanceof {{line.result_type}},
            "At <{{type1}}> {{operator.sign}} <{{type2}}> expected result type <{{line.result_type}}>")

          {% if "true" not in line.order_matters %}
              let result2 = operand2 {{operator.sign}} operand1

              arktest.assertTrue(result2 instanceof {{line.result_type}},
                "At <{{type2}}> {{operator.sign}} <{{type1}}> expected result type <{{line.result_type}}>")
          {% endif %}
        });
    {% endfor %}
  {% endfor %}

    return testSuite.run()
}

  {% endfor %}
{% endfor %}
