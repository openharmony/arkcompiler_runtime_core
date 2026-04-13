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
    Type erasure for Awaited<T>. Per spec:
    - If T is neither a type parameter nor a subtype of Promise, Effective type is (T)
    - If T is a Promise<U>, Effective type is Awaited<U> (the Effective type of U)
    - If T is a type parameter with in variance, Effective type is never
    - If T is a type parameter with out variance or no variance, Effective type is upperBound(T)
---*/

type AwaitedNum = Awaited<number>
type AwaitedStr = Awaited<string>
type AwaitedPromise = Awaited<Promise<number>>
type AwaitedNested = Awaited<Promise<Promise<string>>>
type AwaitedUnion = Awaited<Promise<number> | string>

class OutBox<out T extends Number> {
    readonly val: T
    constructor(v: T) { this.val = v }
    checkAwaited(p: Object): boolean {
        return p instanceof Awaited<T>
    }
}

class InvariantBox<T extends Number> {
    readonly val: T
    constructor(v: T) { this.val = v }
    checkAwaited(p: Object): boolean {
        return p instanceof Awaited<T>
    }
}

function main(): int {
    let suite = new arktest.ArkTestsuite("AwaitedTypeErasure");

    suite.addTest("Awaited of non-Promise type", () => {
        let val: AwaitedNum = 39.0
        arktest.assertEQ(val, 39.0)
    });

    suite.addTest("Awaited of string type", () => {
        let val: AwaitedStr = "awaited"
        arktest.assertEQ(val, "awaited")
    });

    suite.addTest("Awaited of Promise unwraps to inner type", () => {
        let val: AwaitedPromise = 45.0
        arktest.assertEQ(val, 45.0)
    });

    suite.addTest("Awaited of nested Promise", () => {
        let val: AwaitedNested = "nested"
        arktest.assertEQ(val, "nested")
    });

    suite.addTest("Awaited of union with Promise", () => {
        let val1: AwaitedUnion = 46.0
        let val2: AwaitedUnion = "text"
        arktest.assertEQ(val1, 46.0)
        arktest.assertEQ(val2, "text")
    });

    suite.addTest("out variance: Awaited<T> effective type is upperBound", () => {
        let box = new OutBox<Double>(48.0)
        arktest.assertTrue(box.checkAwaited(48.0))
        arktest.assertFalse(box.checkAwaited("not a number"))
    });

    suite.addTest("no variance: Awaited<T> effective type is upperBound", () => {
        let box = new InvariantBox<Double>(36.0)
        arktest.assertTrue(box.checkAwaited(36.0))
        arktest.assertFalse(box.checkAwaited("not a number"))
    });

    return suite.run();
}
