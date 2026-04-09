/*---
Copyright (c) 2021-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    A tuple type is a reference type created as a fixed set of other types.
    The value of a tuple type is a group of values of types that comprise
    the tuple type.
---*/

const testSuite = new arktest.ArkTestsuite("Readonly_tuple_neg");
{%- for c in cases %}
{{c.decl}}
{%- set fun_name = 'testCase' + loop.index0 | string %}
function {{fun_name}}() {
    let v: [{{c.type}}] = [{{c.val}}]
    let q = v
    arktest.assertEQ(v, q)
    arktest.assertEQ(v[0], q[0])
}
testSuite.addTest("{{fun_name}}", {{fun_name}})
{% endfor %}

function main(): int {
    return testSuite.run()
}
