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

let p = Promise.reject<number>(new Error('5'));

p.then<void, void>((value: number): void => {
    console.log('Test failed. The promise should not be fulfilled.');
}, (err: Error): void => {
    if (!(err instanceof Error) || (err as Error).message != '5') {
        console.log('Test failed. The promise should be rejected with the provided value.');
        return;
    }
    console.log('Test passed.');
});
