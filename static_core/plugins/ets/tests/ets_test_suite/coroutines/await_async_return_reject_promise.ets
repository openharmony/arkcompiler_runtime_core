/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

async function returnRejectedPromise(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        reject(new Error('Reject'));
    })
}

async function returnRejectingPromise(): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        setTimeout(() => {
            reject(new Error('Reject'));
        }, 10);
    })
}

function main() {
    try {
        await returnRejectedPromise();
        assertTrue(false, 'Awaited a reject Promise should throw the reason.');
    } catch (err: Error) {
        assertTrue(err.toString().contains('Reject', 0), 'Awaited value of rejected promise is different\
                                                          from the expected value. Actual value: ' + err);
    }
    try {
        await returnRejectingPromise();
        assertTrue(false, 'Awaited a reject Promise should throw the reason.');
    } catch (err: Error) {
        assertTrue(err.toString().contains('Reject', 0), 'Awaited value of rejected promise is different\
                                                          from the expected value. Actual value: ' + err);
    }
}
