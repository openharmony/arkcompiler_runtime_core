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
desc: Checking the implementation of interface readonly field (declaration field, default value of type).
---*/

{% yaml "readonly_field_interface.data.yaml" as yaml_file %}
const testSuite = new arktest.ArkTestsuite('readonly_field_interface.impl_interface_field_decl');
{% for c in yaml_file.cases + yaml_file.cases2 %}
{% set fun_name = 'testFun' + loop.index0 | string %}
{% set cl_name = 'A' + loop.index0 | string %}
interface I{{cl_name}} {
    readonly fieldI: {{c.type}}
}

class {{cl_name}} implements I{{cl_name}} {
    readonly fieldI: {{c.check_type}}
}

function {{fun_name}}() {
    let a: {{cl_name}} = new {{cl_name}}()
    arktest.assertTrue(a.fieldI instanceof {{c.check_type}}, "[{{c.type}}->{{c.check_type}}]: expected {{c.check_type}} but was " + (typeof a.fieldI))
    arktest.assertEQ(a.fieldI, {{c.default_value}})
}
testSuite.addTest("{{fun_name}}", {{fun_name}})
{% endfor %}

function main(): int {
    let res = testSuite.run();
    return res;
}
