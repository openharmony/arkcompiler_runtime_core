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

{% yaml "class_literals_negative.params.yaml" as testData %}
{% for c in testData.cases %}
/*---
desc: >-
    Object literal is an expression that can be used to create
    a class instance with methods and fields with initial values
tags: [negative, compile-only]
expected_out: |-
    {{ c.expected|replace('\n', '')}}
---*/

abstract {{c.decl}}

{% if c.type %}
function foo(type: {{c.type}}): {{c.type}} {
    return type
}
{% endif %}

function main() {
   // variable type
   let classLiteralAsVar{{':' if c.type}} {{c.type}} = {{c.literal}}

   {% if c.type %}
   // parameter type
   let classLiteralAsParam{{':' if c.type}} {{c.type}} = foo({{c.literal}})
   {% endif %}

   {% if c.call %}
   classLiteralAsVar.{{c.call}}
   classLiteralAsParam.{{c.call}}
   {% endif %}
}
{% endfor %}