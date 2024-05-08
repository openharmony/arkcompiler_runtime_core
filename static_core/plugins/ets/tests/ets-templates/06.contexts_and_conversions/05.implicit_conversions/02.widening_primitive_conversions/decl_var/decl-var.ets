/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
desc: Declaration context, variable declaration, widening primitive conversion, implicit
params: from {{c.from_type}} to {{c.to_type}}
---*/
{{c.defs}}

function main(): int {
{%- for v in c['values'] %}
    let s{{loop.index}}: {{c.from_type}} = {{v.expr|safe}}
    let d{{loop.index}}: {{c.to_type}} = s{{loop.index}}  // this var declaration is tested
    if (d{{loop.index}} != {{v.val}}) {
        return 1
    }
{%- endfor %}
    return 0
}
{%- endfor %}
