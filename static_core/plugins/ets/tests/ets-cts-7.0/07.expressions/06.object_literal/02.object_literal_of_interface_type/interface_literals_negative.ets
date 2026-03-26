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
desc: >-
    Object literal is an expression that can be used to create
    a class instance with methods and fields with initial values
tags: [negative, compile-only]
expected_out: |-
    {{ c.expected|replace('\n', '')}}
---*/

interface I {
    {{c.decl|indent}}
}

function foo(type: I): I {
    return type
}

function main() {

   // variable type
   let interfaceLiteralAsVar: I = {{c.literal|replace('\n', '')}}
   // parameter type
   let interfaceLiteralAsParam: I = foo({{c.literal|replace('\n', '')}})

   {% if c.call %}
   interfaceLiteralAsVar.{{c.call}}
   interfaceLiteralAsParam.{{c.call}}
   {% endif %}

}
{% endfor %}
