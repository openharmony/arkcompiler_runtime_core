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
desc: Type void can be used as a type argument that instantiates a generic function or method
---*/

function foo<T>(p: T): T { return p }

type F1<T> = () => T

function testGenericFunctionWithVoid() {
    arktest.assertEQ(foo<void>(undefined), undefined)
}

function testFunctionTypeVoidReturnVoid() {
    const f1: F1<void> = (): void => {}
    arktest.assertTrue(f1 instanceof Function)
}

function testFunctionTypeVoidReturnUndefined() {
    const f3: F1<void> = (): undefined => { return undefined }
    arktest.assertTrue(f3 instanceof Function)
}

function main() {
    const suite = new arktest.ArkTestsuite('void as generic function type argument')
    suite.addTest('generic function called with void type argument', testGenericFunctionWithVoid)
    suite.addTest('F1<void> with explicit void return type', testFunctionTypeVoidReturnVoid)
    suite.addTest('F1<void> with explicit undefined return type', testFunctionTypeVoidReturnUndefined)
    return suite.run()
}
