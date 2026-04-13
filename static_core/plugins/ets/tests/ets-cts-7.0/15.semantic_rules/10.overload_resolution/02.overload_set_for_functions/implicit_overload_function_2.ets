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
assertion: If an overload set is formed from implicit overloads only, then the order 
           of the overload set corresponds to the textual order of entity declaration.
desc: Overload Resolution for Overload Declarations - implicit function overload.
---*/

function max(a: double, b: double): double {
    return 200 + (a > b ? a : b);
}

function max(a: int, b: int): int {
    return 100 + (a > b ? a : b);
}

function max(...a: double[]): double {
    let max: double = a[0];
    for (let x: number of a) {
        if (x > max) {
             max = x;
        }
    }
    return 300 + max;
}

function main() {
    let i: int = 1;
    let j: int = 2;
    let pi: number = 3.14;

    let testSuite = new arktest.ArkTestsuite('Implicit overload function 2')
    {% for c in cases %}
    testSuite.addTest('test_{{loop.index}}', () => {
        arktest.assertEQ({{c.call}}, {{c.result}})
    })
    {% endfor %}
    let res = testSuite.run()
    return res
}
