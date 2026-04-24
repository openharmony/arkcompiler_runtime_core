/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

function Failing(): int {
    throw new Error('Failed');
}

function SetTimeoutTest(): void {
    setTimeout(Failing);
}

function AbortTest(): void {
    const params: LaunchParams = {abortFlag: true,  workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.MAIN)};
    launch(params, Failing);
    Coroutine.Schedule();
}

function NoAbortTest(): void {
    const params: LaunchParams = {abortFlag: false,  workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.MAIN)};
    let finished = new CompletableJob<void>();
    launch(params, Failing).catch((_e: Error) => {
        finished.finish();
    });
    Coroutine.Schedule();
    finished.Await();
}

export function main(args: FixedArray<string>): int {
    let type: string = args[0];
    if (type == 'SetTimeout') {
        SetTimeoutTest();
    } else if (type == 'Abort') {
        AbortTest();
    } else if (type == 'NoAbort') {
        NoAbortTest();
    } else {
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(2);
    }
    return 0;
}
