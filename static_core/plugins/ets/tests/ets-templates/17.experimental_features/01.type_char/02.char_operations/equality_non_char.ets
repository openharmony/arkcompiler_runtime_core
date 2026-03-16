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

{% for modifier in ["let", "const"] %}
{% for u_value in ["u0001", "u0002"] %}

/*---
desc: >-
    The equality operator checking a value of char type against a value of
    a non-char type returns false. If the result is known at compile time
    to always evaluate as false, a compile-time warning is issued
test_suffix: "{{modifier}}_{{u_value}}"
---*/

function check<T>(a0: char, x: T, assert_message: string): void {
    arktest.assertFalse(a0 == x, assert_message)
}

function checkNE<T>(a0: char, x: T, assert_message: string): void {
    arktest.assertNE(a0, x, assert_message)
}

function main() {
    let testSuite = new arktest.ArkTestsuite("equality_non_char_{{modifier}}");

    {% for v in vals %}
        testSuite.addTest("equality_non_char_{{modifier}}_{{u_value}}_{{v.type}}", () => {
            {{modifier}} a0: char = c'\{{u_value}}'
            {{modifier}} x: {{v.type}} = {{v.init}}
            let assert_message: string = "expect not equal char and {{v.type}} type values for \\{{u_value}}"
            // test via function call, both equality operator and assertNE
            check<{{v.type}}>(a0, x, assert_message)
            checkNE<{{v.type}}>(a0, x, assert_message + " NE")
            // test inline, both equality operator and assertNE
            arktest.assertFalse(a0 == x, assert_message + " inline")
            arktest.assertNE(a0, x, assert_message + " inline NE")
        });
    {%- endfor %}

    return testSuite.run();
}
{%- endfor %}
{%- endfor %}
