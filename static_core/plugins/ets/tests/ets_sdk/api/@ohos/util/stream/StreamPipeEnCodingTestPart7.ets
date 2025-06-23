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

    suite.addTest("Transform stream encoding validation (gb2312).", testTransformStreamWriteEncodingGb2312)
    suite.addTest("Readable stream flowing state validation.", testReadableStreamReadableFlowingState)
    suite.addTest("Readable event triggers validation (3 events).", testReadableStreamOnReadableEventCount3)
    suite.addTest("Readable event triggers validation (4 events).", testReadableStreamOnReadableEventCount4)
    suite.addTest("Readable event triggers validation (5 events).", testReadableStreamOnReadableEventCount5)
    suite.addTest("Pipe data validation with writable stream.", testPipeDataValidationWithWritableStream)
    suite.addTest("Pipe data validation with multiple writable streams.", testPipeDataValidationAbc)
    suite.addTest("Pipe data validation with numeric data.", testPipeDataValidationNumericData)
    suite.addTest("Pipe data validation with multiple destinations.", testPipeDataValidationMultipleDestinations)
    suite.addTest("Pipe data validation with three destinations.", testPipeDataValidationThreeDestinations)
    suite.addTest("Writable stream encoding validation for multiple encodings.",
        testWritableStreamWriteMultipleEncodings)
    suite.addTest("Writable stream end method validation for multiple encodings.",
        testWritableStreamEndMultipleEncodings)
    suite.addTest("Set default encoding for writable stream with multiple encodings.",
        testWritableStreamSetDefaultEncodingMultipleEncodings)

    return suite.run();
}

const newArr = ['ascii', 'utf-8', 'UTF-8', 'gbk', 'GBK', 'GB2312', 'gb2312',
        'GB18030', 'gb18030', 'ibm866', 'iso-8859-2', 'iso-8859-3',
        'iso-8859-4', 'iso-8859-5', 'iso-8859-6', 'iso-8859-7',
        'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13',
        'iso-8859-14', 'iso-8859-15', 'koi8-r', 'koi8-u', 'macintosh',
        'windows-874', 'windows-1250', 'windows-1251', 'windows-1252',
        'windows-1253', 'windows-1254', 'windows-1255', 'windows-1256',
        'windows-1257', 'windows-1258', 'big5', 'euc-jp', 'iso-2022-jp',
        'shift_jis', 'euc-kr', 'x-mac-cyrillic', 'utf-16be', 'utf-16le'];

const resArr = ['utf-8', 'utf-8', 'UTF-8', 'gbk', 'GBK', 'GB2312', 'gb2312',
        'GB18030', 'gb18030', 'ibm866', 'iso-8859-2', 'iso-8859-3',
        'iso-8859-4', 'iso-8859-5', 'iso-8859-6', 'iso-8859-7',
        'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13',
        'iso-8859-14', 'iso-8859-15', 'koi8-r', 'koi8-u', 'macintosh',
        'windows-874', 'windows-1250', 'windows-1251', 'windows-1252',
        'windows-1253', 'windows-1254', 'windows-1255', 'windows-1256',
        'windows-1257', 'windows-1258', 'big5', 'euc-jp', 'iso-2022-jp',
        'shift_jis', 'euc-kr', 'x-mac-cyrillic', 'utf-16be', 'utf-16le'];

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

