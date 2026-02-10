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
    The execution of a Record object literal completes abruptly if so does
    the execution of a name-value pair expression. Subsequent pairs are not evaluated.
---*/

let evaluationOrder: string = ""

function track(n: number): number {
    evaluationOrder += n.toString()
    if (n == 2) {
        throw new Error("abrupt")
    }
    return n
}

let checkException = (e: Error): boolean => { return e instanceof Error }

function foo(): void {
    let r: Record<string, number> = {
        "a": track(1),
        "b": track(2),
        "c": track(3),
    }
}

function main(): void {
    evaluationOrder = ""
    arktest.expectThrow(() => foo(), checkException)
    arktest.assertEQ(evaluationOrder, "12")
}
