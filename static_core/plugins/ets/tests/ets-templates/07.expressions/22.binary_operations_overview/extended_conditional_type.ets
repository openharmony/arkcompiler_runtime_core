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
desc: Extended semantics for logical operators ‘&&’, ‘||’ as per chapter 15.14.1.
---*/
//#4097 (urunner) note use this after declgen runs support test suffix: "{{case1.type}}"

// type alias for union for test suffix support
type union_example = boolean|string

let alwaysFalse1: boolean = false
let alwaysTrue1: boolean = false

{% if "true" in case1.always_false %}
  alwaysFalse1 = true
{% endif %}
{% if "true" in case1.always_true %}
  alwaysTrue1 = true
{% endif %}

class Cls {}
enum EAB {A, B}
enum ES {a = "", b = "1"}
enum EN {a = 0, b = 1}

function main(): int {
  let testSuite = new arktest.ArkTestsuite("extended_conditional_type_{{case1.type}}");

  {% for case2 in cases %}
    testSuite.addTest("extended_conditional_type_{{case1.type}}_{{case2.type}}", () => {
      let runtime_cond: boolean = Math.random() > 0.5

      let operand1true: {{case1.type}} = {{case1.init_t}}
      let operand1false: {{case1.type}} = {{case1.init_f}}
      let operand1runtime: {{case1.type}} = runtime_cond ? operand1true : operand1false

      let operand2true: {{case2.type}} = {{case2.init_t}}
      let operand2false: {{case2.type}} = {{case2.init_f}}

      let t_a_f = operand1true && operand2false
      let exp_t_a_f: string = alwaysFalse1 ? "{{case1.type}}" : "{{case2.type}}"
      arktest.assertTrue(alwaysFalse1 ? t_a_f instanceof {{case1.type}} : t_a_f instanceof {{case2.type}},
        "t_a_f <{{case1.type}}> && <{{case2.type}}> expected type <" +
        exp_t_a_f + "> actual <" + (typeof t_a_f) + ">")

      let t_o_f = operand1true || operand2false
      let exp_t_o_f: string = alwaysFalse1 ? "{{case2.type}}" : "{{case1.type}}"
      arktest.assertTrue(alwaysFalse1 ? t_o_f instanceof {{case2.type}} : t_o_f instanceof {{case1.type}},
        "t_o_f <{{case1.type}}> || <{{case2.type}}> expected type <" +
        exp_t_o_f + "> actual <" + (typeof t_o_f) + ">")

      let f_a_t = operand1false && operand2true
      let exp_f_a_t: string = alwaysTrue1 ? "{{case2.type}}" : "{{case1.type}}"
      arktest.assertTrue(alwaysTrue1 ? f_a_t instanceof {{case2.type}} : f_a_t instanceof {{case1.type}},
        "f_a_t <{{case1.type}}> && <{{case2.type}}> expected type <" +
        exp_f_a_t + "> actual <" + (typeof f_a_t) + ">")

      let f_o_t = operand1false || operand2true
      let exp_f_o_t: string = alwaysTrue1 ? "{{case1.type}}" : "{{case2.type}}"
      arktest.assertTrue(alwaysTrue1 ? f_o_t instanceof {{case1.type}} : f_o_t instanceof {{case2.type}},
        "f_o_t <{{case1.type}}> || <{{case2.type}}> expected type <" +
        exp_f_o_t + "> actual <" + (typeof f_o_t) + ">")

      let t_a_t = operand1true && operand2true
      let exp_t_a_t: string = alwaysFalse1 ? "{{case1.type}}" : "{{case2.type}}"
      arktest.assertTrue(alwaysFalse1 ? t_a_t instanceof {{case1.type}} : t_a_t instanceof {{case2.type}},
        "t_a_t <{{case1.type}}> && <{{case2.type}}> expected type <" +
        exp_t_a_t + "> actual <" + (typeof t_a_t) + ">")

      let t_o_t = operand1true || operand2true
      let exp_t_o_t: string = alwaysFalse1 ? "{{case2.type}}" : "{{case1.type}}"
      arktest.assertTrue(alwaysFalse1 ? t_o_t instanceof {{case2.type}} : t_o_t instanceof {{case1.type}},
        "t_o_t <{{case1.type}}> || <{{case2.type}}> expected type <" +
        exp_t_o_t + "> actual <" + (typeof t_o_t) + ">")

      let f_a_f = operand1false && operand2false
      let exp_f_a_f: string = alwaysTrue1 ? "{{case2.type}}" : "{{case1.type}}"
      arktest.assertTrue(alwaysTrue1 ? f_a_f instanceof {{case2.type}} : f_a_f instanceof {{case1.type}},
        "f_a_f <{{case1.type}}> && <{{case2.type}}> expected type <" +
        exp_f_a_f + "> actual <" + (typeof f_a_f) + ">")

      let f_o_f = operand1false || operand2false
      let exp_f_o_f: string = alwaysTrue1 ? "{{case1.type}}" : "{{case2.type}}"
      arktest.assertTrue(alwaysTrue1 ? f_o_f instanceof {{case1.type}} : f_o_f instanceof {{case2.type}},
        "f_o_f <{{case1.type}}> || <{{case2.type}}> expected type <" +
        exp_f_o_f + "> actual <" + (typeof f_o_f) + ">")

      let r_a_f = operand1runtime && operand2false
      arktest.assertTrue(r_a_f instanceof ({{case1.type}} | {{case2.type}}),
        "r_a_f <{{case1.type}}> && <{{case2.type}}> expected type <{{case1.type}} | {{case2.type}}> actual <" + (typeof r_a_f) + ">")

      let r_o_f = operand1runtime || operand2false
      arktest.assertTrue(r_o_f instanceof ({{case1.type}} | {{case2.type}}),
        "r_o_f <{{case1.type}}> || <{{case2.type}}> expected type <{{case1.type}} | {{case2.type}}> actual <" + (typeof r_o_f) + ">")

      let r_a_t = operand1runtime && operand2true
      arktest.assertTrue(r_a_t instanceof ({{case1.type}} | {{case2.type}}),
        "r_a_t <{{case1.type}}> && <{{case2.type}}> expected type <{{case1.type}} | {{case2.type}}> actual <" + (typeof r_a_t) + ">")

      let r_o_t = operand1runtime || operand2true
      arktest.assertTrue(r_o_t instanceof ({{case1.type}} | {{case2.type}}),
        "r_o_t <{{case1.type}}> || <{{case2.type}}> expected type <{{case1.type}} | {{case2.type}}> actual <" + (typeof r_o_t) + ">")
    });
  {% endfor %}

    return testSuite.run()
}

{% endfor %}
