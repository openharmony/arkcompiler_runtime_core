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
{% yaml 'ne_two_bigints.params.yaml' as yaml_file %}
/*---
desc: >-
    Value equality is used for operands of bigint type.
    Two bigints are equal if they have the same value.
---*/

function neAnyAny(a: Any, b: Any): void {
    arktest.assertNE(a, b)
    arktest.assertFalse(a == b)
    arktest.assertFalse(a === b)
    arktest.assertTrue(a != b)
    arktest.assertTrue(a !== b)
}
function neAnyBigInt(a: Any, b: BigInt): void {
    arktest.assertNE(a, b)
    arktest.assertFalse(a == b)
    arktest.assertFalse(a === b)
    arktest.assertTrue(a != b)
    arktest.assertTrue(a !== b)
}
function neBigIntAny(a: BigInt, b: Any): void {
    arktest.assertNE(a, b)
    arktest.assertFalse(a == b)
    arktest.assertFalse(a === b)
    arktest.assertTrue(a != b)
    arktest.assertTrue(a !== b)
}

function main(): int {
    let tests = new arktest.ArkTestsuite('Bigint equality. Any type arguments')

{%- for c in yaml_file.cases %}
    tests.addTest('{{loop.index}}', () => {
{%- for b in c['values'] %}
        neAnyAny({{c.a}}, {{b}})
        neAnyBigInt({{c.a}}, {{b}})
        neBigIntAny({{c.a}}, {{b}})
{%- endfor %}
    })
{%- endfor %}
    return tests.run()
}
