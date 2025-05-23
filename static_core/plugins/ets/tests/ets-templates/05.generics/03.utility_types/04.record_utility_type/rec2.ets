/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    Type K is restricted to number types, type string, union types
    constructed from these types, and literals of these types.
tags: [{{c.tags}}]
---*/

{{c.decl}}

{%- for v in c['values'] %}
let key{{loop.index}}: {{c.type}} = {{v.key}}, val{{loop.index}}: {{c.type}} = {{v.val}}
{%- endfor %}

function main() {
    let r: Record<{{c.type}}, {{c.type}}> = {}
{%- for v in c['values'] %}
    r[key{{loop.index}}] = val{{loop.index}}
    assertTrue(r[key{{loop.index}}] == val{{loop.index}})
{%- endfor %}
}
{% endfor %}
