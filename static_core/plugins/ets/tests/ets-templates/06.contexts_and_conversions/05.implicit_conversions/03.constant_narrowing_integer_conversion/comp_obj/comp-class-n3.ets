/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Composite context, class field, constant narrowing integer conversion, implicit
tags: [compile-only, negative]
params: from {{c.from_type}} to {{c.to_type}}
---*/
{%- for v in c['values'] %}
const c{{loop.index}}: {{c.from_type}} = {{v.expr|safe}}
{%- endfor %}

class A { fld: {{c.to_type}} }

function main(): int {
{%- for v in c['values'] %}
    let d{{loop.index}}: A = { fld: {{v.expr|safe}} }
    if (d{{loop.index}}.fld != {{v.val}}) {
        return 1
    }
    d{{loop.index}} = { fld: c{{loop.index}} }
    if (d{{loop.index}}.fld != {{v.val}}) {
        return 1
    }
{%- endfor %}
    return 0
}
{%- endfor %}
