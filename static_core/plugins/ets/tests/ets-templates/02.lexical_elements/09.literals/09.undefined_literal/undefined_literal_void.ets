/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Undefined literal - void
---*/

function f(): void { return undefined }

function main(): int {
    let myTestsuite = new arktest.ArkTestsuite("UndefinedLiteralVoid");

    myTestsuite.addTest("AssigningVoidAsTypeAnnotation", () => {
        let v: void = undefined
        arktest.assertEQ(v, undefined)
    });

    myTestsuite.addTest("AssigningVariableOfTypeUndefinedToVariableOfTypeVoid", () => {
        let v: void = undefined
        let u: undefined = v
        arktest.assertEQ(u, undefined)
    });

    myTestsuite.addTest("AssigningVariableOfTypeVoidToVariableOfTypeUndefined", () => {
        let u: undefined = undefined
        let v: void = u
        arktest.assertEQ(v, undefined)
    });

    myTestsuite.addTest("UndefinedLiteralInUnionType", () => {
        let i: int | undefined = undefined
        arktest.assertEQ(i, undefined)
    });

    myTestsuite.addTest("EmptyFunctionShouldReturnUndefined", () => {
        let f = () => {}
        let result = f()
        arktest.assertEQ(result, undefined)
    });

    myTestsuite.addTest("VoidFunctionShouldReturnUndefined", () => {
        let result = f()
        arktest.assertEQ(result, undefined)
    });

    return myTestsuite.run();
}