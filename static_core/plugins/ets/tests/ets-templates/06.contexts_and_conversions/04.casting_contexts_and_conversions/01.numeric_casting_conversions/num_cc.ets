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
{%- for v in c['values'] %}

/*---
desc: A numeric casting conversion occurs if the target type and the expression type are both numeric or char.
params: from {{c.from_type}} to {{c.to_type}}
---*/
let s{{loop.index}}: {{c.from_type}} = {{v.sval}}
let d{{loop.index}}: {{c.to_type}} = {{v.dval}}

function main(): int {
    let d: {{c.to_type}}
    d = s{{loop.index}} as {{c.to_type}}
    if (d != d{{loop.index}}) {
        return 1
    }
    return 0
}
{%- endfor %}
{%- endfor %}
