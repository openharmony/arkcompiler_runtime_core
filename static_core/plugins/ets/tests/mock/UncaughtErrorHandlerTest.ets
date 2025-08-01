/**
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


class Event {
    constructor() {
        this.promise = new Promise<Number>((resolve: (val: Number) => void) => {
            this.resolveFn = resolve;
        })
    }

    public Wait() {
        await this.promise;
    }

    public Fire() {
        this.resolveFn!(0);
    }

    private promise: Promise<Number>;
    private resolveFn: ((val: Number) => void) | null = null;
}

function Failing(): int {
    throw new Error('Failed');
}

function SimpleTest(): void {
    StdProcess.on('uncaughtError', (err: Object): void => {
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(2);
    });
    Failing();
}

function SetTimeoutTest(): void {
    StdProcess.on('uncaughtError', (err: Object): void => {
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(2);
    });
    setTimeout(Failing);
}

function NoDeadlockTest(): void {
    StdProcess.on('uncaughtError', (err: Object): void => {
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(2);
    });
    StdProcess.on('unhandledPromiseRejection', (reason: Error, pr: Object): void => {
        Failing();
    })
    Promise.reject(new Error('rejected promise'));
}

function NoAbortTest(): void {
    StdProcess.on('uncaughtError', (err: Object): void => {
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(2);
    });
    try {
        Failing();
    } catch (e) {
    }
}

function SimpleConcurrentTest(): void {
    let first_event = new Event();
    let second_event = new Event();
    let procManager = new StdProcess.ProcessManager();
    StdProcess.on('uncaughtError', (err: Object): void => {
        first_event.Fire();
        second_event.Wait();
        procManager.exit(2);
    });

    const params: LaunchParams = {abortFlag: true};
    launch<void, () => void>((): void => {
        first_event.Wait();
        setTimeout(() => {
            second_event.Fire();
        }, 500);
        StdProcess.on('uncaughtError', (err: Object): void => {
            procManager.exit(1);
        });
        throw new Error('second error');
    }, params);

    throw new Error('first error');
}

export function main(args: FixedArray<string>): int {
    let type: string = args[0];
    if (type == 'Simple') {
        SimpleTest();
    } else if (type == 'SetTimeout') {
        SetTimeoutTest();
    } else if (type == 'NoDeadlock') {
        NoDeadlockTest();
    } else if (type == 'NoAbort') {
        NoAbortTest();
    } else if (type == 'SimpleConcurrent') {
        SimpleConcurrentTest();
    } else {
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(1);
    }
    return 0;
}
