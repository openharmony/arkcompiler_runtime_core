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
  All errors thrown in an ArkTS program should either have an ability to be
  handled by the developer or considered uncaught. In case of abnormal job
  completion the corresponding promise gets rejected and the original error
  is not considered uncaught.
tags: [no-warmup]
---*/

async function failingJob(): Promise<void> {
    throw new Error("job error");
}

async function throwingTask(): Promise<void> {
    throw new Error("handled error");
}

async function innerTask(): Promise<void> {
    throw new Error("inner failure");
}

async function outerTask(): Promise<void> {
    await innerTask();
}

function main(): int {
    let suite = new arktest.ArkTestsuite("Error handling policy");

    suite.addAsyncTest("Promise rejection absorbed via await try/catch", async (): Promise<void> => {
        try {
            await failingJob();
            arktest.assertTrue(false, "Should not reach here");
        } catch (e) {
            arktest.assertEQ((e as Error).message, "job error");
        }
    });

    suite.addAsyncTest("Rejection handled via .catch() is not uncaught", async (): Promise<void> => {
        let result = await throwingTask()
            .catch((e: Error | null): int => {
                arktest.assertEQ((e as Error).message, "handled error");
                return 100;
            });
        arktest.assertEQ(result, 100);
    });

    suite.addAsyncTest("Errors propagate through nested async functions", async (): Promise<void> => {
        try {
            await outerTask();
            arktest.assertTrue(false, "Should not reach here");
        } catch (e) {
            arktest.assertEQ((e as Error).message, "inner failure");
        }
    });

    return suite.run();
}
