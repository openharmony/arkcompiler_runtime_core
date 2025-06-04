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
class PoisonedIterable<T> implements Iterable<T> {
    constructor(err: Error) {
        this.error = err;
    }

    [Symbol.iterator](): Iterator<T> {
        throw this.error;
    }

    private error: Error;
}

let error = new Error();
Promise.race<Object>(new PoisonedIterable<Object>(error)).then<void, void>((value: Object): void => {
    console.log("Test failed. The promise should not be resolved.");
}, (err: Object): void => {
    if (err != error) {
        console.log('Test failed. The promise should be rejected by the specified value.');
        return;
    }
    console.log("Test passed.");
});
