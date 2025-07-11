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
import { Job } from "std/core"

function return_value(i: int): String {
    return "value " + i;
}

function n_cor_launch(n_cor: int): int {
    let p: (Job<String> | undefined)[] = new (Job<String> | undefined)[n_cor];
    for (let i = 0; i < n_cor; ++i) {
        p[i] = launch<String, (i: int) => String>(return_value, i);
    }
    for (let i = 0; i < n_cor; ++i) {
        let val = (p[i]!).Await();
        if(val != return_value(i)) {
            console.println("p[" + i + "] = \"" + val + "\" instead of \"" + return_value(i) + "\"");
            return 1;
        }
    }
    return 0;
}

function throw_exception(): Object {
    throw new Error();
}

function n_cor_exection_launch(n_cor: int): int { 
    let p: (Job<Object> | undefined)[] = new (Job<Object> | undefined)[n_cor];
    for (let i = 0; i < n_cor; ++i) {
        p[i] = launch<Object, () => Object>(throw_exception);
    }
    for (let i = 0; i < n_cor; ++i) {
        try {
            (p[i]!).Await();
        } catch (e) {
            if (!(e instanceof Error)) {
              console.println("Expected Error but another exception has been thrown!: " + e);
              return 1;
            }
            continue;
        }
        console.println("Was no error throwing");
        return 1;
    }
    return 0;
}

// !1581, molotkovmikhail - test should be start with coroutine pool to test it
export function main(): int {
    // Create 5 coroutines and save 5 coroutines
    if(n_cor_launch(5) != 0) {
        console.println("First launch of 5 coroutines failed!!");
        return -1;
    }
    // Reuse 5 coroutines
    if(n_cor_launch(5) != 0) {
        console.println("Second launch of 5 coroutines failed!!");
        return -1;
    }
    // Create 10 coroutines: reuse 5 coroutines and create new 5 ones. Save 10 coroutines 
    if(n_cor_launch(10) != 0) {
        console.println("First launch of 10 coroutines failed!!");
        return -1;
    }
    // Reuse 10 coroutines
    if(n_cor_launch(10) != 0) {
        console.println("Second launch of 10 coroutines failed!!");
        return -1;
    }
    // Reuse 5 coroutines and throw exception in them
    if(n_cor_exection_launch(5) != 0) {
        console.println("First launch of 5 throw-error coroutines failed!!");
        return -1;
    }
    // Reuse 10 coroutines
    if(n_cor_launch(10) != 0) {
        console.println("First launch of 10 coroutines after 5 throw-error ones failed!!");
        return -1;
    }
    return 0;
}
