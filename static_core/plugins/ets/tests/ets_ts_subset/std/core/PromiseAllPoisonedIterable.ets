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
    [Symbol.iterator](): Iterator<T> {
        throw new Error("error");
    }
}

Promise.all<Object>(new PoisonedIterable<Object>()).then<void, void>((value: Array<Object>): void => {
    console.log("Test failed. The promise should not be resolved.");
}, (err: Error): void => {
    if (!(err instanceof Error)) {
        console.log("Test failed. The promise should be rejected by the Error.");
        return;
    }
    console.log("Test passed.");
});
