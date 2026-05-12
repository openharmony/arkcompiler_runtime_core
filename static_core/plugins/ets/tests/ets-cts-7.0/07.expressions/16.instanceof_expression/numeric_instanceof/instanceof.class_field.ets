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
desc: instanceof expression. Class field.  
---*/

const testSuite = new arktest.ArkTestsuite('instanceof.class_field');
{%- yaml "instanceof.bigint.data.yaml" as dt1 %}
{%- yaml "instanceof.double.data.yaml" as dt2 %}
{%- yaml "instanceof.float.data.yaml" as dt3 %}
{%- yaml "instanceof.long.data.yaml" as dt4 %}
{%- yaml "instanceof.int.data.yaml" as dt5 %}
{%- yaml "instanceof.short.data.yaml" as dt6 %}
{%- yaml "instanceof.byte.data.yaml" as dt7 %}

{% for c in dt1.cases + dt2.cases + dt3.cases + dt4.cases + dt5.cases + dt6.cases + dt7.cases %}
{#- Creating function name to ensure that they are unique. #}
{%- set func = 'instanceofTest' +  loop.index0 | string %}
{%- set className = 'C' + loop.index0 | string %}
// ------
class {{className}} {
    v : {{c.type}} = {{c.value}}
}

function {{func}}(){
    arktest.assertEQ(new {{className}}().v instanceof {{c.instanceOfType}}, {{c.result}}, 
        "'<{{c.type}}> instanceof {{c.instanceOfType}}' expression has an unexpected execution result.")
}
testSuite.addTest('{{func}}', {{func}})
{% endfor %}

// run tests
function main(): int {
    let res = testSuite.run();
    return res;
}
