/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

/*---
desc: >-
    Numeric type values, by using a call to the standard library
    (see Standard Library) class BigInt methods or constructors.
---*/
{% for c in cases %}
{% set outer_loop_index = loop.index %}
{%- for t in c['values'] %}
const prim{{outer_loop_index}}_{{loop.index}}: {{c.from_type}} = {{t.expr|safe}}
{%- endfor %}
{%- endfor %}

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Tuple_with_bigintParam");
    {% for c in cases %}
        {% set outer_loop_index = loop.index %}
        testSuite.addTest("bigint_tuple_{{loop.index}}", () => {
            let a: [boolean, {% for t in c['values'] %}bigint{{ ", " if not loop.last }}{% endfor %}] =
            [true, {% for t in c['values'] %}new BigInt(prim{{outer_loop_index}}_{{loop.index}}){{ ", " if not loop.last }}{% endfor %}]
            {%- for t in c['values'] %}
            arktest.assertTrue( a[{{loop.index}}] == {{t.val}})
            {%- endfor %}
        }); 
    {%- endfor %}

    return testSuite.run();
}

