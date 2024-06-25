/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import {AtomicFlag} from "std/debug/concurrency"

class Event {
    public Wait() {
        while (this.flag.get() != true) {
            Coroutine.Schedule();
        }
    }

    public Fire() {
        this.flag.set(true);
    }

    private flag = new AtomicFlag(false);
};

let thenEvent = new Event();
let promiseEvent = new Event();
let counter = 0;

function step(expected: int) : void {
    assert (counter == expected) : "Test failed on step: " + Number.toString(expected);
    counter++;
}

function foo() {}

function bar(p: Promise<void>) : void {
    p.then<void>((): void => {
        // Wait for main coro will process the rest callbacks
        thenEvent.Wait();
        step(4);
    });
    // Wait for adding p2.then callback
    promiseEvent.Wait();
}

function main() {
    let p1 = launch foo();
    let p2 = launch bar(p1);

    p1.then<void>((): void => {
        step(0);
        // Add then callback to the promise queue
        p2.then<void>((): void => {
            step(2);
        });
        promiseEvent.Fire();
    });

    p1.then<void>((): void => {
        step(1);
        // Await on p2 to add then callback to the coro queue
        await p2;
        p2.then<void>((): void => {
            step(3);
            thenEvent.Fire();
        });
    });
}
