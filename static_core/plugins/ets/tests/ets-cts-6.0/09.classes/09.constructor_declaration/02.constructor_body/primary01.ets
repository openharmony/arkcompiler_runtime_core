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

class A {
    fldA: string

    constructor(a: string) {
        this.fldA = a
    }
}

class B extends A {
    constructor(a: string) {
        if (a == '') {
            throw new Error('empty')
        }
        switch (a) {
            case '1':
                super(a)
                break
            case '2':
                super(a + a)
                break
            default:
                throw new Error('wrong arg = ' + a)
        }
    }
}

function main() {
    let b: B
    arktest.expectError(() => { b = new B('') }, 'empty')

    b = new B('1')
    arktest.assertEQ(b.fldA, '1')

    b = new B('2')
    arktest.assertEQ(b.fldA, '22')

    arktest.expectError(() => { b = new B('3') }, 'wrong arg = 3')
}
