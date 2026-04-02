/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: check bitwise complement for double
---*/

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Bitwise_c_doubles");

    {% for v in values %}
        testSuite.addTest("bitwise_c_double_{{loop.index}}", () => {
            const a: double = {{v.value}}
            arktest.assertEQ(~a, {{v.result}})
        });
    {%- endfor %}

    return testSuite.run(); 
}
