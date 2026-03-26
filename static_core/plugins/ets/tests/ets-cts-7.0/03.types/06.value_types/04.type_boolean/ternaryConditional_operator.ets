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
desc: The conditional operator (? :) with boolean type.
---*/

function main(): int {

    let testSuite = new arktest.ArkTestsuite("Conditional_operator_with_boolean");
  
    testSuite.addTest("TrueConditionalCheck", () => {
        let a: boolean = true;
        let x = a ? 3 : 5;
        arktest.assertEQ( x, 3 )
    });

    testSuite.addTest("FalseConditionalCheck", () => {
        let a: boolean = false;
        let x = a ? 3 : 5;
        arktest.assertEQ( x, 5 )
    });

    testSuite.addTest("1_0_ConditionalCheck", () => {
        let x = 1 ? 3 : 5;
        arktest.assertEQ(x, 3)
        x = 0 ? 3 : 5;
        arktest.assertEQ(x, 5)
    });

    return testSuite.run(); 
}
