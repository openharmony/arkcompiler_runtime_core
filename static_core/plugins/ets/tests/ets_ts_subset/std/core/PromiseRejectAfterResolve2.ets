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

let thenable = new Error('abc');
let p = new Promise<string>((resolve: (value: string) => void, reject: (err: Error) => void): void => {
    resolve('abc');
    reject(thenable);
});

p.then<void, void>((value: string): void => {
    if (value != 'abc') {
        console.log('Test failed. Expected value \'abc\' but got ' + value + '.');
        return;
    }
    console.log('Test passed.');
}, (err: Error): void => {
    console.log('Test failed. The promise should not be rejected.');
});
