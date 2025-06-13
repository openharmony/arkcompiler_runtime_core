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

    suite.addTest("Write after finish error", testStreamWritableWriteAfterFinishError);
    suite.addTest("Write after finish error", testStreamWritableConstructorSetMethodsError);
    suite.addTest("Writable ended state", testWritableEndedState);
    suite.addTest("Writable finished state", testWritableFinishedState);
    suite.addTest("Writable finish event", testWritableFinishEvent);
    suite.addTest("Writable prefinish event", testWritablePrefinishEvent);
    suite.addTest("Writable finish sync state", testWritableFinishSyncState);

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

function mustNotCall() {
    let fn: Function = () => { }
    return mustCall(fn, 0);
}

function mustCall(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (err: Error | undefined | null): void => {
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

// Test cases ported from ArkTS 1.0:testStreamWritableWritevFinish006
function testStreamWritableWriteAfterFinishError() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            setTimeout(() => {
                callback.unsafeCall();
            });
        }
    }
    let writableStream = new MyWritable();
    writableStream.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'write after end');
    }));
    writableStream.on('finish', (): void => {
        writableStream.write('test');
    });
    writableStream.end();
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamWritableConstructorSetMethods001
function testStreamWritableConstructorSetMethodsError() {
    let writableStream = new stream.Writable();
    const test = (): void => {
        writableStream.end('test');
    }
    let exceptionCheck = (e: Error | Exception): boolean => {
        return ((e as BusinessError).message == "The doWrite() method is not implemented")
    }
    arktest.expectThrow(() => { test(); }, exceptionCheck);
}

// Test cases ported from ArkTS 1.0:testStreamWritableEndedState001
function testWritableEndedState() {
    let endCalled = false;
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            arktest.assertEQ(this.writableEnded, false);
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    arktest.assertEQ(writableStream.writable, true);
    arktest.assertEQ(writableStream.writableEnded, false);
    writableStream.end('testing ended', 'utf8', (): void => {
        arktest.assertEQ(writableStream.writable, false);
        arktest.assertEQ(writableStream.writableEnded, true);
        endCalled = true;
    });
    arktest.assertEQ(writableStream.writable, false);
    arktest.assertEQ(writableStream.writableEnded, true);
    writableStream.on('finish', (): void => {
        arktest.assertTrue(endCalled);
    });
}

// Test cases ported from ArkTS 1.0:testStreamWritableFinished002
function testWritableFinishedState() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            arktest.assertEQ(this.writableFinished, false);
            callback.unsafeCall();
        }
    }
    let writable = new MyWritable();
    writable.on('finish', mustCall((): void => {
        arktest.assertEQ(writable.writableFinished, true);
    }));
    writable.end('testing finished state', undefined, mustCall((): void => {
        arktest.assertEQ(writable.writableFinished, true);
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamWritableFinished003
function testWritableFinishEvent() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    let finishCalled = false;
    writableStream.end();
    writableStream.on('finish', (): void => {
        finishCalled = true;
    });
    writableStream.on('finish', (): void => {
        arktest.assertTrue(finishCalled);
    });
}

// Test cases ported from ArkTS 1.0:testStreamWritableFinished004
function testWritablePrefinishEvent() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    let sync = true;
    writableStream.on('prefinish', (): void => {
        arktest.assertTrue(sync);
    });
    writableStream.end();
    sync = false;
}

// Test cases ported from ArkTS 1.0:testStreamWritableFinished005
function testWritableFinishSyncState() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    let sync = true;
    writableStream.on('finish', (): void => {
        sync = false;
    });
    writableStream.on('prefinish', (): void => {
        arktest.assertTrue(sync);
    });
    writableStream.end();
    sync = false;
}

class TestWritable extends stream.Writable {
    private fn: (chunk: string | Uint8Array, type: string, enc: string) => void;
    constructor(fn: (chunk: string | Uint8Array, type: string, enc: string) => void) {
        super();
        this.fn = fn;
    }
    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        this.fn(chunk, typeof chunk, encoding);
        callback.unsafeCall();
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

class ErrorEventWritable extends stream.Writable {
    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        if (chunk as string === 'C') {
            callback.unsafeCall(new Error('error'));
        } else {
            callback.unsafeCall();
        }
    }
}
