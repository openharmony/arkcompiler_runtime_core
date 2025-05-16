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

function main(): int {
    const suite = new ArkTestsuite("Stream API tests")

    suite.addTest("Create a readable stream", testStreamReadableConstructor001);
    suite.addTest("Create a readable stream", testStreamReadableConstructor002);
    suite.addTest("Test stream readable read", testStreamReadableRead001);
    suite.addTest("Test stream readable resume", testStreamReadableResume001);
    suite.addTest("Test stream readable pause", testStreamReadablePause001);
    suite.addTest("Test stream readable setEncoding", testStreamReadableSetEncoding001);
    suite.addTest("Test stream readable isPaused", testStreamReadableIsPaused001);
    suite.addTest("Test stream readable pipe", testStreamReadablePipe001);
    suite.addTest("Test stream readable unpipe", testStreamReadableUnpipe001);
    suite.addTest("Test stream readable on", testStreamReadableOn001);
    suite.addTest("Test stream readable off", testStreamReadableOff001);
    suite.addTest("Test stream readable doInitialize", testStreamReadableDoInitialize001);
    suite.addTest("Test stream readable doRead", testStreamReadableDoRead001);
    suite.addTest("Test stream readable push string", testStreamReadablePush001);
    suite.addTest("Test stream readable push uint8 array", testStreamReadablePush002);

    return suite.run()
}

// this test case will be completed future
function testStreamReadableConstructor001() {
    let readable = new stream.Readable();
    assertEQ(typeof readable, "object");
    assertNE(readable, undefined);
}

// this test case will be completed future
function testStreamReadableConstructor002() {
    let readableOptions: stream.ReadableOptions = {
        encoding: "utf-8"
    };
    let readable = new stream.Readable(readableOptions);
    assertEQ(typeof readable, "object");
    assertNE(readable, undefined);
}

// this test case will be completed future
function testStreamReadableRead001() {
    let readableStream = new stream.Readable();
    let ret = readableStream.read(1);
    assertEQ(ret, null);
}

// this test case will be completed future
function testStreamReadableResume001() {
    let readableStream = new stream.Readable();
    let ret: stream.Readable = readableStream.resume();
    assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamReadablePause001() {
    let readableStream = new stream.Readable();
    let ret: stream.Readable = readableStream.pause();
    assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamReadableSetEncoding001() {
    let readableStream = new stream.Readable();
    let ret: boolean = readableStream.setEncoding("utf8");
    assertTrue(ret);
}

// this test case will be completed future
function testStreamReadableIsPaused001() {
    let readableStream = new stream.Readable();
    let ret: boolean = readableStream.isPaused();
    assertFalse(ret);
}

// this test case will be completed future
function testStreamReadablePipe001() {
    let readableStream = new stream.Readable();
    let writable = new stream.Writable();
    let ret: stream.Writable = readableStream.pipe(writable);
    assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamReadableUnpipe001() {
    let readableStream = new stream.Readable();
    let writable = new stream.Writable();
    let ret: stream.Readable = readableStream.unpipe(writable);
    assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamReadableOn001() {
    let readableStream = new stream.Readable();
    let callback: CallbackFnType = (...params: Object[]) => {};
    readableStream.on("data", callback);
    assertTrue(true);
}

// this test case will be completed future
function testStreamReadableOff001() {
    let readableStream = new stream.Readable();
    let callback: CallbackFnType = (...params: Object[]) => {};;
    readableStream.off("data", callback);
    assertTrue(true);
}

// this test case will be completed future
function testStreamReadableDoInitialize001() {
    let readableStream = new stream.Readable();
    readableStream.doInitialize(() => {});
    assertTrue(true);
}

// this test case will be completed future
function testStreamReadableDoRead001() {
    let readableStream = new stream.Readable();
    readableStream.doRead(1);
    assertTrue(true);
}

// this test case will be completed future
function testStreamReadablePush001() {
    let readableStream = new stream.Readable();
    let ret: boolean = readableStream.push("test", "utf8");
    assertFalse(ret);
}

// this test case will be completed future
function testStreamReadablePush002() {
    let readableOptions: stream.ReadableOptions = {
        encoding: "utf-8",
        highWatermark: 20
    };
    let readableStream = new stream.Readable(readableOptions);
    let array = new Uint8Array(10);
    array[0] = 1;
    array[1] = 2;
    let ret: boolean = readableStream.push(array, "utf8");
    assertTrue(ret);
}
