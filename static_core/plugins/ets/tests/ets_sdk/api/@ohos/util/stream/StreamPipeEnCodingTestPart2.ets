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

    suite.addTest("Writable stream encoding validation (gb18030).", testWritableStreamWriteEncodingGb18030)
    suite.addTest("Writable stream encoding validation (gbk).", testWritableStreamWriteEncodingGbk)
    suite.addTest("Writable stream encoding validation (gb2312).", testWritableStreamWriteEncodingGb2312)
    suite.addTest("Writable stream end method validation (gb18030).", testWritableStreamEndEncodingGb18030)

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

function mustCallUint8Array(fn?: Function, times: number = 1) {
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

// Test cases ported from ArkTS 1.0:testStreamEncoding001
function testWritableStreamWriteEncodingGb18030() {
    class MyWrite extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
                arktest.assertEQ(chunk, '测试');
                arktest.assertEQ(encoding, 'gb18030');
                callback.unsafeCall();
            }
            let wrappedFn = mustCallUint8Array(fn);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let writable = new MyWrite();
    let result = writable.write('测试', 'gb18030');
    arktest.assertTrue(result);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamEncoding002
function testWritableStreamWriteEncodingGbk() {
    class MyWrite extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
                arktest.assertEQ(chunk, '你好');
                arktest.assertEQ(encoding, 'gbk');
                callback.unsafeCall();
            }
            let wrappedFn = mustCallUint8Array(fn);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let writable = new MyWrite();
    let result = writable.write('你好', 'gbk');
    arktest.assertTrue(result);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamEncoding003
function testWritableStreamWriteEncodingGb2312() {
    class MyWrite extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
                arktest.assertEQ(chunk, '你好');
                arktest.assertEQ(encoding, 'gb2312');
                callback.unsafeCall();
            }
            let wrappedFn = mustCallUint8Array(fn);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let writable = new MyWrite();
    let result = writable.write('你好', 'gb2312');
    arktest.assertTrue(result);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamEncoding004
function testWritableStreamEndEncodingGb18030() {
    class MyWrite extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
                arktest.assertEQ(chunk, '测试');
                arktest.assertEQ(encoding, 'gb18030');
                callback.unsafeCall();
            }
            let wrappedFn = mustCallUint8Array(fn);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let writable = new MyWrite();
    writable.end('测试', 'gb18030', mustCall());
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}
