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
    Since a spread expression copies values, the attribute readonly of the source array does not affect an array created by the spread expression.
    If a spread expression is to create a readonly target array, then the attribute readonly must be used for the target array or for the rest parameter.
---*/

function main(): int {
    let myTestsuite = new arktest.ArkTestsuite("SpreadReadonlyArraySource");

    myTestsuite.addTest("ReadonlyIntArrayToMutable", () => {
        let a: readonly int[] = [1, 2, 3]
        let rw: int[] = [...a]
        rw[1] = 4
        arktest.assertEQ(rw[1], 4)
        arktest.assertEQ(a[1], 2)
    });

    myTestsuite.addTest("ReadonlyNumberArrayToMutableWithElements", () => {
        let a: readonly number[] = [1.0, 2.0, 3.0]
        let rw = [0.0, ...a, 4.0]
        rw[0] = 5.0
        rw[3] = 6.0
        arktest.assertEQ(rw[0], 5.0)
        arktest.assertEQ(rw[3], 6.0)
    });

    myTestsuite.addTest("ReadonlyByteArraySpread", () => {
        let a: readonly byte[] = [1 as byte, 2 as byte, 3 as byte]
        let rw: byte[] = [...a]
        rw[0] = 11 as byte
        arktest.assertEQ(rw[0], 11)
        arktest.assertEQ(a[0], 1)
    });

    myTestsuite.addTest("ReadonlyCharArraySpread", () => {
        let a: readonly char[] = [c'a', c'b', c'c']
        let rw: char[] = [...a]
        rw[0] = c'Z'
        arktest.assertEQ(rw[0], c'Z')
        arktest.assertEQ(a[0], c'a')
    });

    myTestsuite.addTest("ReadonlyToReadonly", () => {
        let a: readonly string[] = ["a", "b", "c"]
        let ro: readonly string[] = [...a]
        arktest.assertEQ(ro[0], "a")
        arktest.assertEQ(ro.length, 3)
    });

    myTestsuite.addTest("MixedReadonlyAndMutableSources", () => {
        let readonly_arr: readonly number[] = [1.0, 2.0]
        let mutable_arr: number[] = [3.0, 4.0]
        let result: readonly number[] = [...readonly_arr, ...mutable_arr]
        arktest.assertEQ(result.length, 4)
        arktest.assertEQ(result[2], 3.0)
    });

    return myTestsuite.run();
}
