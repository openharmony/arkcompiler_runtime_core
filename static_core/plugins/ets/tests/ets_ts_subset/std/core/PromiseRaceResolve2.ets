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

let resolve: ((value: string) => void) | null = null;
let reject: ((err: Error) => void) | null = null;
let p1 = new Promise<string>((_resolve: (v: string)=>void): void => {
    resolve = _resolve;
});
let p2 = new Promise<string>((_: (v: string)=>void, _reject: (err: Error)=>void): void => {
    reject = _reject;
});
Promise.race<string>([p1, p2]).then<void, void>((value: Object): void => {
    console.log('Test failed. The promise should not be resolved.');
}, (err: Error): void => {
    if (!(err instanceof Error) || (err as Error).message != 'xyz') {
        console.log('Test failed. The promise should be rejected by the specified value.');
        return;
    }
    console.log('Test passed.');
});
reject!(new Error('xyz'));
resolve!('abc');
