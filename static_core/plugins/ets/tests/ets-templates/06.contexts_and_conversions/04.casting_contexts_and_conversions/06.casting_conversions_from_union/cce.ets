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
    A casting conversion from union converts an expression of union type
    to one of the types of the union, or to a type that is derived from such one type.
params: from {{c.from_type|safe}}
---*/
{{c.decl}}

function foo(p: Object|null|undefined): Object|null|undefined {
    return p
}

{%- for v in c['types'] %}
let s{{loop.index}}: {{c.from_type}} = {{v.v}}
{%- endfor %}

function main(): int {
{%- for v in c['types'] %}
    try {
        let d{{loop.index}} = foo(s{{loop.index}}) as {{v.type}}
        return 1
    } catch (e) {
        if (!(e instanceof ClassCastError)) {
            return 1
        }
    }
{%- endfor %}
    return 0
}
{%- endfor %}
