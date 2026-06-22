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

    const ownerTask = (): void => {
        waitForStart();

        console.group();
        console.log('GROUP|worker=00|phase=grouped_before_other_log');
        stage.store(1);

        waitForStage(2);
        console.log('GROUP|worker=00|phase=grouped_after_other_log');
        stage.store(3);

        waitForStage(4);
        console.log('GROUP|worker=00|phase=grouped_after_foreign_group_end');
        console.groupEnd();
        console.log('GROUP|worker=00|phase=plain_after_own_group_end');
    };

    const observerTask = (): void => {
        waitForStart();

        waitForStage(1);
        console.log('GROUP|worker=01|phase=plain_during_foreign_group');
        stage.store(2);

        waitForStage(3);
        console.groupEnd();
        console.log('GROUP|worker=01|phase=plain_after_foreign_group_end');
        stage.store(4);
    };

    const owner = new EAWorker(() => {
        ownerTask();
    });
    const observer = new EAWorker(() => {
        observerTask();
    });

    owner.start();
    observer.start();
    startFlag.store(true);

    await owner.join();
    await observer.join();
    return 0;
}
