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

function sortStringTest(): int
{
    let size: int = 10;
    let arr = new Array<String>();
    let expected = new Array<String>();
    let sorted = new Array<String>();

    for (let i = 0; i < size; ++i) {
        arr.push("value " + i);
        expected.push("value " + i);
    }

    try {
        sorted = arr.toSorted();
    } catch (e) {
        return 1;
    }

    for (let i = 0; i < size; ++i) {
        if (expected[i] != sorted[i]) {
            console.log('Wrong result');
            console.log('Element ' + i + ': expected ' + expected[i] + ', got ' + sorted[i]);
            return 1;
        }
    }
    return 0;
}

function runTest(result: int, message: String): int
{
    if (result == 0) {
        return 0;
    }
    console.log("FAILED: " + message);
    return 1;
}

function main(): int
{
    return runTest(sortStringTest(), "Test sort for Array<String>");
}