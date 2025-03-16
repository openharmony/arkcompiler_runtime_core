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

function sumFunc(): int {
    return (() => 10)() + (() => 20)();
}

function sum(a: int, b: int): int {
    let t = Date.now();
    // mock time-consuming operation
    while ((Date.now() - t) < 500) {
        continue;
    }
    return a + b;
}

function main(): int {
    let longTaskTestsuite = new ArkTestsuite("taskpool.LongTask");
    longTaskTestsuite.addTest("ExecuteTest", () =>
    {
        let task = new taskpool.LongTask(sumFunc);
        let result = await taskpool.execute(task);
        assertEQ(result, 30);
    });
    longTaskTestsuite.addTest("DoubleExecutionTest", (): void throws =>
    {
        let task = new taskpool.LongTask(sumFunc);
        taskpool.execute(task);
        expectError(() => { taskpool.execute(task) },
                    new Error("taskpool:: The long task can only be executed once"));
    });
    longTaskTestsuite.addTest("LongTaskWithParamsTest", (): void throws =>
    {
        let task = new taskpool.LongTask(sum, 100, 200);
        let res = await taskpool.execute(task);
        assertEQ(res, 300);
    });
    return longTaskTestsuite.run();
}
