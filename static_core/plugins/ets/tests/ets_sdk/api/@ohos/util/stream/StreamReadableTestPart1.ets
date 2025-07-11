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
import buffer from '@ohos.buffer';

const mustChecks: Array<Function> = new Array<Function>();

function main(): int {
    const suite = new arktest.ArkTestsuite("Stream API tests")

    suite.addTest("Readable resume and pause functionality", testReadableResumeAndPause)
    suite.addTest("Readable pause and read data", testReadablePauseAndRead)
    suite.addTest("Readable resume triggers readable event", testReadableResumeTriggersEvent)
    suite.addTest("Readable isPaused returns false initially", testReadableInitialIsPaused)
    suite.addTest("Readable isPaused returns true after pause", testReadablePauseUpdatesState)
    suite.addTest("Readable isPaused returns false after resume", testReadableResumeUpdatesState)
    suite.addTest("Readable object mode property validation", testReadableObjectModeProperty)
    suite.addTest("Readable property validation", testReadableDefaultProperty)
    suite.addTest("Readable error handling and property update", testReadableErrorHandling)
    suite.addTest("Readable high watermark property validation", testReadableHighWatermarkProperty)
    suite.addTest("Readable flowing state validation", testReadableFlowingState)
    suite.addTest("Readable flowing state after resume", testReadableResumeFlowingState)
    suite.addTest("Readable length property validation (empty)", testReadableLengthEmpty)
    suite.addTest("Readable length property validation (with data)", testReadableLengthWithData)
    suite.addTest("Readable length property validation (partial read)", testReadableLengthPartialRead)
    suite.addTest("Readable length property validation (end event)", testReadableLengthAfterEnd)
    suite.addTest("Readable length property validation (multiple pushes)", testReadableLengthMultiplePushes)
    suite.addTest("Readable ended property validation (not ended)", testReadableEndedNotFinished)
    suite.addTest("Readable property validation (default state)", testReadableDefaultState)
    suite.addTest("Readable flowing state validation (with data)", testReadableFlowingWithData)
    suite.addTest("Readable ended property validation (default state)", testReadableEndedDefaultState)
    suite.addTest("Readable ended property validation (after finish)", testReadableEndedAfterFinish)
    suite.addTest("Readable property validation (pause and resume)", testReadablePauseAndResumeState)

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

// Test cases ported from ArkTS 1.0:testStreamReadableResumePause001
function testReadableResumeAndPause() {
    let readable = new TestReadable();
    let dataEmittedAfterResume = false;
    let dataCalled = false;
    readable.on('data', mustCallNoArgs((): void => {
        if (!dataEmittedAfterResume) {
            readable.pause();
            setTimeout(() => {
                readable.resume();
                dataEmittedAfterResume = true;
                readable.push('test');
            }, 50);
        }
        dataCalled = true;
    }, 2));
    readable.push('test');
    await new Promise<void>((resolve) => {
        setTimeout(() => {
            resolve(undefined);
        }, 20);
    });
    arktest.assertTrue(dataCalled);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamReadableResumePause002
function testReadablePauseAndRead() {
    let readable = new TestReadable();
    readable.push('test');
    readable.pause();
    let dataChunk = readable.read();
    arktest.assertEQ((dataChunk as Object).toString(), "test");
}

// Test cases ported from ArkTS 1.0:testStreamReadableResumePause003
function testReadableResumeTriggersEvent() {
    let readable = new TestReadable();
    readable.pause();
    readable.on('readable', mustCall());
    setTimeout(() => {
        readable.resume();
        readable.push('test');
    }, 0);
    arktest.assertTrue(readable.readable, "readable should be true");
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamReadableResumePause004
function testReadableInitialIsPaused() {
    let readable = new TestReadable();
    arktest.assertFalse(readable.isPaused());
}

// Test cases ported from ArkTS 1.0:testStreamReadableResumePause005
function testReadablePauseUpdatesState() {
    let readable = new TestReadable();
    readable.pause();
    arktest.assertTrue(readable.isPaused());
}

// Test cases ported from ArkTS 1.0:testStreamReadableResumePause006
function testReadableResumeUpdatesState() {
    let readable = new TestReadable();
    readable.pause();
    readable.resume();
    arktest.assertFalse(readable.isPaused());
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties001
function testReadableObjectModeProperty() {
    let readable = new stream.Readable();
    arktest.assertEQ(readable.readableObjectMode, false);
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties002
function testReadableDefaultProperty() {
    let readable = new stream.Readable();
    arktest.assertTrue(readable.readable, "readable should be true");
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties003
function testReadableErrorHandling() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
            throw new Error('Simulated error');
        }
    }
    let readable = new MyRead();
    readable.push('test');
    arktest.assertTrue(readable.readable, "readable should be true");
    readable.on('error', mustCall((): void => {
        arktest.assertFalse(readable.readable, "readable should be false");
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties004
function testReadableHighWatermarkProperty() {
    let readable = new stream.Readable();
    arktest.assertEQ(readable.readableHighWatermark, 16 * 1024);
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties005
function testReadableFlowingState() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    readable.on('data', (): void => {
    });
    arktest.assertEQ(readable.readableFlowing, true);
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties006
function testReadableResumeFlowingState() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    readable.resume();
    arktest.assertEQ(readable.readableFlowing, true);
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties007
function testReadableLengthEmpty() {
    let readable = new stream.Readable();
    arktest.assertEQ(readable.readableLength, 0);
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties008
function testReadableLengthWithData() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    let testData = 'Hello world';
    readable.push(testData);
    arktest.assertEQ(readable.readableLength, testData.length);
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties009
function testReadableLengthPartialRead() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    let testData = 'Hello world!';
    readable.push(testData);
    let chunk = readable.read(testData.length / 2);
    arktest.assertTrue(chunk !== null);
    arktest.assertEQ(chunk!.length, testData.length / 2);
    arktest.assertEQ(readable.readableLength, testData.length / 2);
    let remainingChunk = readable.read(testData.length / 2);
    arktest.assertTrue(remainingChunk !== null);
    arktest.assertEQ(remainingChunk!.length, testData.length / 2);
    arktest.assertEQ(readable.readableLength, 0);
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties010
function testReadableLengthAfterEnd() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    readable.push('Some data');
    readable.push(null);
    readable.on('data', (): void => {
    });
    readable.on('end', (): void => {
        arktest.assertEQ(readable.readableLength, 0);
    });
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties011
function testReadableLengthMultiplePushes() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    let testData1 = 'First part';
    let testData2 = 'Second part';
    readable.push(testData1);
    arktest.assertEQ(readable.readableLength, testData1.length);
    readable.push(testData2);
    arktest.assertEQ(readable.readableLength, testData1.length + testData2.length);
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties012
function testReadableEndedNotFinished() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    readable.push('test data');
    readable.read();
    arktest.assertFalse(readable.readableEnded, "readableEnded should be false");
    readable.read();
    arktest.assertFalse(readable.readableEnded, "readableEnded should be false");
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties013
function testReadableDefaultState() {
    let readable = new stream.Readable();
    arktest.assertTrue(readable.readable, "readable should be true");
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties014
function testReadableFlowingWithData() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    readable.push('test data');
    arktest.assertEQ(readable.readableFlowing, null);
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties015
function testReadableEndedDefaultState() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    arktest.assertFalse(readable.readableEnded, "readableEnded should be false");
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties016
function testReadableEndedAfterFinish() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    readable.push('Some data');
    readable.push(null);
    readable.on('finished', mustCall((): void => {
        arktest.assertTrue(readable.readableEnded, "readableEnded should be true");
    }, 0));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamReadableProperties017
function testReadablePauseAndResumeState() {
    class MyRead extends stream.Readable {
        constructor() {
            super();
        }
        doRead(size: number) {
        }
    }
    let readable = new MyRead();
    readable.pause();
    arktest.assertTrue(readable.readable, "readable should be true");
    readable.resume();
    arktest.assertTrue(readable.readable, "readable should be true");
}

class TestReadable extends stream.Readable {
    constructor() {
        super();
    }
    doRead(size: number) {
    }
}
