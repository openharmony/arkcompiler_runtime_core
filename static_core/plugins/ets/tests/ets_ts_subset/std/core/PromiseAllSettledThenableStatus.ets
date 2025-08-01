/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

class Resolver implements PromiseLike<number> {
    then<U, E>(onFulfilled?: (value: number) => U | PromiseLike<U>, _?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        return Promise.resolve().then<U>((): U | PromiseLike<U> => {
            return onFulfilled!(42);
        })
    }
}

class LateRejector implements PromiseLike<number> {
    then<U, E>(onFulfilled?: (value: number) => U | PromiseLike<U>, onRejected?:
            (err: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        return Promise.resolve().then<U>((): U | PromiseLike<U> => {
            let v = onFulfilled!(9);
            onRejected!(new Error('err'));
            return v;
        });
    }
}

let thenable: PromiseLike<number>[] = [new Resolver(), new LateRejector()];
Promise.allSettled(thenable).then<void, void>((values:PromiseSettledResult<number>[]): void => {
    let v = (values[0] as PromiseFulfilledResult<number>).value;
    if(v != 42) {
        console.log('Test failed. Expected 42, but got ' + v + '.');
    }
    if(values[0].status != 'fulfilled' || values[1].status != 'fulfilled') {
        console.log('Test failed. Both the status should be fulfilled.');
    }
    v = (values[1] as PromiseFulfilledResult<number>).value;
    if(v != 9) {
        console.log('Test failed. Expected 9, but got ' + v + '.');
    }
    console.log('Test passed.');
},(e: Error): void => {
    console.log('Test failed. The promise should not be rejected.');
}).catch((error: Error) : void => {
    console.log('Test failed. The error is ' + error.message + '.');
});
