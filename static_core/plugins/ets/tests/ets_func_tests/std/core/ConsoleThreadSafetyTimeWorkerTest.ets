/**
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

// This scenario validates exact console output, so it uses a paired .expected file instead of arktest assertions.

async function main(): Promise<int> {
    const sharedTimeLabel = 'THREAD_SAFE_TIMER';
    const missingTimeLabel = 'THREAD_SAFE_TIMER_MISSING';
    const startFlag = new AtomicBoolean(false);
    const stage = new AtomicInt(0);

    const waitForStart = (): void => {
        while (!startFlag.load()) {
            Coroutine.Schedule();
        }
    };

    const waitForStage = (expected: int): void => {
        while (stage.load() != expected) {
            Coroutine.Schedule();
        }
    };

    const timeOwner = (): void => {
        waitForStart();
        console.time(sharedTimeLabel);
        stage.store(1);
    };

    const timeObserver = (): void => {
        waitForStart();

        waitForStage(1);
        console.time(sharedTimeLabel);
        stage.store(2);

        waitForStage(3);
        console.timeEnd(missingTimeLabel);
    };

    const owner = new EAWorker(() => {
        timeOwner();
    });
    const observer = new EAWorker(() => {
        timeObserver();
    });

    owner.start();
    observer.start();
    startFlag.store(true);

    waitForStage(2);
    stage.store(3);

    await owner.join();
    await observer.join();
    return 0;
}
