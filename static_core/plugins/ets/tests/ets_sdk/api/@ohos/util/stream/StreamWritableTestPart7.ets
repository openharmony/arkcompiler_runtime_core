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

    suite.addTest("Verify drain event after end", testWritableDrainEventAfterEnd);
    suite.addTest("Handle doWritev event", testWritableDoWritevEvent);
    suite.addTest("Check writable length after end use doWrite", testWritableLengthAfterEndUseDoWrite);
    suite.addTest("Check writable length after end use doWritev", testWritableLengthAfterEndUseDoWritev);
    suite.addTest("Check writable length after cork and uncork", testWritableLengthAfterCorkUncork);
    suite.addTest("Check writable length after doWritev", testWritableLengthAfterDoWritev);
    suite.addTest("Check writable length after async doWritev", testWritableLengthAfterAsyncDoWritev);
    suite.addTest("Verify callback after doWrite", testWritableAfterDoWriteCallback);
    suite.addTest("Verify doWrite order", testWritableDoWriteOrder);
    suite.addTest("Verify doWritev order", testWritableDoWritevOrder);
    suite.addTest("Handle write after end error", testStreamWritableWriteAfterEnd);
    suite.addTest("Handle end after cork without uncork", testStreamWritableEndAfterCork);
    suite.addTest("Handle finish and close events", testStreamWritableFinishAndClose);
    suite.addTest("Handle finish and close events asynchronously", testStreamWritableFinishAndCloseAsync);
    suite.addTest("Verify drain event with high watermark", testWritableDrainEventHighWatermark);
    suite.addTest("Verify drain event with async high watermark", testWritableDrainEventHighWatermarkAsync);
    suite.addTest("Handle drain event after end", testStreamWritableDrainEventAfterEnd);
    suite.addTest("Write uint array data to stream", testWritableWriteUintArrayData);

    return suite.run()
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

// Test cases ported from ArkTS 1.0:testStreamWritableLengthAfterEnd001
function testWritableDrainEventAfterEnd() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    writableStream.end('end test');
    arktest.assertEQ(writableStream.writableLength, 0);
}

// Test cases ported from ArkTS 1.0:testStreamWritableLengthAfterEnd002
function testWritableDoWritevEvent() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            setTimeout(() => {
                callback.unsafeCall();
            }, 0);
        }
    }
    let writableStream = new MyWritable();
    writableStream.on('finish', () => { arktest.assertEQ(writableStream.writableLength, 0); });
    writableStream.end('end test');
}

// Test cases ported from ArkTS 1.0:testStreamWritableLengthAfterEnd003
function testWritableLengthAfterEndUseDoWrite() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    writableStream.cork();
    writableStream.write('1');
    writableStream.write('2');
    writableStream.write('3');
    writableStream.uncork();
    writableStream.end();
    arktest.assertEQ(writableStream.writableLength, 0);
}

// Test cases ported from ArkTS 1.0:testStreamWritableLengthAfterEnd004
function testWritableLengthAfterEndUseDoWritev() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWritev(chunks: string[] | Uint8Array[], callback: Function) {
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    writableStream.cork();
    writableStream.write('1');
    writableStream.write('2');
    writableStream.write('3');
    writableStream.uncork();
    writableStream.end();
    arktest.assertEQ(writableStream.writableLength, 0);
}

// Test cases ported from ArkTS 1.0:testStreamWritableLengthAfterEnd005
function testWritableLengthAfterCorkUncork() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWritev(chunks: string[] | Uint8Array[], callback: Function) {
            setTimeout(() => {
                callback.unsafeCall();
            }, 0);
        }
    }
    let writableStream = new MyWritable();
    writableStream.on('finish', () => { arktest.assertEQ(writableStream.writableLength, 0); });
    writableStream.cork();
    writableStream.write('1');
    writableStream.write('2');
    writableStream.write('3');
    writableStream.uncork();
    writableStream.end();
}

