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

class MyRead1 extends TestReadable {
    constructor() {
        super();
    }
    doRead(size: number) {
        this.push('Some data');
        this.push(null);
    }
}
class MyWrite1 extends CustomWritable {
    str = "{\"0\":83,\"1\":111,\"2\":109,\"3\":101,\"4\":32,\"5\":100,\"6\":97,\"7\":116,\"8\":97}";
    constructor() {
        super();
    }
    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
            arktest.assertEQ(JSON.stringify(chunk as Uint8Array), this.str);
            callback.unsafeCall();
        }
        let wrappedFn = mustCallFnWrite(fn);
        wrappedFn(chunk, encoding, callback);
    }
}

// Test cases ported from ArkTS 1.0:testStreamPipe004
function testPipeReadableAndWritableStreams() {
    let readable = new MyRead1();
    let writable = new MyWrite1();
    readable.pipe(writable);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyRead2 extends stream.Readable {
    sumData = 0;
    constructor() {
        super();
    }
    doRead(size: number) {
        const onceData = 8 * 1024;
        if ((this.sumData + onceData) < 41000) {
            this.sumData += onceData;
            this.push('a'.repeat(onceData));
        } else if (this.sumData === 41000) {
            this.push(null);
        } else {
            this.push('a'.repeat(41000 - this.sumData));
            this.sumData += (41000 - this.sumData);
        }
    }
}
class MyWrite2 extends stream.Writable {
    sum = 0;
    callCount = 0;
    constructor() {
        super();
    }
    doWrite(chunk: string, encoding: string, callback: Function): void {
        let fn = (chunk: string, encoding: string, callback: Function) => {
            this.sum += chunk.length;
            this.callCount++;
            if (this.callCount === 6) {
                arktest.assertEQ(this.sum, 41000);
            }
            setTimeout(() => {
                callback.unsafeCall()
            }, 100)
        }
        let wrappedFn = mustCallFn(fn, 6);
        wrappedFn(chunk, encoding, callback);
    }
}

// Test cases ported from ArkTS 1.0:testStreamPipe005
function testPipeLargeDataTransferValidation() {
    let readable = new MyRead2();
    let writable = new MyWrite2();
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
