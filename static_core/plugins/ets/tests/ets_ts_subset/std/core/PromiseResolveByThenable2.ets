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
    then<U, E>(onFulfilled?: (value: string) => U | PromiseLike<U>, onRejected?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        return Promise.resolve(onFulfilled!('resolved')) as PromiseLike<U | E>;
    }
};

let thenable = new Thenable();
let p = Promise.resolve<string>(thenable);
p.then<void, void>((value: string): void => {
    if (value == 'resolved') {
        console.log('Test passed.');
    } else {
        console.log('Test failed. The promise should be resolved by the provided value.');
    }
}, (error: Error): void => {
    console.log('Test failed. The promise should be resolved.');
});