// Test cases ported from ArkTS 1.0:testStreamWritableAfterDoWriteCallback001
function testWritableLengthAfterDoWritev() {
    let chunks = '';
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            callback.unsafeCall();
            arktest.assertEQ(typeof chunk, 'string');
            chunks = chunks + chunk as string;
        }
    }
    let writableStream = new MyWritable();
    writableStream.cork();
    writableStream.write('1');
    writableStream.write('2');
    writableStream.write('3');
    writableStream.uncork();
    writableStream.end();
    arktest.assertEQ(chunks, '123');
}

// Test cases ported from ArkTS 1.0:testStreamWritableDoWrite001
function testWritableLengthAfterAsyncDoWritev() {
    let num = 0;
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            num++;
            if (chunk === 'end test') {
                arktest.assertEQ(num, 4);
            }
            setTimeout(() => {
                callback.unsafeCall();
            }, 0);
        }
    }
    let writableStream = new MyWritable();
    writableStream.cork();
    writableStream.write('1');
    writableStream.write('2');
    writableStream.write('3');
    writableStream.uncork();
    writableStream.end('end test');
}

// Test cases ported from ArkTS 1.0:testStreamWritableDoWrite002
function testWritableAfterDoWriteCallback() {
    let chunksBuffer = '';
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWritev(chunks: string[] | Uint8Array[], callback: Function) {
            (chunks as string[]).forEach((chunk: string) => {
                chunksBuffer = chunksBuffer + chunk.toString();
            });
            setTimeout(() => {
                callback.unsafeCall();
            }, 0);
        }
    }
    let writableStream = new MyWritable();
    writableStream.on('finish', () => { arktest.assertEQ(chunksBuffer, '12345'); });
    writableStream.cork();
    writableStream.write('1');
    writableStream.write('2');
    writableStream.write('3');
    writableStream.uncork();
    writableStream.write('4');
    writableStream.write('5');
    writableStream.end();
}

// Test cases ported from ArkTS 1.0:testStreamWritableEnd001
function testWritableDoWriteOrder() {
    let ticked = false;
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            setTimeout(() => {
                callback.unsafeCall();
            }, 0);
        }
    }
    let writableStream = new MyWritable();
    writableStream.write('pretest');
    writableStream.end();
    let func1: Function = (error: Error) => {
        arktest.assertEQ(ticked, true);
        arktest.assertEQ(error.message, 'write after end');
    }
    writableStream.write('test', 'utf8', mustCall(func1));
    ticked = true;
    let func2: Function = (error: Error) => {
        arktest.assertEQ(error.message, 'write after end');
    }
    writableStream.on('error', mustCall(func2));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamWritableEnd002
function testWritableDoWritevOrder() {
    let ticked = false;
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            if (chunk === 'end test') {
                ticked = true;
            }
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    writableStream.cork();
    writableStream.write('test');
    writableStream.end('end test');
    arktest.assertEQ(ticked, true);
}

// Test cases ported from ArkTS 1.0:testStreamWritableEnd003
function testStreamWritableWriteAfterEnd() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    let ticked = false;
    writableStream.on('finish', () => { ticked = true; });
    writableStream.on('close', () => { arktest.assertEQ(ticked, true); });
    writableStream.cork();
    writableStream.write('test');
    writableStream.end('end test');
}

// Test cases ported from ArkTS 1.0:testStreamWritableEnd004
function testStreamWritableEndAfterCork() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            setTimeout(() => {
                callback.unsafeCall();
            }, 0);
        }
    }
    let writableStream = new MyWritable();
    let ticked = false;
    writableStream.on('finish', () => { ticked = true; });
    writableStream.on('close', () => { arktest.assertEQ(ticked, true); });
    writableStream.write('test');
    writableStream.end('end test');
}

