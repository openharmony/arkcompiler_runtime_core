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

    const worker0 = (): void => {
        waitForStart();

        console.println('PRINTLN|worker=00|step=0|payload=W00-I000-W00-I000-W00-I000-W00-I000-');
        stage.store(1);

        waitForStage(2);
        console.println('PRINTLN|worker=00|step=1|payload=W00-I001-W00-I001-W00-I001-W00-I001-');
        stage.store(3);
    };

    const worker1 = (): void => {
        waitForStart();

        waitForStage(1);
        console.println('PRINTLN|worker=01|step=0|payload=W01-I000-W01-I000-W01-I000-W01-I000-');
        stage.store(2);

        waitForStage(3);
        console.println('PRINTLN|worker=01|step=1|payload=W01-I001-W01-I001-W01-I001-W01-I001-');
    };

    const first = new EAWorker(() => {
        worker0();
    });
    const second = new EAWorker(() => {
        worker1();
    });

    first.start();
    second.start();
    startFlag.store(true);

    await first.join();
    await second.join();
    return 0;
}
