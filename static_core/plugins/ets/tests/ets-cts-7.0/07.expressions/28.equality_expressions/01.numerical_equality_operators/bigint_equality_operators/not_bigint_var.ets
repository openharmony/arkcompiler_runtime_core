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

let b: Any = 0n
{%- for c in yaml_file.cases %}
let a_{{loop.index}} = {{c.exp}}
{%- endfor %}

function main(): int {
    let tests = new arktest.ArkTestsuite('Bigint equality  with not a bigint type defined as top-level variable')
{%- for c in yaml_file.cases %}
    tests.addTest('{{loop.index}}', () => {
        arktest.assertNE(a_{{loop.index}}, b)
        arktest.assertFalse(a_{{loop.index}} == b)
        arktest.assertFalse(a_{{loop.index}} === b)
        arktest.assertTrue(a_{{loop.index}} != b)
        arktest.assertTrue(a_{{loop.index}} !== b)
    })
{% endfor %}
    return tests.run()
}
