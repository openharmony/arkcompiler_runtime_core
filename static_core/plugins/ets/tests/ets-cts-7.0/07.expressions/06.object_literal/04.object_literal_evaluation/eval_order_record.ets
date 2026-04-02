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
    Name-value pairs of the Record object literal are evaluated from left to right
    in the textual order they occur in the source code.
---*/

let evaluationOrder: number[] = []

function track(n: number): string {
    evaluationOrder.push(n)
    return n.toString()
}

function main(): void {
    evaluationOrder = []
    let r: Record<string, string> = {
        "c": track(1),
        "a": track(2),
        "b": track(3),
    }

    arktest.assertEQ(evaluationOrder.length, 3)
    arktest.assertEQ(evaluationOrder[0], 1)
    arktest.assertEQ(evaluationOrder[1], 2)
    arktest.assertEQ(evaluationOrder[2], 3)
    arktest.assertEQ(r["c"], "1")
    arktest.assertEQ(r["a"], "2")
    arktest.assertEQ(r["b"], "3")
}
