/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

import { launch } from "std/concurrency"

async function create_7000_coro_test(): Promise<void> {
    const coro_to_create = 6999;
    let j = new CompletableJob<void>();
    let jobs = new Array<Promise<void>>();

    let wait_func = (): void => { j.Await(); }

    for (let i = 1; i < coro_to_create; ++ i) {
        jobs.push(launch(wait_func));
    }

    j.finish();
    for (let job of jobs) {
        await job;
    }
}

function main() {
    let testsuite = new arktest.ArkTestsuite("EAWorker");
    testsuite.addAsyncTest("create_7000_coro_test", create_7000_coro_test)
    return testsuite.run();
}
