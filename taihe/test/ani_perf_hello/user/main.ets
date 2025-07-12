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
import * as hello from "hello";

loadLibrary("ani_perf_hello");

function runTestFunction(prompt: string, n: int, fn: () => void): void {
    const s = Chrono.nanoNow();
    for (let i: int = 0; i < n; i++) {
        fn();
    }
    const t = Chrono.nanoNow();
    let durationNs: double = (t - s) / n;
    console.log(`>> Done ${prompt}: ${durationNs / 1000} μs.`);
}

function testData() {
    hello.setData({
        a: "hello",
        b: 1.5,
        c: 12,
        d: true,
        f: false,
        g: true,
        e: false,
    });
    let data = hello.getData();
}

function testRecord() {
    hello.setRecord({
        "key1": "value1",
        "key2": "value2",
        "key3": "value3",
    });
    let rec = hello.getRecord();
}

function main() {
    runTestFunction("testData preheat", 500, testData);
    runTestFunction("testData test", 2000, testData);
    runTestFunction("testRecord preheat", 500, testRecord);
    runTestFunction("testRecord test", 2000, testRecord);
}