// Test cases ported from ArkTS 1.0:testStreamEncoding027
function testTransformStreamWriteEncodingGb2312() {
    class MyTransform extends stream.Transform {
        constructor() {
            super()
        }
        doTransform(chunk: string, encoding: string, callback: Function): void {
            let fn = (chunk: string, encoding: string, callback: Function) => {
                arktest.assertEQ(encoding, 'gb2312');
                arktest.assertEQ(chunk, '你好123');
                callback.unsafeCall();
            }
            let wrappedFn = mustCallFn(fn);
            wrappedFn(chunk, encoding, callback);
        }
    }
    let transform = new MyTransform();
    transform.write('你好123', 'gb2312');
    transform.end();
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamEncoding031
function testReadableStreamReadableFlowingState() {
    class MyReadable extends stream.Readable {
        constructor() {
            super();
        }

        doRead(size: number) {

        }
    }
    let readable = new MyReadable()
    arktest.assertEQ(readable.readableFlowing, null);
}

// Test cases ported from ArkTS 1.0:testStreamEncoding032
function testReadableStreamOnReadableEventCount3() {
    class MyReadable extends stream.Readable {
        constructor() {
            super();
        }

        doRead(size: number) {

        }
    }
    let readable = new MyReadable()
    let readableTest = 0;
    readable.on('readable', () => {
        readableTest++;
    });
    readable.push('test');
    readable.push('test');
    readable.push('test');
    setTimeout(() => {
        arktest.assertEQ(readableTest, 1);
    })
}

// Test cases ported from ArkTS 1.0:testStreamEncoding033
function testReadableStreamOnReadableEventCount4() {
    class MyReadable extends stream.Readable {
        constructor() {
            super();
        }

        doRead(size: number) {

        }
    }
    let readable = new MyReadable()
    let readableTest = 0;
    readable.on('readable', () => {
        readableTest++;
    });
    readable.push('test');
    readable.push('test');
    readable.push('test');
    readable.push('test');
    setTimeout(() => {
        arktest.assertEQ(readableTest, 1);
    })
}

// Test cases ported from ArkTS 1.0:testStreamEncoding034
function testReadableStreamOnReadableEventCount5() {
    class MyReadable extends stream.Readable {
        constructor() {
            super();
        }

        doRead(size: number) {

        }
    }
    let readable = new MyReadable()
    let readableTest = 0;
    readable.on('readable', () => {
        readableTest++;
    });
    readable.push('test');
    readable.push('test');
    readable.push('test');
    readable.push('test');
    readable.push('test');
    setTimeout(() => {
        arktest.assertEQ(readableTest, 1);
    })
}

// Test cases ported from ArkTS 1.0:testStreamEncoding035
function testPipeDataValidationWithWritableStream() {
    const str = "{\"0\":116,\"1\":101,\"2\":115,\"3\":116}";
    class MyReadable extends stream.Readable {
        constructor() {
            super();
        }

        doRead(size: number) {
            this.push('test');
            this.push(null)
        }
    }
    class MyWritable extends stream.Writable {
        data = ''
        constructor() {
            super()
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            this.data = (this.data) + JSON.stringify(chunk as Uint8Array);
            callback.unsafeCall()
        }
    }

    let src = new MyReadable();
    let dest = new MyWritable();
    src.pipe(dest);
    setTimeout(() => {
        arktest.assertEQ(dest.data, str);
    })
}

// Test cases ported from ArkTS 1.0:testStreamEncoding036
function testPipeDataValidationAbc() {
    const str = "{\"0\":97,\"1\":98,\"2\":99}";
    class MyReadable extends stream.Readable {
        constructor() {
            super();
        }

        doRead(size: number) {
            this.push('abc');
            this.push(null)
        }
    }
    class MyWritable extends stream.Writable {
        data = ''
        constructor() {
            super()
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            this.data = (this.data) + JSON.stringify(chunk as Uint8Array);
            callback.unsafeCall()
        }
    }

    let src = new MyReadable();
    let dest = new MyWritable();
    src.pipe(dest);
    setTimeout(() => {
        arktest.assertEQ(dest.data, str);
    })
}

// Test cases ported from ArkTS 1.0:testStreamEncoding037
function testPipeDataValidationNumericData() {
    const str = "{\"0\":49,\"1\":50,\"2\":51}";
    class MyReadable extends stream.Readable {
        constructor() {
            super();
        }

        doRead(size: number) {
            this.push('123');
            this.push(null)
        }
    }
    class MyWritable extends stream.Writable {
        data = ''
        constructor() {
            super()
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            this.data = (this.data) + JSON.stringify(chunk as Uint8Array);
            callback.unsafeCall()
        }
    }

    let src = new MyReadable();
    let dest = new MyWritable();
    src.pipe(dest);
    setTimeout(() => {
        arktest.assertEQ(dest.data, str);
    })
}

// Test cases ported from ArkTS 1.0:testStreamEncoding038
function testPipeDataValidationMultipleDestinations() {
    class MyReadable extends stream.Readable {
        constructor() {
            super();
        }

        doRead(size: number) {
            this.push('123');
            this.push(null)
        }
    }
    class MyWritable extends stream.Writable {
        count: number
        constructor() {
            super()
            this.count = 0
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            this.count++
            callback.unsafeCall()
        }
    }

    let src = new MyReadable();
    let dest1 = new MyWritable();
    let dest2 = new MyWritable();
    src.pipe(dest1);
    src.pipe(dest2);
    setTimeout(() => {
        arktest.assertEQ(dest1.count + dest2.count, 2);
    })
}

// Test cases ported from ArkTS 1.0:testStreamEncoding039
function testPipeDataValidationThreeDestinations() {
    class MyReadable extends stream.Readable {
        constructor() {
            super();
        }

        doRead(size: number) {
            this.push('123');
            this.push(null)
        }
    }
    class MyWritable extends stream.Writable {
        count: number
        constructor() {
            super()
            this.count = 0
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            this.count++
            callback.unsafeCall()
        }
    }

    let src = new MyReadable();
    let dest1 = new MyWritable();
    let dest2 = new MyWritable();
    let dest3 = new MyWritable();
    src.pipe(dest1);
    src.pipe(dest2);
    src.pipe(dest3);
    setTimeout(() => {
        arktest.assertEQ(dest1.count + dest2.count + dest3.count, 3);
    })
}

// Test cases ported from ArkTS 1.0:testStreamExtraEncoding001
function testWritableStreamWriteMultipleEncodings() {
    for (let i = 0; i < 43; i++) {
        class MyWrite extends stream.Writable {
            constructor() {
                super();
            }
            doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
                let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
                    arktest.assertEQ(chunk, '测试');
                    arktest.assertEQ(encoding, newArr[i]);
                    callback.unsafeCall();
                }
                let wrappedFn = mustCallUint8Array(fn);
                wrappedFn(chunk, encoding, callback);
            }
        }
        let writable = new MyWrite();
        let result = writable.write('测试', newArr[i]);
        arktest.assertEQ(result, true);
    }
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamExtraEncoding002
function testWritableStreamEndMultipleEncodings() {
    for (let i = 0; i < 43; i++) {
        class MyWrite extends stream.Writable {
            constructor() {
                super();
            }
            doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
                let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
                    arktest.assertEQ(chunk, '测试');
                    arktest.assertEQ(encoding, newArr[i]);
                    callback.unsafeCall();
                }
                let wrappedFn = mustCallUint8Array(fn);
                wrappedFn(chunk, encoding, callback);
            }
        }
        let writable = new MyWrite();
        writable.end('测试', newArr[i], mustCall());
    }
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamExtraEncoding003
function testWritableStreamSetDefaultEncodingMultipleEncodings() {
    for (let i = 0; i < 43; i++) {
        class MyWrite extends stream.Writable {
            constructor() {
                super();
            }
            doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
                let fn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
                    arktest.assertEQ(chunk, '你好');
                    arktest.assertEQ(encoding, resArr[i].toLowerCase());
                    callback.unsafeCall();
                }
                let wrappedFn = mustCallUint8Array(fn);
                wrappedFn(chunk, encoding, callback);
            }
        }
        let writable = new MyWrite();
        writable.setDefaultEncoding(newArr[i]);
        writable.write('你好');
    }
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}
