/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Array literals can be assigned to the array of void type in any form
---*/

type A1<T> = T[]
type A2<T> = Array<T>

function testArrayTypeAliasVoid() {
    const a1: A1<void> = [undefined]
    arktest.assertEQ(a1.length, 1)
}

function testArrayClassVoid() {
    const a2: A2<void> = [undefined, undefined]
    arktest.assertEQ(a2.length, 2)
}

function testDirectVoidArray() {
    const a3: void[] = [undefined]
    arktest.assertEQ(a3.length, 1)
}

function main() {
    const suite = new arktest.ArkTestsuite('void array types')
    suite.addTest('T[] parametrized with void', testArrayTypeAliasVoid)
    suite.addTest('Array<T> parametrized with void', testArrayClassVoid)
    suite.addTest('direct void[] array', testDirectVoidArray)
    return suite.run()
}
