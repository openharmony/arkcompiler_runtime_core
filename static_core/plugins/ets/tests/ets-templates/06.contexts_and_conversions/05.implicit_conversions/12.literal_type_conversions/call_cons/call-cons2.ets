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
desc: Call context, constructor, literal type to type of literal conversion, implicit
params: to {{c.to_type}}
tags: [{{c.tags}}]
---*/

{%- for v in c['values'] %}
class A{{loop.index}} {
    f: {{c.to_type}}
    constructor(p: {{v.type}}) { this.f = p }
}
{%- endfor %}

function main() {
{%- for v in c['values'] %}
    assertTrue(new A{{loop.index}}({{v.expr}}).f == {{v.val or v.expr}})
{%- endfor %}
}
{%- endfor %}
