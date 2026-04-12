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

/*---
desc: Checking the implementation of interface readonly field (init field and namespace).
---*/

{% yaml "readonly_field_interface.data.yaml" as yaml_file %}
const testSuite = new arktest.ArkTestsuite('readonly_field_interface.impl_interface_namespace_field_init');
{% for c in yaml_file.cases + yaml_file.cases2 + yaml_file.cases3 + yaml_file.cases4 %}
{% set namespace = 'testNS' + loop.index0 | string %}
namespace {{namespace}} {
    interface I {
        readonly fieldI: {{c.type}}
    }
  
    class A implements I {
        readonly fieldI: {{c.check_type}} = {{c.base_value}}
    }

    export function runTest() {
        let a: A = new A()
        arktest.assertTrue(a.fieldI instanceof {{c.check_type}}, "{{namespace}} [{{c.type}}->{{c.check_type}}]: expected {{c.check_type}} but was " + (typeof a.fieldI))
        arktest.assertEQ(a.fieldI, {{c.expected_value}})
    }
}
testSuite.addTest("{{namespace}}", {{namespace}}.runTest)
{% endfor %}

function main(): int {
    let res = testSuite.run();
    return res;
}
