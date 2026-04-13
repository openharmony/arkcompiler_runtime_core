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
  A Promise class allows to register callbacks via .then(), .catch(),
  and .finally() methods that are called upon Promise resolution
  and/or rejection. If multiple callbacks are registered from jobs on
  the same worker thread, execution order matches registration order.
tags: [no-warmup]
---*/

async function resolving(): Promise<int> {
    return 25;
}

async function resolvingForFinally(): Promise<int> {
    return 18;
}

async function resolvingStr(): Promise<string> {
    return "a";
}

async function failing(): Promise<void> {
    throw new Error("task failed");
}

async function failingForFinally(): Promise<void> {
    throw new Error("rejected");
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Promise callbacks");

    suite.addAsyncTest(".then() transforms the resolved value", async (): Promise<void> => {
        let result = await resolving().then((v: int): int => v + 10);
        arktest.assertEQ(result, 35);
    });

    suite.addAsyncTest(".finally() on resolve does not alter the result", async (): Promise<void> => {
        let finallyCalled = false;
        let result = await resolvingForFinally()
            .finally((): void => { finallyCalled = true; });
        arktest.assertEQ(result, 18);
        arktest.assertTrue(finallyCalled);
    });

    suite.addAsyncTest(".catch() handles rejection with fallback value", async (): Promise<void> => {
        let result = await failing()
            .catch((e: Error | null): int => 55);
        arktest.assertEQ(result, 55);
    });

    suite.addAsyncTest(".catch() + .finally() chain on rejected promise", async (): Promise<void> => {
        let finallyCalled = false;
        let fallback = await failingForFinally()
            .catch((e: Error | null): int => 77)
            .finally((): void => { finallyCalled = true; });
        arktest.assertEQ(fallback, 77);
        arktest.assertTrue(finallyCalled);
    });

    suite.addAsyncTest("Chained .then() callbacks execute in registration order", async (): Promise<void> => {
        let result = await resolvingStr()
            .then((v: string): string => v + "b")
            .then((v: string): string => v + "c")
            .then((v: string): string => v + "d");
        arktest.assertEQ(result, "abcd");
    });

    return suite.run();
}