// Test cases ported from ArkTS 1.0:testStreamWritableCallback001
function testStreamWritableFinishAndClose() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWritev(chunks: string[] | Uint8Array[], callback: Function) {
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    let tChunk = '';
    let func1: Function = () => { tChunk = tChunk + '1'; }
    let func2: Function = () => { tChunk = tChunk + '2'; }
    let func3: Function = () => { tChunk = tChunk + '3'; }
    let func4: Function = () => { tChunk = tChunk + '4'; }
    let func5: Function = () => { tChunk = tChunk + '5'; }
    writableStream.on('finish', () => { arktest.assertEQ(tChunk, '12345'); });
    writableStream.cork();
    writableStream.write('test1', 'utf8', func1);
    writableStream.write('test2', 'utf8', func2);
    writableStream.write('test3', 'utf8', func3);
    writableStream.write('test4', 'utf8', func4);
    writableStream.write('test5', 'utf8', func5);
    writableStream.end();
}

// Test cases ported from ArkTS 1.0:testStreamWritableDefencoding001
function testStreamWritableFinishAndCloseAsync() {
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            if (chunk === 'test1') {
                arktest.assertEQ(encoding, 'gbk');
            }
            if (chunk === 'test2') {
                arktest.assertEQ(encoding, 'gb2312');
            }
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    writableStream.setDefaultEncoding('gb2312');
    writableStream.write('test1', 'gbk');
    writableStream.write('test2');
}

// Test cases ported from ArkTS 1.0:testStreamWritableDrainEvent001
function testWritableDrainEventHighWatermark() {
    let needDrain = false;
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWritev(chunks: string[] | Uint8Array[], callback: Function) {
            if ((this.writableLength as number) >= (this.writableHighWatermark as number)) {
                needDrain = true;
            }
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    writableStream.on('drain', () => { arktest.assertEQ(needDrain, true); })
    writableStream.cork();
    let ret = writableStream.write('test');
    while (ret) {
        ret = writableStream.write('test2kshngnseio  oioo i哦啊贵妃红 O哦i奥i鞥 aiofjoioj oa jioa ajg io id noa id ofean io');
    }
    writableStream.uncork();
}

// Test cases ported from ArkTS 1.0:testStreamWritableDrainEvent002
function testWritableDrainEventHighWatermarkAsync() {
    let needDrain = false;
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWritev(chunks: string[] | Uint8Array[], callback: Function) {
            if ((this.writableLength as number) >= (this.writableHighWatermark as number)) {
                needDrain = true;
            }
            setTimeout(() => {
                callback.unsafeCall();
            }, 0);
        }
    }
    let writableStream = new MyWritable();
    writableStream.on('drain', () => { arktest.assertEQ(needDrain, true); })
    let ret = writableStream.write('test');
    while (ret) {
        ret = writableStream.write('test2kshngnseio  oioo i哦啊贵妃红 O哦i奥i鞥 aiofjoioj oa jioa ajg io id noa id ofean io');
    }
}

// Test cases ported from ArkTS 1.0:testStreamWritableDrainEvent003
function testStreamWritableDrainEventAfterEnd() {
    let needDrain = true;
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWritev(chunks: string[] | Uint8Array[], callback: Function) {
            setTimeout(() => {
                callback.unsafeCall();
            }, 0);
        }
    }
    let writableStream = new MyWritable();
    writableStream.on('drain', () => { needDrain = false })
    writableStream.on('finish', () => { arktest.assertEQ(needDrain, true); })
    let ret = writableStream.write('test');
    while (ret) {
        ret = writableStream.write('test2kshngnseio  oioo i哦啊贵妃红 O哦i奥i鞥 aiofjoioj oa jioa ajg io id noa id ofean io');
    }
    writableStream.end();
}

function testWritableWriteUintArrayData() {
    let param: Object[] = [];
    class MyWritable extends stream.Writable {
        constructor() {
            super();
        }
        doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
            arktest.assertEQ((chunk as Uint8Array).toString(), '11,22,33,44,55,66');
            callback.unsafeCall();
        }
    }
    let writableStream = new MyWritable();
    let uint8Array = new Uint8Array([11, 22, 33, 44, 55, 66]);
    writableStream.write(uint8Array);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}
