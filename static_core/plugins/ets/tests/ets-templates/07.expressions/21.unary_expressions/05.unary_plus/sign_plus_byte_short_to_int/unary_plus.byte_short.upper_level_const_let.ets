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
desc: Unary plus (const and let in upper level). Checking the conversion of byte and short types to int type.
---*/
const testSuite = new arktest.ArkTestsuite('unary_plus.byte_short.upper_level_const_let');
{% set modifs = ['const', 'let'] %}
{%- yaml "unary_plus.byte_short.test_data.yaml" as td%}
{%- for c in td.testData %}
{%- set index = loop.index0 | string %}
// {{c.desc}}
{%- for m in modifs %}
{# Creating variable names to ensure that they are unique at the same code level. #}
{%- set val = m + 'Val' + index %}
{%- set res = m + 'Res' + index %}
{{m}} {{val}}: {{c.type}} = {{c.value}}
{{m}} {{res}} = {{c.format | format(val)}}

{#- Creating function name to ensure that they are unique. #}
{%- set func = res + 'Test' %}
function {{func}}(): void {
    arktest.assertTrue({{res}} instanceof {{c.resultType}}, "The type of the actual result (" + (typeof {{res}}) + ") is of a different type than expected ({{c.resultType}})")
    arktest.assertEQ({{res}}, {{c.result}})
}
testSuite.addTest('{{func}}', {{func}})
{% endfor %}
// ------
{% endfor %}
function main(): int {
    let res = testSuite.run();
    return res;
}
