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

function id<T>(v: T): T {
    return v;
}

Promise.resolve<string | undefined | null>(null)
    .then<string | undefined | null>(id)
    .catch((e: Error): void => {
        console.log('Test failed: ' + e);
    });
Promise.resolve<string | undefined | null>(undefined)
    .then<string | undefined | null>(id)
    .catch((e: Error): void => {
        console.log('Test failed: ' + e);
    });
Promise.all<string | undefined | null>([Promise.resolve<string | undefined | null>(undefined)])
    .then<Array<string | undefined | null>>(id)
    .catch((e: Error): void => {
        console.log('Test failed: ' + e);
    });
Promise.all<string | undefined | null>([Promise.resolve<string | undefined | null>(null)])
    .then<Array<string | undefined | null>>(id)
    .catch((e: Error): void => {
        console.log('Test failed: ' + e);
    });
Promise.allSettled<string | undefined | null>([Promise.resolve<string | undefined | null>(undefined)])
    .then<PromiseSettledResult<string | undefined | null>[]>(id)
    .catch((e: Error): void => {
        console.log('Test failed: ' + e);
    });
Promise.allSettled<string | undefined | null>([Promise.resolve<string | undefined | null>(null)])
    .then<PromiseSettledResult<string | undefined | null>[]>(id)
    .catch((e: Error): void => {
        console.log('Test failed: ' + e);
    });
Promise.race<string | undefined | null>([Promise.resolve<string | undefined | null>(undefined)])
    .then<string | undefined | null>(id)
    .catch((e: Error): void => {
        console.log('Test failed: ' + e);
    });
Promise.race<string | undefined | null>([Promise.resolve<string | undefined | null>(null)])
    .then<string | undefined | null>(id)
    .catch((e: Error): void => {
        console.log('Test failed: ' + e);
    });
Promise.any<string | undefined | null>([Promise.resolve<string | undefined | null>(undefined)])
    .then<string | undefined | null>(id)
    .catch((e: Error): void => {
        console.log('Test failed: ' + e);
    });
Promise.any<string | undefined | null>([Promise.resolve<string | undefined | null>(null)])
    .then<string | undefined | null>(id)
    .catch((e: Error): void => {
        console.log('Test failed: ' + e);
    });
