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
    The execution of a nested object literal completes abruptly if so does
    the execution of an inner object literal.
---*/

class Inner {
    value: number
}

class Outer {
    inner?: Inner
    name?: string
}

let evaluationOrder: string = ""

function track(s: string, n: number): number {
    evaluationOrder += s
    if (n < 0) {
        throw new NullPointerError()
    }
    return n
}

let checkException = (e: Error): boolean => { return e instanceof NullPointerError }

function foo(): void {
    let obj: Outer = {
        inner: { value: track("i", -1) },
        name: track("n", 0).toString()
    }
}

function main(): void {
    evaluationOrder = ""
    arktest.expectThrow(() => foo(), checkException)
    arktest.assertEQ(evaluationOrder, "i")
}
