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

    suite.addTest("Set default encoding for duplex stream (gbk).", testDuplexStreamSetDefaultEncodingGbk)
    suite.addTest("Set default encoding for duplex stream (gb2312).", testDuplexStreamSetDefaultEncodingGb2312)
    suite.addTest("Transform stream encoding validation (gb18030).", testTransformStreamWriteEncodingGb18030)
    suite.addTest("Transform stream encoding validation (gbk).", testTransformStreamWriteEncodingGbk)

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

// Test cases ported from ArkTS 1.0:testStreamEncoding023
function testDuplexStreamSetDefaultEncodingGbk() {
    class MyDuplex extends stream.Duplex {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
                arktest.assertEQ(encoding, 'gbk');
                callback.unsafeCall();
            }
            let wrappedFn = mustCallUint8Array(fn);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let duplex = new MyDuplex();
    let result = duplex.setDefaultEncoding('gbk');
    duplex.write('你好');
    arktest.assertTrue(result);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamEncoding024
function testDuplexStreamSetDefaultEncodingGb2312() {
    class MyDuplex extends stream.Duplex {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
            let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
                arktest.assertEQ(encoding, 'gb2312');
                callback.unsafeCall();
            }
            let wrappedFn = mustCallUint8Array(fn);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let duplex = new MyDuplex();
    let result = duplex.setDefaultEncoding('gb2312');
    duplex.write('你好');
    arktest.assertTrue(result);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamEncoding025
function testTransformStreamWriteEncodingGb18030() {
    class MyTransform extends stream.Transform {
        constructor() {
            super()
        }
        doTransform(chunk: string, encoding: string, callback: Function): void {
            let fn = (chunk: string, encoding: string, callback: Function) => {
                arktest.assertEQ(encoding, 'gb18030');
                arktest.assertEQ(chunk, '你好123');
                callback.unsafeCall();
            }
            let wrappedFn = mustCallFn(fn);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let transform = new MyTransform();
    transform.write('你好123', 'gb18030');
    transform.end();
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamEncoding026
function testTransformStreamWriteEncodingGbk() {
    class MyTransform extends stream.Transform {
        constructor() {
            super()
        }
        doTransform(chunk: string, encoding: string, callback: Function): void {
            let fn = (chunk: string, encoding: string, callback: Function) => {
                arktest.assertEQ(encoding, 'gbk');
                arktest.assertEQ(chunk, '你好123');
                callback.unsafeCall();
            }
            let wrappedFn = mustCallFn(fn);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let transform = new MyTransform();
    transform.write('你好123', 'gbk');
    transform.end();
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}
