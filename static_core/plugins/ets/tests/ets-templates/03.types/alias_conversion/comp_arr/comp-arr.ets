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
desc: Composite context, array element, boxing conversion, implicit
params: from {{c.from_type}} to {{c.to_type}}
---*/
{%- for t in c['values'] %}
let s{{loop.index}}: {{c.from_type}} = {{t.expr|safe}}
{%- endfor %}

function main(): int {
{%- for t in c['values'] %}
    let d{{loop.index}}: {{c.to_type}}[] = [s{{loop.index}}, s{{loop.index}}]
    d{{loop.index}} = []
    d{{loop.index}} = [s{{loop.index}}]
    if (d{{loop.index}}[0].unboxed() != {{t.val}}) {
        return 1
    }
{%- endfor %}
    return 0
}
{%- endfor %}
