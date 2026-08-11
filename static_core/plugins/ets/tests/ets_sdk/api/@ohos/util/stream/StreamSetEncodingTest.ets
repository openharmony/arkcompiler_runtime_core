/**
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

import { stream } from '@ohos.util.stream';
import buffer from '@ohos.buffer';

const mustChecks: Function[] = [];

function main(): int {
    const suite = new arktest.ArkTestsuite('Stream API tests');

    suite.addTest('Readable length after partial read and setEncoding', testPartialReadThenSetEncodingLength);
    suite.addTest('Readable content after partial read and setEncoding', testPartialReadThenSetEncodingContent);
    suite.addTest('Readable setEncoding after multiple pushes', testSetEncodingAfterMultiplePushes);
    suite.addTest('Readable partial read after setEncoding', testPartialReadAfterSetEncoding);
    suite.addTest('Readable setEncoding after full read', testSetEncodingAfterFullRead);
    suite.addTest('Readable setEncoding round-trip', testSetEncodingRoundTrip);
    suite.addTest('Readable push Uint8Array subarray after setEncoding', testPushUint8ArrayAfterSetEncoding);
    suite.addTest('Readable partial read cross strings after setEncoding', testPartialReadCrossStringsAfterSetEncoding);
    suite.addAsyncTest('Readable pipe with subarray buffer', testPipeWithSubarrayBuffer);
    suite.addAsyncTest('Readable pipe after setEncoding', testPipeAfterSetEncoding);

    return suite.run();
}

async function mustCallCheckAll() {
    return new Promise<void>((resolve, reject) => {
        setTimeout(() => {
            try {
                mustChecks.forEach(check => { check.unsafeCall(); });
                mustChecks.length = 0;
                resolve(undefined);
            } catch (e) {
                mustChecks.length = 0;
                reject(e);
            }
        }, 2000);
    });
}

function mustCallNoArgs(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (): void => {
        callCount++;
        fn?.unsafeCall();
    }
    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
}

class TestReadable extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}

class CollectWritable extends stream.Writable {
    public collected: string = '';

    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        if (chunk instanceof string) {
            this.collected += chunk;
        } else {
            this.collected += buffer.from(chunk as Uint8Array).toString();
        }
        setTimeout(() => { callback.unsafeCall(); }, 0);
    }
}

function testPartialReadThenSetEncodingLength() {
    let readable = new TestReadable();
    let testData = 'Hello world!';
    readable.push(testData);
    let halfLen: int = testData.length / 2;
    let chunk = readable.read(halfLen);
    arktest.assertTrue(chunk !== null);
    if (chunk !== null) {
        arktest.assertEQ(chunk.toString(), 'Hello ');
    }
    arktest.assertEQ(readable.readableLength, halfLen);

    readable.setEncoding('utf8');
    arktest.assertEQ(readable.readableLength, halfLen);
}

function testPartialReadThenSetEncodingContent() {
    let readable = new TestReadable();
    let testData = 'Hello world!';
    readable.push(testData);
    let halfLen: int = testData.length / 2;
    readable.read(halfLen);

    readable.setEncoding('utf8');
    let remaining = readable.read();
    arktest.assertTrue(remaining !== null);
    arktest.assertEQ(remaining as string, 'world!');
    arktest.assertEQ(readable.readableLength, 0);
}

function testSetEncodingAfterMultiplePushes() {
    let readable = new TestReadable();
    readable.push('AAAA');
    readable.push('BBBB');
    readable.push('CCCC');

    let chunk = readable.read(2);
    arktest.assertTrue(chunk !== null);
    if (chunk !== null) {
        arktest.assertEQ(chunk.toString(), 'AA');
    }
    arktest.assertEQ(readable.readableLength, 10);

    readable.setEncoding('utf8');
    arktest.assertEQ(readable.readableLength, 10);

    let remaining = readable.read();
    arktest.assertTrue(remaining !== null);
    arktest.assertEQ(remaining as string, 'AABBBBCCCC');
    arktest.assertEQ(readable.readableLength, 0);
}

function testPartialReadAfterSetEncoding() {
    let readable = new TestReadable();
    readable.setEncoding('utf8');
    readable.push('Hello world!');
    arktest.assertEQ(readable.readableLength, 12);

    let chunk = readable.read(6);
    arktest.assertTrue(chunk !== null);
    arktest.assertEQ(chunk as string, 'Hello ');
    arktest.assertEQ(readable.readableLength, 6);

    let remaining = readable.read();
    arktest.assertTrue(remaining !== null);
    arktest.assertEQ(remaining as string, 'world!');
    arktest.assertEQ(readable.readableLength, 0);
}

function testSetEncodingAfterFullRead() {
    let readable = new TestReadable();
    readable.push('Hello world!');
    let chunk = readable.read(12);
    arktest.assertTrue(chunk !== null);
    arktest.assertEQ(readable.readableLength, 0);

    readable.setEncoding('utf8');
    arktest.assertEQ(readable.readableLength, 0);

    let nothing = readable.read();
    arktest.assertTrue(nothing === null);
}

function testSetEncodingRoundTrip() {
    let readable = new TestReadable();
    let testData = 'Hello world!';
    readable.push(testData);

    let halfLen: int = testData.length / 2;
    let firstChunk = readable.read(halfLen);
    arktest.assertTrue(firstChunk !== null);
    if (firstChunk !== null) {
        arktest.assertEQ(firstChunk.toString(), 'Hello ');
    }

    readable.setEncoding('utf8');
    arktest.assertEQ(readable.readableLength, halfLen);

    let secondChunk = readable.read();
    arktest.assertTrue(secondChunk !== null);
    arktest.assertEQ(secondChunk as string, 'world!');
    arktest.assertEQ(readable.readableLength, 0);
}

function testPushUint8ArrayAfterSetEncoding() {
    let readable = new TestReadable();
    readable.setEncoding('utf8');
    let full = new Uint8Array([88, 89, 72, 105, 90, 90]);
    let sub = full.subarray(2, 4);
    readable.push(sub);
    arktest.assertEQ(readable.readableLength, 2);
    let data = readable.read();
    arktest.assertTrue(data !== null);
    arktest.assertEQ(data as string, 'Hi');
    arktest.assertEQ(readable.readableLength, 0);
}

function testPartialReadCrossStringsAfterSetEncoding() {
    let readable = new TestReadable();
    readable.setEncoding('utf8');
    readable.push('AB');
    readable.push('CD');
    arktest.assertEQ(readable.readableLength, 4);
    let chunk = readable.read(3);
    arktest.assertTrue(chunk !== null);
    arktest.assertEQ(chunk as string, 'ABC');
    arktest.assertEQ(readable.readableLength, 1);
    let remaining = readable.read();
    arktest.assertTrue(remaining !== null);
    arktest.assertEQ(remaining as string, 'D');
    arktest.assertEQ(readable.readableLength, 0);
}

async function testPipeWithSubarrayBuffer() {
    let readable = new TestReadable();
    readable.push('Hello world!');
    readable.read(6);
    readable.push(null);
    let writable = new CollectWritable();
    writable.on('finish', mustCallNoArgs((): void => {
        arktest.assertEQ(writable.collected, 'world!');
    }));
    readable.pipe(writable);
    arktest.expectNoThrow(() => { waitForCompletion(async () => { await mustCallCheckAll(); }) });
}

async function testPipeAfterSetEncoding() {
    let readable = new TestReadable();
    readable.setEncoding('utf8');
    readable.push('Hello world!');
    readable.push(null);
    let writable = new CollectWritable();
    writable.on('finish', mustCallNoArgs((): void => {
        arktest.assertEQ(writable.collected, 'Hello world!');
    }));
    readable.pipe(writable);
    arktest.expectNoThrow(() => { waitForCompletion(async () => { await mustCallCheckAll(); }) });
}
