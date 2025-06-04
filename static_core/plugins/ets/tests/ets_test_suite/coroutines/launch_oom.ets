/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

let counter = 0;

function body(): Object {
    return new Object();
}

export function main(): int {
    try {
        for (let i = 0; i < 50000; ++i) {
            launch body();
            ++counter;
        }
    } catch (e) {
        if (e instanceof OutOfMemoryError) {
            return 0;
        } else {
            console.println("Wrong exception thrown!");
            return 1;
        }
    }
    console.println("No exception thrown but should be!");
    return 1;
}
