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

let p = new Promise<number>((resolve: (v: number) => void, _: (error: Error) => void) => {
    resolve(7);
});

p.then<void, void>((value: number): void => {
    if (value != 7) {
        console.log('Test failed. The promise shoule be fulfilled by 7 but got ' + value + '.');
        return;
    }
    console.log('Test passed.');
}, (err: Error): void => {
    console.log('Test failed. The promise shouldn`t be rejected.');
});
