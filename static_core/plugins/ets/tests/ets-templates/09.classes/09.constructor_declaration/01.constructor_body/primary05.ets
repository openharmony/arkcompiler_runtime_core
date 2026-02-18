/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    Mandatory call to a superconstructor if a class has an extension clause.
---*/

let res = ''

class A {
    x: Object
    constructor() {
        this.x = 'a'
        this.assert()
    }

    assert() {
        arktest.assertTrue(false, 'this method should not be called')
    }
}

class B extends A {
    y: Object
    constructor() {
        super()
        this.y = 'b'
    }

    assert() {
        arktest.expectThrow(() => { res = this.y.toString() })
    }
}

function main() {
    let b = new B()
    arktest.assertEQ(b.x, 'a')
    arktest.assertEQ(b.y, 'b')
    arktest.assertEQ(res, '')
}
