/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

function ReturnAbc(): string {
    return "abc";
}

function TaskpoolTestIsCancel() {
    assert(taskpoolTask.isCancel() == false);
    let t = new taskpoolTask(ReturnAbc);
    taskpool.execute(t);
    assert(taskpoolTask.isCancel() == false);
}

function TaskpoolTestCancelNonStartedTask() {
    let t = new taskpoolTask(ReturnAbc);
    let isErrorOccurred = false;
    try {
        taskpool.cancel(t);
        taskpool.execute(t);
    } catch(e: Error) {
        isErrorOccurred = true;
        assert(e.toString() == "Error: taskpool:: task is not executed or has been executed");
    }
    assert(isErrorOccurred);
}

function TaskpoolTestCancelExecutedTask() {
    let t = new taskpoolTask(ReturnAbc);
    let isErrorOccurred = false;
    try {
        await taskpool.execute(t);
        taskpool.cancel(t);
    } catch(e: Error) {
        isErrorOccurred = true;
        assert(e.toString() == "Error: taskpool:: task is not executed or has been executed");
    }
    assert(isErrorOccurred);
}

function TaskpoolTestCancelExecutingTask() {
    let t = new taskpoolTask(ReturnAbc);
    let p = taskpool.execute(t);
    try {
        taskpool.cancel(t);
    } catch(e: Error) {
        assert(e.toString() == "Error: taskpool:: task is not executed or has been executed");
        return;
    }
    let isErrorOccurred = false;
    try {
        let res = await p;
    } catch(e: Error) {
        assert(e.toString() == "Error: taskpool:: task has been canceled");
        isErrorOccurred = true;
    }
    assert(isErrorOccurred);
}

function main(): int {
    TaskpoolTestIsCancel();
    TaskpoolTestCancelNonStartedTask();
    TaskpoolTestCancelExecutedTask();
    TaskpoolTestCancelExecutingTask();
    return 0;
}
