/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
{% yaml 'rel_false.params.yaml' as yaml_file %}
/*---
desc: Case of if not both operands are bigint
---*/

{%- for c in yaml_file.cases %}
let b_{{loop.index}} = {{c.b}}
{%- endfor %}

function main(): int {
    let tests = new arktest.ArkTestsuite('BigInt relational operations with numeric')
{%- for c in yaml_file.cases %}
    tests.addTest('{{loop.index}}', () => {
        arktest.assertFalse(b_{{loop.index}} {{c.op}} {{c.exp}}, `Failed: ${ b_{{loop.index}} } {{c.op}} {{c.exp}}`)
    })
{% endfor %}
    return tests.run()
}
