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

function singleArg(a: Any): int { return 1 }

function singleArg(a: Object): int { return 2 }

function multipleArgs(a: double, b: Any, c: string): int { return 1 }

function multipleArgs(a: double, b: Object, c: string): int { return 2 }

function optionalArg(a: double, b?: Any): int { return 1 }

function optionalArg(a: double, b?: Object): int { return 2 }

function main() {
    let testSuite = new arktest.ArkTestsuite('Implicit overload function 4')
    {% for c in cases %}
    testSuite.addTest('test_{{loop.index}}', () => {
        arktest.assertEQ({{c.call}}, {{c.result}})
    })
    {% endfor %}
    let res = testSuite.run()
    return res
}
