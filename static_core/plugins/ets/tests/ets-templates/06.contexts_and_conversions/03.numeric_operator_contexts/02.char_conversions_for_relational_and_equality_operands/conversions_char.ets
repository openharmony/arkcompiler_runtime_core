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

/*---
desc: >-
    A char type operand is widened to one of the following:
    - Type int if the other operand is of types byte, char or int; or
    - Type of the other operand.
---*/

const cx: char = c'\u0001' // top-level constant
let tx: char = c'\u0001' // top-level variable
{% for t in types %}
const cy{{t[0]}}: {{t[0]}} = {{t[1]}}
let ty{{t[0]}}: {{t[0]}} = {{t[1]}}
{% endfor %}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("Context_widening_primitives_char");
    {% for t in types %}
    {% set outer_loop_index = loop.index %}
    {% for op in operators %}
        testSuite.addTest("conversion_char_to_{{outer_loop_index}}_{{loop.index}}", () => {
            let x: char = c'\u0001';
            let y: {{t[0]}} = {{t[1]}};
            arktest.assertEQ((x {{op[0]}} y), {{op[1]}})
            arktest.assertEQ((y {{op[0]}} x), {{op[1]}})
        });
        testSuite.addTest("conversion_const_char_to_{{outer_loop_index}}_{{loop.index}}", () => {
            const x: char = c'\u0001';
            const y: {{t[0]}} = {{t[1]}};
            arktest.assertEQ((x {{op[0]}} y), {{op[1]}})
            arktest.assertEQ((y {{op[0]}} x), {{op[1]}})
        });
        testSuite.addTest("conversion_literal_char_to_{{outer_loop_index}}_{{loop.index}}", () => {
            let y: {{t[0]}} = {{t[1]}};
            arktest.assertEQ(( c'\u0001' {{op[0]}} y), {{op[1]}})
            arktest.assertEQ((y {{op[0]}} c'\u0001' ), {{op[1]}})
        });
        testSuite.addTest("conversion_top_lvl_char_to_{{outer_loop_index}}_{{loop.index}}", () => {
            arktest.assertEQ(( tx {{op[0]}} ty{{t[0]}} ), {{op[1]}})
            arktest.assertEQ(( ty{{t[0]}} {{op[0]}} tx ), {{op[1]}})
        });
        testSuite.addTest("conversion_top_lvl_const_char_to_{{outer_loop_index}}_{{loop.index}}", () => {
            arktest.assertEQ(( cx {{op[0]}} cy{{t[0]}} ), {{op[1]}})
            arktest.assertEQ(( cy{{t[0]}} {{op[0]}} cx ), {{op[1]}})
        });
    {%- endfor %}
    {%- endfor %}

    return testSuite.run();

}