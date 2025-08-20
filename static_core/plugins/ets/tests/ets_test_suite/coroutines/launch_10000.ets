/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

function create_10000_coro_test() {
    const coro_to_create = 10000;
    let arr = new Int32Array(new ArrayBuffer(16384))

    let wait_func = (arr: Int32Array) => {Atomics.wait(arr, 0, 0);}
    let mainWaitJob = launch<void, (arr: Int32Array) => void> (wait_func, arr);
    let awaitIt = ()=>{ mainWaitJob.Await(); }

    for (let i = 1; i < coro_to_create; ++ i) {
        launch<void, () => void> (awaitIt);
    }

    Atomics.store(arr, 0, 1);
    Atomics.notify(arr, 0);
}

function main() {
    let testsuite = new arktest.ArkTestsuite("EAWorker");
    testsuite.addTest("create_10000_coro_test", create_10000_coro_test)
    return testsuite.run();
}
