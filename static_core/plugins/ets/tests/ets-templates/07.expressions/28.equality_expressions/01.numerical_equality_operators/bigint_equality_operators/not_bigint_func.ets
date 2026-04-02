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

{% yaml 'not_bigint.params.yaml' as yaml_file %}

/*---
desc: >-
    If one operand is of type bigint, and the other is of a numeric type, then the
    result is false.
---*/

function eq_1(a: bigint, b: Any) {
    arktest.assertNE(a, b)
    arktest.assertTrue(a != b)
    arktest.assertTrue(a !== b)
    arktest.assertFalse(a == b)
    arktest.assertFalse(a === b)
}
function eq_2(a: BigInt, b: Any) {
    arktest.assertNE(a, b)
    arktest.assertTrue(a != b)
    arktest.assertTrue(a !== b)
    arktest.assertFalse(a == b)
    arktest.assertFalse(a === b)
}

function main(): int {
    let tests = new arktest.ArkTestsuite('Bigint equality with not a bigint type passed through a function argument')
{%- for c in yaml_file.cases %}
    tests.addTest('{{loop.index}}', () => {
        eq_1(0n, {{c.exp}})
        eq_2(0n, {{c.exp}})
    })
{% endfor %}
    return tests.run()
}
