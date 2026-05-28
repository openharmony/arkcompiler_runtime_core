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

{% for c in cases %}

/*---
desc: Dispatching via accessor methods and fields of a class member - negative cases.
tags: [compile-only, negative]
expected_out: "{{c.exp_out}}"
---*/

class A { // all static, ie no instance methods to dispatch to
    static str_static: string = "static"
    static foo_get() { return A.str_static }
    static foo_set(val: string) { A.str_static = val }
}

{{c.decl}}

function main() {
    {{c.use}}
}
{%- endfor %}
