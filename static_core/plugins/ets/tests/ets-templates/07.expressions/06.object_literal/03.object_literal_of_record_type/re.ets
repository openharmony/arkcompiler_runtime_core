/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

{% for c in cases %}
/*---
desc: >-
    The generic Record<Key, Value> type (see Record Utility Type) is used to map the properties of
    a type (Key type) to another type (Value type). A special form of an object literal is used to
    initialize the value of such type.
---*/

{% if c.decl %}
{{ c.decl }}
{% endif %}

function main() {
{%- if c.use %}
    {{ c.use|indent }}
{% endif %}

    let multipleAssertSuite = new arktest.ArkTestsuite("multiple asserts check");

{%- if c.checks %}
    {% for subcheck in c.checks %}
    multipleAssertSuite.addTest('{{ subcheck.call }}', () => {
        arktest.assertEQ({{ subcheck.call }}, {{ subcheck.value }});
    });
    {% endfor %}
{%- endif %}

    const result: int = multipleAssertSuite.run()

{%- if c.second_checks %}

    {{ c.after|indent }}

    let multipleAssertSuite_second = new arktest.ArkTestsuite("multiple asserts check second");

    {% for subcheck in c.second_checks %}
    multipleAssertSuite_second.addTest('{{ subcheck.call }}', () => {
        arktest.assertEQ({{ subcheck.call }}, {{ subcheck.value }});
    });
    {% endfor %}

    const result_second: int = multipleAssertSuite_second.run()
    arktest.assertEQ(result + result_second, 0)
{%- else %}
    arktest.assertEQ(result, 0)
{%- endif %}
}
{% endfor %}