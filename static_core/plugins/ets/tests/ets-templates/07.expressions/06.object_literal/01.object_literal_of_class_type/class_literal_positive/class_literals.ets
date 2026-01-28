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
---*/

{{c.decl}}

function foo(type: {{c.type}}): {{c.type}} {
    return type
}

function main() {
   
   let multipleAssertSuite = new arktest.ArkTestsuite("type {{c.type}}");

   {% for subcheck in c.checks %}
   // variable type
   let classLiteralAsVar{{loop.index}}: {{c.type}} = {{subcheck.literal}}
   // parameter type
   let classLiteralAsParam{{loop.index}}: {{c.type}} = foo({{subcheck.literal}})

   multipleAssertSuite.addTest("{{subcheck.field}}", () => {
        arktest.assertEQ(classLiteralAsVar{{loop.index}}.{{subcheck.field}}, {{subcheck.value}});
        arktest.assertEQ(classLiteralAsParam{{loop.index}}.{{subcheck.field}}, {{subcheck.value}});
   });
   {% endfor %}
   const result: int = multipleAssertSuite.run()
   arktest.assertEQ(result, 0)
}
{% endfor %}
