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

async function AsyncFailing(): Promise<string> {
    throw Error("AsyncFailing");
}

function Failing(): string {
    throw Error("Failing");
}

function SingleRejectedJob(): null {
    throw Error("I'm your single rejected job");
}

function main(): void {
    StdProcess.on("unhandledJobRejection", (obj: Object): void => {
        assertTrue(false, "Job handler didn't switch");
    });
    StdProcess.on("unhandledPromiseRejection", (obj: Object): void => {
        assertTrue(false, "All promises should be handled");
    });

    StdProcess.on("unhandledJobRejection", (obj: Object): void => {
        assertTrue(obj instanceof Error, "not Erorr is not expected")

        assertEQ((obj as Error).message, "I'm your single rejected job");

        StdProcess.on("unhandledJobRejection", (obj: Object): void => {
            assertTrue(false, "All jobs should be handled");
        });
    });

    let g = launch<string, () => string>(Failing);

    try {
        launch<string, () => string>(Failing).Await();
    } catch(e) {
    }

    AsyncFailing().catch((e: NullishType) => {});

    let p = Promise.reject(new Error("promise1"));
    let q = p.then(() => {
        console.println("Test failed: unreachable code");
        let procManager = new StdProcess.ProcessManager();
        procManager.exit(1);
    });
    q.catch((e: NullishType) => {});

    let f = async (): Promise<void> => {
        try {
            await Promise.reject(new Error("promise2"));
        } catch (e) {
        }
    };

    f();

    try {
        g.Await();
    } catch (e) {
    }

    launch<null, () => null>(SingleRejectedJob);
}