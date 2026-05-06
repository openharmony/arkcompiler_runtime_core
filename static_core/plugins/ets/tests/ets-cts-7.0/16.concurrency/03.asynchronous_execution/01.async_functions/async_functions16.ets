/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
desc: Test cases for various usage scenarios of asynchronous functions.
      Promise.allSettled in asynchronous functions.
---*/

async function allSettledExample(): Promise<void> {
    const promises = [
        Promise.resolve("Resolved"),
        Promise.reject(new Error("Rejected")),
    ];
    const results = await Promise.allSettled(promises);
    arktest.assertEQ(results.length, 2);
    arktest.assertEQ(results[0].status, "fulfilled");
    arktest.assertEQ((results[0] as PromiseFulfilledResult<string>).value, "Resolved");
    arktest.assertEQ(results[1].status, "rejected");
    arktest.assertEQ((results[1] as PromiseRejectedResult).reason instanceof Error, true);
    arktest.assertEQ(((results[1] as PromiseRejectedResult).reason as Error).message, "Rejected");
}

async function startTest() {
    await allSettledExample();
}

function main(): void {
  waitForCompletion(startTest);
}
