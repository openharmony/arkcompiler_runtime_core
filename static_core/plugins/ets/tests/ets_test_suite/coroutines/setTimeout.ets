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

import { launch } from "std/concurrency"

let zeroTimeout = 0;
let shortTimeout = 10;
let mediumTimeout = 100;
let longTimeout = 1000;

function setTimeoutTest() {
    let flag = false;
    let start = Date.now();
    setTimeout(() => {
        arktest.assertTrue(flag);
        arktest.assertTrue(Date.now() - start >= shortTimeout);
    }, shortTimeout);
    flag = true;
}

function firstTimerIdTest() {
    let id = setTimeout(() => {}, 0);
    arktest.assertTrue(id == 1);
}

function clearTimeoutTest() {
    let id = setTimeout(() => { arktest.assertTrue(false); }, mediumTimeout);
    clearTimeout(id);
}

function clearIntervalTestFromCallback() {
    let flag = true;
    let id: number = -1;
    id = setInterval(() => {
        arktest.assertTrue(flag);
        flag = false;
        arktest.assertTrue(id != -1);
        clearInterval(id);
    }, zeroTimeout);
}

function clearIntervalFromAnotherTimer() {
    let flag = false;
    let id = setInterval(() => { flag = true; }, shortTimeout);
    setTimeout(() => {
        arktest.assertTrue(flag);
        clearInterval(id);
    }, mediumTimeout);
}

function clearTimeoutFromAnotherWorkerTest() {
    let id = setTimeout(() => { arktest.assertTrue(false); }, longTimeout);
    launch<void, (i: number) => void>(clearTimeout, id);
}

function sameWorkerIdTest() {
    let parentWorkerId = CoroutineExtras.getWorkerId();
    setTimeout(() => { arktest.assertTrue(CoroutineExtras.getWorkerId() == parentWorkerId) }, zeroTimeout);
}

function testSetTimeoutLaunch() {
    launch<void, () => void>(setTimeoutTest);
}

function testClearTimeoutLaunch() {
    launch<void, () => void>(clearTimeoutTest);
}

function testClearIntervalFromCbLaunch() {
    launch<void, () => void>(clearIntervalTestFromCallback);
}

function testClearIntervalFromTimerLaunch() {
    launch<void, () => void>(clearIntervalFromAnotherTimer);
}

function testCearTimeoutFromWorkerLaunch() {
    launch<void, () => void>(clearTimeoutFromAnotherWorkerTest);
}

function testSameWorkerIdTestLaunch() {
    launch<void, () => void>(sameWorkerIdTest);
}

function testFirstTimerIdLaunch() {
    launch<void, () => void>(firstTimerIdTest);
}

function main(): int {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    
    let testSuite = new arktest.ArkTestsuite('coroutines.setTimeout');

    /**
     * Test on main coroutine worker
     * Note: firstTimerIdTest should be executed at the first, or it will fail
     */
    testSuite.addTest('firstTimerIdTest', firstTimerIdTest);
    testSuite.addTest('setTimeoutTest', setTimeoutTest);
    testSuite.addTest('clearTimeoutTest', clearTimeoutTest);
    testSuite.addTest('clearIntervalTestFromCallback', clearIntervalTestFromCallback);
    testSuite.addTest('clearIntervalFromAnotherTimer', clearIntervalFromAnotherTimer);
    testSuite.addTest('clearTimeoutFromAnotherWorkerTest', clearTimeoutFromAnotherWorkerTest);
    testSuite.addTest('sameWorkerIdTest', sameWorkerIdTest);
    
    /**
     * Test on non-main coroutine worker using launch
     */
    testSuite.addTest('testSetTimeoutLaunch', testSetTimeoutLaunch);
    testSuite.addTest('testClearTimeoutLaunch', testClearTimeoutLaunch);
    testSuite.addTest('testClearIntervalFromCbLaunch', testClearIntervalFromCbLaunch);
    testSuite.addTest('testClearIntervalFromTimerLaunch', testClearIntervalFromTimerLaunch);
    testSuite.addTest('testCearTimeoutFromWorkerLaunch', testCearTimeoutFromWorkerLaunch);
    testSuite.addTest('testSameWorkerIdTestLaunch', testSameWorkerIdTestLaunch);
    testSuite.addTest('testFirstTimerIdLaunch', testFirstTimerIdLaunch);

    /**
     * Note: none of these tests will actually be registered as failing by ArkTest in case if it fails. 
     * Instead, the whole program will fail with the uncaught error. This is to be fixed in future.
     * This is because all the setTimeouts are executed as separate coroutines.
     */
    let res = testSuite.run();
    
    return res;
}
