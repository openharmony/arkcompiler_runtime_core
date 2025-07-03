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

let error = new Error();

class Thenable implements PromiseLike<Error> {
    then<U, E>(_?: (value: Error) => U | PromiseLike<U>, onRejected?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        return Promise.resolve<E>(onRejected!(error));
    }
}

Promise.any([new Thenable()]).then<void, void>((value: Error): void => {
    console.log('Test failed. The promise should not be resolved.');
}, (err: AggregateError): void => {
    if (err.errors.length != 1) {
        console.log('Test failed. Expected errors count 1 but got ' + err.errors.length + '.');
        return;
    }
    if (err.errors[0] != error) {
        console.log('Test failed. The promise should be rejected by the specified value.');
        return;
    }
    console.log('Test passed.');
});
