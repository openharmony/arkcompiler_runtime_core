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

/*---
desc: >-
    Relational operators can be applied to char values (see Character Relational Operators).
test_suffix: "{{modifier}}"
---*/

function check_less(v1: char, v2: char, assert_message: string): void {
    arktest.assertTrue(v1 < v2, assert_message + " (via call)")
}

function check_leq(v1: char, v2: char, assert_message: string): void {
    arktest.assertTrue(v1 <= v2, assert_message + " (via call)")
}

function check_greater(v1: char, v2: char, assert_message: string): void {
    arktest.assertTrue(v1 > v2, assert_message + " (via call)")
}

function check_geq(v1: char, v2: char, assert_message: string): void {
    arktest.assertTrue(v1 >= v2, assert_message + " (via call)")
}

function main() {
    {{modifier}} v1: char = c'\u0001'
    {{modifier}} v2: char = c'\u0002'
    {{modifier}} v1_1: char = c'\u0001'

    // test operator inline
    arktest.assertTrue(v1 < v2, "expect less comparison to apply to char values")
    arktest.assertTrue(v1 <= v2, "expect leq comparison to apply to char values")
    arktest.assertTrue(v2 > v1, "expect greater comparison to apply to char values")
    arktest.assertTrue(v2 >= v1, "expect geq comparison to apply to char values")
    arktest.assertTrue(v1 <= v1_1, "expect leq comparison to apply to char values 1")
    arktest.assertTrue(v1 >= v1_1, "expect geq comparison to apply to char values 1")
    // test operator via function call
    check_less(v1, v2, "expect less comparison to apply to char values")
    check_leq(v1, v2, "expect leq comparison to apply to char values")
    check_greater(v2, v1, "expect greater comparison to apply to char values")
    check_geq(v2, v1, "expect geq comparison to apply to char values")
    check_leq(v1, v1_1, "expect leq comparison to apply to char values 1")
    check_geq(v1, v1_1, "expect geq comparison to apply to char values 1")
}
{%- endfor %}
