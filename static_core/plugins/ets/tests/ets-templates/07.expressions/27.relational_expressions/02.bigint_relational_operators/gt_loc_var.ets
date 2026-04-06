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

function main(): int {
    let tests = new arktest.ArkTestsuite('Bigint operator ">". Local variables')
{%- for c in yaml_file.cases %}
    tests.addTest('{{loop.index}}', () => {
        let a = {{c.bigint}}
{%- for v in c['values'] %}
        let b_{{loop.index}} = {{v}}
        arktest.assertTrue(a + {{c.diff or '1n'}} > b_{{loop.index}})
        arktest.assertFalse(a > b_{{loop.index}})
        arktest.assertFalse(a - {{c.diff or '1n'}} > b_{{loop.index}})
{%- endfor %}
    })
{% endfor %}
    return tests.run()
}
