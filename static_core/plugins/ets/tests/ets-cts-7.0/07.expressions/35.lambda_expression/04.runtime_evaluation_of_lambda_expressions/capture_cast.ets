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

{% for cc in cases %}

/*---
desc: Every time a lambda expression is evaluated, the outer variables referred to by
      the lambda expression are captured.
---*/


const iv: {{cc.typeIn}} = 22;

function sender(): {{cc.typeIn}} {
    return 42;
}

let outerReceiver: {{cc.typeOut}}

/* To Global */

function fromFunctionToOut() {
    let x = () => { outerReceiver = sender() }
    x();
    arktest.assertEQ(42, outerReceiver)
}

function fromConstantToOut() {
    let x = () => { outerReceiver = iv }
    x();
    arktest.assertEQ(22, outerReceiver)
}

function fromlambdaLocalToOut() {
    let x = () => {
        let local: {{cc.typeIn}} = 11
        outerReceiver = local
    }
    x();
    arktest.assertEQ(11, outerReceiver)
}

/* To Local */

function fromFunctionToLocal() {
    let localReceiver: {{cc.typeOut}}
    let x = () => { localReceiver = sender() }
    x();
    arktest.assertEQ(42, localReceiver)
}

function fromConstantToLocal() {
    let localReceiver: {{cc.typeOut}}
    let x = () => { localReceiver = iv }
    x();
    arktest.assertEQ(22, localReceiver)
}

function fromlambdaLocalToLocal() {
    let localReceiver: {{cc.typeOut}}
    let x = () => {
        let local: {{cc.typeIn}} = 11
        localReceiver = local
    }
    x();
    arktest.assertEQ(11, localReceiver)
}



function main(): int {
    const suite = new arktest.ArkTestsuite("Capture and Cast from {{cc.typeIn}} to {{cc.typeOut}}")
        suite.addTest("fromFunctionToOut", () => fromFunctionToOut())
        suite.addTest("fromConstantToOut()", () => fromConstantToOut())
        suite.addTest("fromlambdaLocalToOut()", () => fromlambdaLocalToOut())

        suite.addTest("fromFunctionToLocal()", () => fromFunctionToLocal())
        suite.addTest("fromConstantToLocal()", () => fromConstantToLocal())
        suite.addTest("fromlambdaLocalToLocal()", () => fromlambdaLocalToLocal())
    return suite.run()
}

{% endfor %}
