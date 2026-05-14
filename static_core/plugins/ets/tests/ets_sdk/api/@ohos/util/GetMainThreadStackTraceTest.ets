/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import {util} from "@ohos.util";

function main(): int {
    const suite = new arktest.ArkTestsuite("GetMainThreadStackTrace Test");

    suite.addTest("Get Main Thread Trace from MainThread.", getMainThreadTraceFromMainThread);
    suite.addTest("Get Main Thread Trace from EAWorker.", getMainThreadTraceFromEAWorker);
    suite.addTest("Get Main Thread Trace from RecursiveCall.", getMainThreadTraceFromRecursiveCall);
    suite.addTest("Get Main Thread Trace from MultiLevelCall.", getMainThreadTraceFromMultiLevelCall);
    suite.run();
    return 0;
}

function checkMainThreadTraceFormat(trace: string): void {
    arktest.assertTrue(trace.startsWith("\tat"));
    arktest.assertTrue(trace.endsWith(")\n"));
}

function getMainThreadTraceFromMainThread(): void {
    let trace = util.getMainThreadStackTrace();
    checkMainThreadTraceFormat(trace);
}

function getMainThreadTraceFromEAWorker(): void {
    const workerInstance = new EAWorker("worker");
    workerInstance.start();
    let workerHandler = new concurrency.MessageHandler((msg: concurrency.Message) => {
        let data = msg.getObject() as string;
        arktest.assertEQ(data, "Hello world");
        let trace = util.getMainThreadStackTrace();
        checkMainThreadTraceFormat(trace);
    }, workerInstance);

    let workerMessage = new concurrency.Message(1, "Hello world", workerHandler);
    workerHandler.sendMessage(workerMessage);
    workerInstance.join();
}

function getMainThreadTraceFromRecursiveCall(): void {
    let trace = util.getMainThreadStackTrace();
    checkMainThreadTraceFormat(trace);
    arktest.assertTrue(trace.includes("getMainThreadTraceFromRecursiveCall"), 
        "Trace should include getMainThreadTraceFromRecursiveCall");
}

function multiLevelHelperA(): string {
    return util.getMainThreadStackTrace();
}

function multiLevelHelperB(): string {
    return multiLevelHelperA();
}

function multiLevelHelperC(): string {
    return multiLevelHelperB();
}

function getMainThreadTraceFromMultiLevelCall(): void {
    let trace = multiLevelHelperC();
    checkMainThreadTraceFormat(trace);
    arktest.assertTrue(trace.includes("multiLevelHelperA"), "Trace should include multiLevelHelperA");
    arktest.assertTrue(trace.includes("multiLevelHelperB"), "Trace should include multiLevelHelperB");
    arktest.assertTrue(trace.includes("multiLevelHelperC"), "Trace should include multiLevelHelperC");
    arktest.assertTrue(trace.includes("getMainThreadTraceFromMultiLevelCall"),
        "Trace should include getMainThreadTraceFromMultiLevelCall");
}
