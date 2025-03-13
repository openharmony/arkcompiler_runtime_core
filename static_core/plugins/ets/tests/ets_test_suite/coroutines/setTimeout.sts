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

type Test = () => void;

let tests = new Array<Test>(
    setTimeoutTest,
    clearTimeoutTest,
    clearIntervalTestFromCallback,
    clearIntervalFromAnotherTimer,
    clearTimeoutFromAnotherWorkerTest,
    sameWorkerIdTest,
);

let zeroTimeout = 0;
let shortTimeout = 10;
let mediumTimeout = 100;
let longTimeout = 1000;

function setTimeoutTest() {
    let flag = false;
    let start = Date.now();
    setTimeout(() => {
        assertTrue(flag);
        console.log(Date.now() - start);
        assertTrue(Date.now() - start >= shortTimeout);
    }, shortTimeout);
    flag = true;
}

function clearTimeoutTest() {
    let id = setTimeout(() => { assertTrue(false); }, mediumTimeout);
    clearTimeout(id);
}

function clearIntervalTestFromCallback() {
    let flag = true;
    let id = -1;
    id = setInterval(() => {
        assertTrue(flag);
        flag = false;
        assertTrue(id != -1);
        clearInterval(id);
    }, zeroTimeout);
}

function clearIntervalFromAnotherTimer() {
    let flag = false;
    let id = setInterval(() => { flag = true; }, shortTimeout);
    setTimeout(() => {
        assertTrue(flag);
        clearInterval(id);
    }, mediumTimeout);
}

function clearTimeoutFromAnotherWorkerTest() {
    let id = setTimeout(() => { assertTrue(false); }, longTimeout);
    launch clearTimeout(id);
}

function sameWorkerIdTest() {
    let parentWorkerId = CoroutineExtras.getWorkerId();
    setTimeout(() => { assertTrue(CoroutineExtras.getWorkerId() == parentWorkerId) }, zeroTimeout);
}

function main() {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    for (let test of tests) {
        launch test();
    }
}
