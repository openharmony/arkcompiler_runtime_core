/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
    Mandatory call to a superconstructor if a class has an extension clause
    on all execution paths of the constructor body.
---*/

abstract class A {
    fldA: string

    constructor(a: string) {
        this.fldA = a
    }
}

final class B extends A {
    fldB: string = this.fldA
    fldC: string = this.fldA + '=' + this.fldB

    // primary constructor
    constructor fromString(a: string) {
        super(a)
    }

    // secondary constructor
    constructor fromInt(a: int) {
        // 1. Optional arbitrary code that does not use this or super.
        if (a == 0) {
            throw new Error()
        }
        // 2. Call to another same-class constructor that uses the keyword this
        switch (a) {
            case 1:
                this.fromString('' + a)
                break
            case 2:
                this.fromString('' + a + a)
                break
            default:
                throw new Error()
        }
    }
}

function main() {
    let b: B
    try {
        b = new B.fromInt(0)
        arktest.assertTrue(false, 'Error expected')
    } catch (e) {
        // OK, expected
    }

    b = new B.fromInt(1)
    arktest.assertEQ(b.fldA, '1')
    arktest.assertEQ(b.fldB, '1')
    arktest.assertEQ(b.fldC, '1=1')

    b = new B.fromInt(2)
    arktest.assertEQ(b.fldA, '22')
    arktest.assertEQ(b.fldB, '22')
    arktest.assertEQ(b.fldC, '22=22')
}
