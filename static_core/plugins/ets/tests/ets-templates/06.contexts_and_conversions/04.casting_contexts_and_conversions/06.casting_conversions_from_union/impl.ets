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
{%- for c in cases %}

/*---
desc: >-
    Another form of conversion from union is implicit conversion from union type
    to the target type.
params: from {{c.from_type|safe}}
tags: [{{c.tags}}]
---*/
{{c.decl}}
let src: {{c.from_type}}

function main(): int {
{%- for v in c['types'] %}
    src = {{v.v}}
    let target{{loop.index}}: {{v.type}} = src
    if (target{{loop.index}} != {{v.v}}) {
        return 1
    }
{%- endfor %}
    return 0
}
{%- endfor %}
