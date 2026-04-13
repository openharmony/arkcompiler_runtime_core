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
{% yaml 'eq_two_bigints.params.yaml' as yaml_file %}
/*---
desc: >-
    Value equality is used for operands of bigint type.
    Two bigints are equal if they have the same value.
---*/

function eq_1(a: bigint, b: bigint): void {
    arktest.assertEQ(a, b)
    arktest.assertTrue(a == b)
    arktest.assertTrue(a === b)
    arktest.assertFalse(a != b)
    arktest.assertFalse(a !== b)
    arktest.assertTrue(!(a < b) && !(a > b) && !(b < a) && !(b > a))
}

function eq_2(a: bigint, b: BigInt): void {
    arktest.assertEQ(a, b)
    arktest.assertTrue(a == b)
    arktest.assertTrue(a === b)
    arktest.assertFalse(a != b)
    arktest.assertFalse(a !== b)
    arktest.assertTrue(!(a < b) && !(a > b) && !(b < a) && !(b > a))
}

function eq_3(a: BigInt, b: BigInt): void {
    arktest.assertEQ(a, b)
    arktest.assertTrue(a == b)
    arktest.assertTrue(a === b)
    arktest.assertFalse(a != b)
    arktest.assertFalse(a !== b)
    arktest.assertTrue(!(a < b) && !(a > b) && !(b < a) && !(b > a))
}

function main(): int {
    let tests = new arktest.ArkTestsuite('Bigint equality. BigInt type function arguments')

{%- for c in yaml_file.cases %}
    tests.addTest('{{loop.index}}', () => {
{%- for a in c['values'] %}
{%- for b in c['values'] %}
        eq_1({{a}}, {{b}})
        eq_2({{a}}, {{b}})
        eq_3({{a}}, {{b}})
{%- endfor %}
{% endfor %}
    })
{%- endfor %}
    return tests.run()
}
