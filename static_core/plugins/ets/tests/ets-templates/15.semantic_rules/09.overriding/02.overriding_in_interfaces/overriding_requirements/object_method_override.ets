/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
    When a method explicitly declared by the interface has the same name as
    the Object’s public method with the override-compatible signature (see
    Override-Compatible Signatures) and is the interface default method (see Default
    InterfaceMethodDeclarations), then such a method will never be invoked.
---*/

interface A {
    toString(): String {
        return "This method will never be called"
    }
}

function main() {
    let a: A = {}
    arktest.assertEQ(a.toString(), '[object Object]')
}
