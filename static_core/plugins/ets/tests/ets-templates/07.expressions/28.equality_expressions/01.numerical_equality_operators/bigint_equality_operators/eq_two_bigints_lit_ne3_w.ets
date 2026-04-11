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
{% yaml 'eq_two_bigints_lit.params.yaml' as yaml_file %}
/*---
desc: >-
    Value equality is used for operands of bigint type.
    Two bigints are equal if they have the same value.
expected_out: { compiler: ["Warning W65001: The equality expression is always false."] }
---*/

function main(): int {
    let tests = new arktest.ArkTestsuite('Bigint equality. Literals')

{%- for c in yaml_file.cases %}
    tests.addTest('{{loop.index}}', () => {
{%- for a in c['values'] %}
{%- for b in c['values'] %}
        arktest.assertEQ({{a}}, {{b}})
        arktest.assertFalse({{a}} !== {{b}})
{%- endfor %}
{%- endfor %}
    })
{%- endfor %}
    return tests.run()
}
