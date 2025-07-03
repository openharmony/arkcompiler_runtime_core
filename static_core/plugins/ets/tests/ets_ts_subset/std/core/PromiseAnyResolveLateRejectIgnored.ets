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
class Resolve implements PromiseLike<string> {
    then<U, E>(onFulfilled?: (value: string) => U | PromiseLike<U>, _?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        return Promise.resolve().then<U>((): U | PromiseLike<U> => {
            return onFulfilled!('abc');
        })
    }
}

class LateReject implements PromiseLike<string> {
    then<U, E>(onFulfilled?: (value: string) => U | PromiseLike<U>, onRejected?:
            (error: Error) => E | PromiseLike<E>): PromiseLike<U | E> {
        return Promise.resolve().then<U>((): U | PromiseLike<U> => {
            let v = onFulfilled!('def');
            onRejected!(new Error('xyz'));
            return v;
        });
    }
}

let thenable: PromiseLike<string>[] = [new Resolve(), new LateReject()];
Promise.any(thenable).then<void, void>((value: string): void => {
    if (value != 'abc') {
        console.log('Test failed. The promise should be resolve by the specified value.');
        return;
    }
    console.log('Test passed.');
}, (err: AggregateError): void => {
    console.log('Test failed. The promise should not be rejected.');
});
