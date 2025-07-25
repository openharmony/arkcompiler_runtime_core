/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
class Thenable implements PromiseLike<string> {
    constructor(e: Error) {
        this.error = e;
    }
    then<U, E>(_?: (value: string) => U | PromiseLike<U>, onRejected?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        return Promise.resolve().then<E>((): E | PromiseLike<E> => {
            return onRejected!(this.error);
        });
    }
    private error: Error;
};

let error = new Error('abc');
let thenable = new Thenable(error);
Promise.allSettled([thenable]).then<void, void>((values: PromiseSettledResult<string>[]): void => {
    if (values.length != 1) {
        console.log('Test failed. Expected an array of length 1 but got length ' + values.length + '.');
        return;
    }
    if (values[0].status != 'rejected') {
        console.log('Test failed. Expected the first element has status \'rejected\' but got \'' +
            values[0].status + '\'.');
        return;
    }
    if ((values[0] as PromiseRejectedResult).reason.message != 'abc') {
        console.log('Test failed. The promise should be rejected by the provided value.');
        return;
    }
    console.log('Test passed.');
}, (error: Error): void => {
    console.log('Test failed. The promise should not be rejected.');
});
