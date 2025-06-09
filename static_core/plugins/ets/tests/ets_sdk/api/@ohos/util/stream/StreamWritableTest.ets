/*
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

import {stream} from "@ohos.util.stream";

type CallbackFnType = (...params: Object[]) => void;

const mustChecks: Array<Function> = new Array<Function>();

function main(): int {
    const suite = new arktest.ArkTestsuite("Stream API tests")

    suite.addTest("Create a writable stream", testStreamWritableConstructor001);
    suite.addTest("Test stream writable write and doWrite", testStreamWritableWrite001);
    suite.addTest("Test stream writable end", testStreamWritableEnd001);
    suite.addTest("Test stream writable setDefaultEncoding", testStreamWritableSetDefaultEncoding001);
    suite.addTest("Test stream writable cork", testStreamWritableCork001);
    suite.addTest("Test stream writable uncork", testStreamWritableUncork001);
    suite.addTest("Test stream writable on", testStreamWritableOn001);
    suite.addTest("Test stream writable off", testStreamWritableOff001);
    suite.addTest("Test stream writable doInitialize", testStreamWritableDoInitialize001);
    suite.addTest("Test stream writable doWritev", testStreamWritableDoWritev001);
    suite.addTest("Writable drain event async high watermark", testWritableDrainEventAsyncHighWatermark);

    return suite.run()
}

// this test case will be completed future
function testStreamWritableConstructor001() {
    let writable = new stream.Writable();
    arktest.assertEQ(typeof writable, "object");
    arktest.assertNE(writable, undefined);
}

// will add more test cases future
function testStreamWritableWrite001() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            arktest.assertEQ((chunk as string).toString(), 'write content', "chunk is valid");
            callback.unsafeCall();
        }
    }

    let writableStream = new MyWritable();
    let ret: boolean = writableStream.write("write content", "utf8", () => {});
    arktest.assertTrue(ret);
}

// this test case will be completed future
function testStreamWritableEnd001() {
    let writableStream = new stream.Writable();
    let ret: stream.Writable = writableStream.end("test", "utf8", () => {});
    arktest.assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamWritableSetDefaultEncoding001() {
    let writableStream = new stream.Writable();
    let ret: boolean = writableStream.setDefaultEncoding("utf8");
    arktest.assertFalse(ret);
}

// this test case will be completed future
function testStreamWritableCork001() {
    let writableStream = new stream.Writable();
    let ret: boolean = writableStream.cork();
    arktest.assertFalse(ret);
}

// this test case will be completed future
function testStreamWritableUncork001() {
    let writableStream = new stream.Writable();
    let ret: boolean = writableStream.uncork();
    arktest.assertFalse(ret);
}

// this test case will be completed future
function testStreamWritableOn001() {
    let writableStream = new stream.Writable();
    let callback: CallbackFnType = (...params: Object[]) => {};
    writableStream.on("data", callback);
    arktest.assertTrue(true);
}

// this test case will be completed future
function testStreamWritableOff001() {
    let writableStream = new stream.Writable();
    let callback: CallbackFnType = (...params: Object[]) => {};
    writableStream.off("data", callback);
    arktest.assertTrue(true);
}

// this test case will be completed future
function testStreamWritableDoInitialize001() {
    let writableStream = new stream.Writable();
    writableStream.doInitialize(() => {});
    arktest.assertTrue(true);
}

// this test case will be completed future
function testStreamWritableDoWritev001() {
    let writableStream = new stream.Writable();
    writableStream.doWritev(["test"], () => {});
    arktest.assertTrue(true);
}

function mustCallTwoArgs(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn = (chunks: string[] | Uint8Array[], callback: Function) => {
        callCount++;
        fn?.unsafeCall(chunks, callback);
    }
    mustChecks.push(() => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
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
                console.error('testTags', '%{public}s', `error: ${e}`)
                reject(e)
            }
        }, 2000)
    })
}

function mustCallNoArgs(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (): void => {
        callCount++;
        fn?.unsafeCall();
    }

    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times. `);
        }
    });
    return wrappedFn;
}

// Test cases ported from ArkTS 1.0:testStreamWritableWritev001
function testWritableDrainEventAsyncHighWatermark() {
    let doWriteVCalled = false;
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWritev(chunks: string[] | Uint8Array[], callback: Function): void {
            let fn = (chunks: string[] | Uint8Array[], callback: Function) => {
                doWriteVCalled = true;
                callback.unsafeCall();
            }
            let wrappedFn = mustCallTwoArgs(fn);
            wrappedFn(chunks, callback);
        }
    }
    let writable = new MyWritable();
    writable.write('test', 'utf8', mustCallNoArgs((): void => {
    }))
    arktest.assertTrue(doWriteVCalled);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}
