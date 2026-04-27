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

{%- for c in cases %}

/*---
desc: |-
    Compile-time vs runtime consistency for typeof.
    {{c.desc}} - typeof via erased type should match direct typeof.
---*/

function erase<T>(v: T): Any {
    return v
}

{{c.decl}}

function main() {
{%- for v in c['vals'] %}
    // Direct typeof (compile-time known type)
    let direct{{loop.index}}: {{v.type}} = {{v.expr}}
    let ct{{loop.index}} = typeof direct{{loop.index}}
    // typeof via erased type (runtime dispatch)
    let rt{{loop.index}} = typeof erase(direct{{loop.index}})
    // Both must return the same result
    arktest.assertEQ(ct{{loop.index}}, "{{c.res}}")
    arktest.assertEQ(rt{{loop.index}}, "{{c.res}}")
{%- endfor %}
}

{%- endfor %}
