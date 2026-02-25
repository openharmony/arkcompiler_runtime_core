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
desc: check ternary if-else operation with condition of float operand
---*/

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Ternary_floats");

    {% for v in values %}
        testSuite.addTest("Ternary_float_{{loop.index}}", () => {
            const a: float = {{v.condition}}
            const b: float = {{v.ifTrue}}
            const c: float = {{v.ifFalse}}
            arktest.assertEQ((a ? b : c), {{v.result}})
        }); 
    {%- endfor %}

    return testSuite.run();
}
