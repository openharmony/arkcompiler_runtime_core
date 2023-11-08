/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

let LOG_ENABLED = false;
function log(m: String): void {
    if (LOG_ENABLED) {
        console.println(m);
    }
}

function log_error(m: String): void {
    console.println("ERROR: " + m);
}

///////

type NullableObject = Object | null;

async function async_chain_element(caller_wid: int, counter: int): Promise<int> {
    let id_worker: int = CoroutineExtras.GetWorkerId();
    if (caller_wid != id_worker) {
        return 1;
    }
    if (counter <= 0) {
        return 0;
    }
    return await async_chain_element(caller_wid, counter - 1);
}

function async_function_chain(caller_wid: int, chain_len: int): int {
    return await async_chain_element(caller_wid, chain_len);
}

function compare_worker_ids_async(chain_len: int): int {
    log("Testing async function chain");

    let id_main: int = CoroutineExtras.GetWorkerId();
    let result = async_function_chain(id_main, chain_len);
    if (result == 0) {
        log("Successfully ran the async function chain")
    } else {
        log("Failed to run the async function chain")
    }
    return result;
}

function return_worker_id(): Int {
    return CoroutineExtras.GetWorkerId();
}

function compare_worker_ids(): int {
    log("Testing launch in a separate worker");

    let id_main: int = CoroutineExtras.GetWorkerId();
    let id_coro: int = (await launch return_worker_id()) as int;
    if (id_main != id_coro) {
        log("Successfully ran coro in a separate worker. Main WID: " + id_main + ", Coro WID: " + id_coro);
        return 0;
    } else {
        log_error("Failed to run coro in a separate worker. Main WID: " + id_main + ", Coro WID: " + id_coro);
        return 1;
    }
}

function job(n: int): NullableObject {
    log("Job with " + n + " iterations in a loop started");
    for (let i = 0; i < n; ++i) {
        //
    }
    return null;
}

function run_batch_launch(batch_size: int, iters: int): int {
    log("Testing batch launch of " + batch_size + " coroutines");
    for (let i = 0; i < batch_size; ++i) {
        launch job(iters);
    }
    return 0;
}

function await_chain(n: int): NullableObject {
    if (n > 0) {
        await launch await_chain(n-1);
    }
    return null;
}

function run_await_chain(len: int): int {
    log("Testing await chain of " + len + " items");
    await launch await_chain(len);    
    return 0;
}

function simple(): NullableObject {
    return null;
}

function run_batch_await(batch_size: int): int {
    log("Testing batch await of " + batch_size + " coroutines");
    for (let i = 0; i < batch_size; ++i) {
        let p = launch simple();
        await p;
    }
    return 0;
}

export function main(): int {
    // Test 1. Run an async function chain and check that all worker IDs are equal to MAIN's
    if (compare_worker_ids_async(5) != 0) {
        log_error("compare async function worker IDs test failed");
        return 1;
    }

    // Test 2. Run a coroutine and make sure that its worker ID is different from MAIN
    if (compare_worker_ids() != 0) {
        log_error("compare worker IDs test failed");
        return 1;
    }

    // Test 3. Run a batch of coroutines without await so they would be distributed to different workers
    if (run_batch_launch(100, 500) != 0) {
        log_error("batch launch test failed");
        return 1;
    }

    // Test 4. Run a chain of several coroutines that await each other. Coroutines will run on different workers.
    if (run_await_chain(50) != 0) {
        log_error("await chain test failed");
        return 1;
    }

    // Test 5. Run coroutines on different workers and await them.
    if (run_batch_await(100) != 0) {
        log_error("batch await test failed");
        return 1;
    }

    return 0;
}