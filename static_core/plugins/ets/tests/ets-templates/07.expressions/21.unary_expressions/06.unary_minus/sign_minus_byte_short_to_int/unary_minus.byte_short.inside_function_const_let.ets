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

/*---
desc: Unary minus (const and let in inside the function). Checking the conversion of byte and short types to int type.
---*/

{% yaml "unary_minus.byte_short.test_data.yaml" as td%}
{%- set modifs = ['const', 'let'] %}
{%- for c in td.testData + td.testData2 %}
{%- set index = loop.index0 | string %}
// {{c.desc}}
{%- for m in modifs %}
function {{m}}{{index}}Test(): void {
    {{m}} val: {{c.type}} = {{c.value}}
    {{m}} res = {{c.format | format('val')}}
    arktest.assertTrue(res instanceof {{c.resultType}}, "The type of the actual result (" + (typeof res) + ") is of a different type than expected ({{c.resultType}})")
    arktest.assertEQ(res, {{c.result}})
}
{%- endfor %}
// --------
{% endfor %}
function main(): int {
    let testSuite = new arktest.ArkTestsuite('unary_minus.byte_short.inside_function_const_let');
    {% for m in modifs %}{% for _ in td.testData%}
    {%- set func = m + loop.index0 | string + 'Test' -%}
    testSuite.addTest('{{func}}', {{func}})
    {% endfor %}{% endfor -%}
    let res = testSuite.run();
    return res;
}

