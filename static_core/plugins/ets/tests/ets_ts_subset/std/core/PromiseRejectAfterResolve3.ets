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

let error = new Error('abc');
let reject: ((err: Error) => void) | null = null;
let p1 = new Promise<string>((_: (value: string) => void, _reject: (err: Error) => void): void => {
    reject = _reject;
});
let p2 = Promise.resolve<Object>(p1);
if (p1 !== p2) {
    throw new Error('Test failed. Expected the same object.');
}

p2.then<void, void>((value: Object): void => {
    console.log('Test failed. Expected the promise p2 to be rejected.');
}, (err: Error): void => {
    if (err !== error) {
        console.log('Test failed. Expetec the promise is rejected by the provided value.');
    } else {
        console.log('Test passed.');
    }
});
reject!(error);
