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
desc: Declaration context, variable, literal type to type of literal conversion, implicit
params: to {{c.to_type}}
tags: [{{c.tags}}]
---*/

{%- for t in c['values'] %}
function cvt{{loop.index}}(p: {{t.type}}) {
    let d: {{c.to_type}} = p;
    assertEQ(d, {{t.val or t.expr}})
}
{%- endfor %}

function main() {
{%- for t in c['values'] %}
    cvt{{loop.index}}({{t.expr}})
{%- endfor %}
}
{%- endfor %}
