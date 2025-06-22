/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License')
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { BusinessError } from "@ohos.base";
import { stream } from "@ohos.util.stream";

const mustChecks: Array<Function> = new Array<Function>();
function main(): int {
    const suite = new arktest.ArkTestsuite("Stream API tests")

    suite.addTest("Pipe with readable and writable streams.", testPipeReadableAndWritableStreams)
    suite.addTest("Pipe large data transfer validation.", testPipeLargeDataTransferValidation)

    return suite.run();
}

async function mustCallCheckAll() {
    return new Promise<void>((resolve, reject) => {
        setTimeout(() => {
            try {
                mustChecks.forEach(check => {
                    check.unsafeCall();
                });
                mustChecks.length = 0;
                resolve(undefined);
            } catch (e: Error) {
                reject(e)
            }
        }, 2000)
    })
}

function mustCallFn(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn = (chunk: string, encoding: string, callback: Function) => {
        callCount++;
        fn?.unsafeCall(chunk, encoding, callback);
    }
    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times. `);
        }
    });
    return wrappedFn;
}

function mustCallFnWrite(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
        callCount++;
        fn?.unsafeCall(chunk, encoding, callback);
    }
    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times. `);
        }
    });
    return wrappedFn;
}

function mustCall(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (err: Error | undefined | null) => {
        callCount++;
        fn?.unsafeCall(err);
    }

    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times. `);
        }
    });
    return wrappedFn;
}

// Test cases ported from ArkTS 1.0:testStreamPipe004
function testPipeReadableAndWritableStreams() {
    const str = "83,111,109,101,32,100,97,116,97";
    class MyRead extends TestReadable {
        constructor() {
            super();
        }
        doRead(size: number) {
            this.push('Some data');
            this.push(null);
        }
    }
    class MyWrite extends CustomWritable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
                arktest.assertEQ(JSON.stringify(chunk), str);
                callback.unsafeCall();
            }
            let wrappedFn = mustCallFnWrite(fn);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let readable = new MyRead();
    let writable = new MyWrite();
    readable.pipe(writable);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamPipe005
function testPipeLargeDataTransferValidation() {
    let sumData = 0;
    let sum = 0;
    let callCount = 0;
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
            const onceData = 8 * 1024;
            if ((sumData + onceData) < 41000) {
                sumData += onceData;
                this.push('a'.repeat(onceData));
            } else if (sumData === 41000) {
                this.push(null);
            } else {
                this.push('a'.repeat(41000 - sumData));
                sumData += (41000 - sumData);
            }
        }
    }
    class MyWrite extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string, encoding: string, callback: Function): void {
            let fn = (chunk: string, encoding: string, callback: Function) => {
                sum += chunk.length;
                callCount++;
                if (callCount === 6) {
                    arktest.assertEQ(sum, 41000);
                }
                setTimeout(() => {
                    callback.unsafeCall()
                }, 100)
            }
            let wrappedFn = mustCallFn(fn, 6);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let readable = new MyRead();
    let writable = new MyWrite();
    readable.pipe(writable);
    readable.unpipe(writable);
    readable.pipe(writable);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class TestReadable extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: number) {
    }

}

class CustomWritable extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        setTimeout(() => {
            callback.unsafeCall();
        }, 0);
    }

    async waitForFinish(): Promise<void> {
        return new Promise<void>((resolve) => {
            this.on('finish', (): void => {
                resolve(undefined);
            });
        });
    }

    async waitForDrain(): Promise<void> {
        return new Promise<void>((resolve) => {
            this.on('drain', (): void => {
                resolve(undefined);
            });
        });
    }
}
