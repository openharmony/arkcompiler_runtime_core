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

let thenable = Promise.resolve();
let resolve: ((value: string) => void)|null = null;
let reject: ((err: Object) => void)|null = null;
let p = new Promise<string>((_resolve: (value: string) => void, _reject: (err: Object) => void): void => {
    resolve = _resolve;
    reject = _reject;
});

p.then<void, void>((value: string): void => {
    console.log("Test passed.");
}, (err: Object): void => {
    console.log("Test failed. The promise should not be rejected.");
});

resolve!("abc");
reject!(thenable);
