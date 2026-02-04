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

function fooReadonlyRestParam(...p_ro: readonly int[]) {
    arktest.assertEQ(p_ro[0], 1)
    arktest.assertEQ(p_ro[1], 2)
}

function main(): int {
    let myTestsuite = new arktest.ArkTestsuite("SpreadToReadonlyTarget");

    myTestsuite.addTest("MutableIntArrayToReadonly", () => {
        let b: int[] = [1, 2, 3]
        let ro: readonly int[] = [...b]
        arktest.assertEQ(ro[0], 1)
        arktest.assertEQ(ro[1], 2)
        arktest.assertEQ(ro[2], 3)
    });

    myTestsuite.addTest("MutableNumberArrayToReadonlyWithElements", () => {
        let b: number[] = [1.0, 2.0]
        let ro: readonly number[] = [0.0, ...b, 3.0]
        arktest.assertEQ(ro.length, 4)
        arktest.assertEQ(ro[2], 2.0)
    });

    myTestsuite.addTest("MutableArrayToReadonlyRestParameter", () => {
        let b: int[] = [1, 2, 3]
        fooReadonlyRestParam(...b)
        arktest.assertEQ(b[0], 1)
    });

    myTestsuite.addTest("MutableFixedArrayToReadonly", () => {
        let b: FixedArray<int> = [1, 2, 3]
        let ro: readonly int[] = [...b]
        arktest.assertEQ(ro[0], 1)
        arktest.assertEQ(ro.length, 3)
    });

    myTestsuite.addTest("MutableTupleToReadonlyArray", () => {
        let b: [int, int] = [1, 2]
        let ro: readonly int[] = [...b]
        arktest.assertEQ(ro[0], 1)
        arktest.assertEQ(ro[1], 2)
    });

    myTestsuite.addTest("ReadonlyTupleToMutableArray", () => {
        let a: readonly [int, int, int] = [1, 2, 3]
        let rw: int[] = [...a]
        rw[1] = 11
        arktest.assertEQ(rw[1], 11)
        arktest.assertEQ(a[1], 2)
    });

    return myTestsuite.run();
}
