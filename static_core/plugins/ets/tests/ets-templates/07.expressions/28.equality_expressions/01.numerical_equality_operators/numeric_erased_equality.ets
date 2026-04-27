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

{% yaml 'numeric_erased_equality.params.yaml' as yaml_file %}
/*---
desc: >-
    Numeric equality through Any-typed parameters tests runtime ets.equals/ets.strictequals.
    Values are erased to Any to force the runtime bytecode path.
---*/

function eqAnyAny(a: Any, b: Any): void {
    arktest.assertTrue(a == b)
    arktest.assertTrue(a === b)
    arktest.assertFalse(a != b)
    arktest.assertFalse(a !== b)
}

function neAnyAny(a: Any, b: Any): void {
    arktest.assertFalse(a == b)
    arktest.assertFalse(a === b)
    arktest.assertTrue(a != b)
    arktest.assertTrue(a !== b)
}

function main(): int {
    let tests = new arktest.ArkTestsuite('Numeric erased equality. Any type arguments')

{%- for c in yaml_file.eq_cases %}
    tests.addTest('eq {{loop.index}}', () => {
    {%- for a in c['values'] %}
    {%- for b in c['values'] %}
        eqAnyAny({{a}}, {{b}})
    {%- endfor %}
    {%- endfor %}
    })
{%- endfor %}

{%- for c in yaml_file.ne_cases %}
    tests.addTest('ne {{loop.index}}', () => {
    {%- for b in c['values'] %}
        neAnyAny({{c.a}}, {{b}})
    {%- endfor %}
    })
{%- endfor %}

    return tests.run()
}
