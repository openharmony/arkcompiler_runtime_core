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
desc: Type void can be used as a type argument that instantiates a generic class
---*/

class A<T> {
    f: T
    m(): T { return this.f }
    constructor(f: T) { this.f = f }
}

function testVoidClassInstantiation() {
    let a1: A<void> = new A<void>(undefined)
    arktest.assertEQ(a1.f, undefined)
}

function testUndefinedClassField() {
    let a2: A<undefined> = new A<undefined>(undefined)
    arktest.assertEQ(a2.f, undefined)
}

function testUndefinedClassMethod() {
    let a2: A<undefined> = new A<void>(undefined)
    arktest.assertEQ(a2.m(), undefined)
}

function main() {
    const suite = new arktest.ArkTestsuite('void as generic class type argument')
    suite.addTest('A<void> can be instantiated', testVoidClassInstantiation)
    suite.addTest('A<undefined> field is undefined', testUndefinedClassField)
    suite.addTest('A<void> method returns undefined', testUndefinedClassMethod)
    return suite.run()
}
