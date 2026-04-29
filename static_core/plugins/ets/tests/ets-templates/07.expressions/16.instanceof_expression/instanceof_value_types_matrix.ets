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

{% for c in cases %}
/*---
desc: >-
    The result of an instanceof expression is true if the actual type of evaluated expr is a subtype of T.
    Otherwise, the result is false.
---*/

{{c.decl}}

function passThrough(value: Any): Any {
    return value
}

function main(): int {
    let suite = new arktest.ArkTestsuite("instanceof value")
{%- for target in targets %}

    suite.addTest("{{c.name}} instanceof {{target.type}}", () => {
{%- if target.type in c.matches %}
        arktest.assertTrue(t instanceof {{target.type}},
            "failed for direct {{c.name}} instanceof {{target.type}}")
{%- else %}
        arktest.assertFalse(t instanceof {{target.type}},
            "failed for direct {{c.name}} instanceof {{target.type}}")
{%- endif %}
{%- if target.type in c.matches %}
        arktest.assertTrue(passThrough(t) instanceof {{target.type}},
            "failed for Any {{c.name}} instanceof {{target.type}}")
{%- else %}
        arktest.assertFalse(passThrough(t) instanceof {{target.type}},
            "failed for Any {{c.name}} instanceof {{target.type}}")
{%- endif %}
    })
{%- endfor %}

    return suite.run()
}
{% endfor %}
