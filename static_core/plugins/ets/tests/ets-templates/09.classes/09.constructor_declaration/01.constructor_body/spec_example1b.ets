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
desc: >-
    Call to a superconstructor if a class has an extension clause.
    The manner new expressions work together with a constructor body execution
    sequence is represented by the example below.
---*/

let res = ''

function trace(msg: string) {
    res += msg + ','
    return res
}

class C {
    C_field = trace("C1")
    constructor() {
        trace("C2")
    }
}

class B extends C {
    B_field = trace("B1")
    override C_field = trace("C3")
    constructor() {
        super ()
        trace ("B2")
    }
}

class A extends B {
    A_field = trace("A1")
    override B_field = trace("B3")
    override C_field = trace("C4")
    constructor() {
        super()
        trace ("A2")
    }
}

function main() {
    res = ''
    let a = new A ()
    arktest.assertEQ(res, 'C1,C2,B1,C3,B2,A1,B3,C4,A2,')
}
