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

import {CoroutineExtras} from "std/debug/concurrency"

class TestCoroutineState {
    constructor(id: int) {
        this.id = id;
    }

    CheckIdAndStep() {
        assert CoroutineExtras.getCoroutineId() == this.id : "The callback was called in the wrong coroutine";
        this.callbackCounter++;
    }

    GetCallbackCounter(): int {
        return this.callbackCounter;
    }

    private id = 0;
    private callbackCounter = 0;
}


let mainCoro : TestCoroutineState;
let barCoro : TestCoroutineState;

function foo() {}

function bar(p1: Promise<void>) {
    barCoro = new TestCoroutineState(CoroutineExtras.getCoroutineId());
    p1.finally((): void => {
        barCoro.CheckIdAndStep();
    });
    p1.then<void>((): void => {
        barCoro.CheckIdAndStep();
    });
    p1.catch<void>((error: NullishType): void => {
        assert false : 'The callback must not be called because promise is resolved';
    });
    // We need to process all callbacks before checking the callback counter in the main coroutine
    await p1;
    throw new Error();
}

function main() {
    let p1 = launch foo();
    let p2 = launch bar(p1);

    mainCoro = new TestCoroutineState(CoroutineExtras.getCoroutineId());

    p2.finally((): void => {
        mainCoro.CheckIdAndStep();
    });
    p2.then<void>((): void => {
        assert false : 'The callback must not be called because promise is rejected';
    });
    p2.catch<void>((error: NullishType): void => {
        mainCoro.CheckIdAndStep();
    });
    try {
        await p2;
    } catch (e: Error) {
        let counter = mainCoro.GetCallbackCounter() + barCoro.GetCallbackCounter();
        assert (counter == 4) : Number.toString(4 - counter) + " of the promise callbacks were not called";
    }
}
