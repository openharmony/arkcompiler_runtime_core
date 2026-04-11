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
{% yaml 'eq_groups.params.yaml' as yaml_file %}

/*---
desc: |-
    Relational operators can be applied to bigint values (see Type bigint) if:
    - both operands are of bigint type; or
    - one operand is of bigint type and other is of a numeric type.
---*/

function gt_1(a: bigint, b: bigint, diff: bigint) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_1(a: bigint, b: byte, diff: bigint) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_1(a: bigint, b: short, diff: bigint) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_1(a: bigint, b: int, diff: bigint) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_1(a: bigint, b: long, diff: bigint) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_1(a: bigint, b: float, diff: bigint) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_1(a: bigint, b: double, diff: bigint) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}

function gt_2(a: BigInt, b: BigInt, diff: BigInt) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_2(a: BigInt, b: Byte, diff: BigInt) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_2(a: BigInt, b: Short, diff: BigInt) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_2(a: BigInt, b: Int, diff: BigInt) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_2(a: BigInt, b: Long, diff: BigInt) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_2(a: BigInt, b: Float, diff: BigInt) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}
function gt_2(a: BigInt, b: Double, diff: BigInt) {
    arktest.assertTrue(a + diff > b)
    arktest.assertFalse(a > b)
    arktest.assertFalse(a - diff > b)
}

function main(): int {
    let tests = new arktest.ArkTestsuite('Bigint operator ">". Function arguments')
{%- for c in yaml_file.cases %}
    tests.addTest('{{loop.index}}', () => {
{%- for v in c['values'] %}
        gt_1({{c.bigint}}, {{v}}, {{c.diff or '1n'}})
        gt_2({{c.bigint}}, {{v}}, {{c.diff or '1n'}})
{%- endfor %}
    })
{% endfor %}
    return tests.run()
}
