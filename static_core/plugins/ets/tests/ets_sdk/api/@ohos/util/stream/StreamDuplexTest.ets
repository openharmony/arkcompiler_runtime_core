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
    const suite = new arktest.ArkTestsuite("Stream API tests")

    suite.addTest("Create a duplex stream", testStreamDuplexConstructor001);
    suite.addTest("Test stream duplex read", testStreamDuplexRead001);
    suite.addTest("Test stream duplex resume", testStreamDuplexResume001);
    suite.addTest("Test stream duplex pause", testStreamDuplexPause001);
    suite.addTest("Test stream duplex setEncoding", testStreamDuplexSetEncoding001);
    suite.addTest("Test stream duplex isPaused", testStreamDuplexIsPaused001);
    suite.addTest("Test stream duplex pipe", testStreamDuplexPipe001);
    suite.addTest("Test stream duplex unpipe", testStreamDuplexUnpipe001);
    suite.addTest("Test stream duplex on", testStreamDuplexOn001);
    suite.addTest("Test stream duplex off", testStreamDuplexOff001);
    suite.addTest("Test stream duplex doInitialize", testStreamDuplexDoInitialize001);
    suite.addTest("Test stream duplex doRead", testStreamDuplexDoRead001);
    suite.addTest("Test stream duplex write", testStreamDuplexWrite001);
    suite.addTest("Test stream duplex end", testStreamDuplexEnd001);
    suite.addTest("Test stream duplex setDefaultEncoding", testStreamDuplexSetDefaultEncoding001);
    suite.addTest("Test stream duplex crok", testStreamDuplexCrok001);
    suite.addTest("Test stream duplex uncrok", testStreamDuplexUncrok001);
    suite.addTest("Test stream duplex doWrite", testStreamDuplexDoWrite001);
    suite.addTest("Test stream duplex doWritev", testStreamDuplexDoWritev001);

    return suite.run()
}

// this test case will be completed future
function testStreamDuplexConstructor001() {
    let duplex = new stream.Duplex();
    arktest.assertEQ(typeof duplex, "object");
    arktest.assertNE(duplex, undefined);
}

// this test case will be completed future
function testStreamDuplexRead001() {
    let duplexStream = new stream.Duplex();
    let ret = duplexStream.read(1);
    arktest.assertEQ(ret, null);
}

// this test case will be completed future
function testStreamDuplexResume001() {
    let duplexStream = new stream.Duplex();
    let ret: stream.Readable = duplexStream.resume();
    arktest.assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamDuplexPause001() {
    let duplexStream = new stream.Duplex();
    let ret: stream.Readable = duplexStream.pause();
    arktest.assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamDuplexSetEncoding001() {
    let duplexStream = new stream.Duplex();
    let ret: boolean = duplexStream.setEncoding("utf8");
    arktest.assertTrue(ret);
}

// this test case will be completed future
function testStreamDuplexIsPaused001() {
    let duplexStream = new stream.Duplex();
    let ret: boolean = duplexStream.isPaused();
    arktest.assertFalse(ret);
}

// this test case will be completed future
function testStreamDuplexPipe001() {
    let duplexStream = new stream.Duplex();
    let writable = new stream.Writable();
    let ret: stream.Writable = duplexStream.pipe(writable);
    arktest.assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamDuplexUnpipe001() {
    let duplexStream = new stream.Duplex();
    let writable = new stream.Writable();
    let ret: stream.Readable = duplexStream.unpipe(writable);
    arktest.assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamDuplexOn001() {
    let duplexStream = new stream.Duplex();
    let callback: CallbackFnType = (...params: Object[]) => {};
    duplexStream.on("data", callback);
    arktest.assertNE(duplexStream, undefined);
}

// this test case will be completed future
function testStreamDuplexOff001() {
    let duplexStream = new stream.Duplex();
    let callback: CallbackFnType = (...params: Object[]) => {};
    duplexStream.off("data", callback);
    arktest.assertNE(duplexStream, undefined);
}

// this test case will be completed future
function testStreamDuplexDoInitialize001() {
    let duplexStream = new stream.Duplex();
    duplexStream.doInitialize(() => {});
    arktest.assertNE(duplexStream, undefined);
}

// this test case will be completed future
function testStreamDuplexDoRead001() {
    class DuplexStream extends stream.Duplex {
        constructor() {
            super();
        }
        doRead(size: number) {}
    }
    let duplexStream = new DuplexStream();
    duplexStream.doRead(1);
    arktest.assertNE(duplexStream, undefined);
}

// this test case will be completed future
function testStreamDuplexWrite001() {
    let duplexStream = new stream.Duplex();
    let ret: boolean = duplexStream.write("data", "utf-8", () => {});
    arktest.assertFalse(ret);
}

// this test case will be completed future
function testStreamDuplexEnd001() {
    let duplexStream = new stream.Duplex();
    let ret: stream.Writable = duplexStream.end("data", "utf-8", () => {});
    arktest.assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamDuplexSetDefaultEncoding001() {
    let duplexStream = new stream.Duplex();
    let ret: boolean = duplexStream.setDefaultEncoding("utf-8");
    arktest.assertFalse(ret);
}

// this test case will be completed future
function testStreamDuplexCrok001() {
    let duplexStream = new stream.Duplex();
    let ret: boolean = duplexStream.cork();
    arktest.assertFalse(ret);
}

// this test case will be completed future
function testStreamDuplexUncrok001() {
    let duplexStream = new stream.Duplex();
    let ret: boolean = duplexStream.uncork();
    arktest.assertFalse(ret);
}

// this test case will be completed future
function testStreamDuplexDoWrite001() {
    let duplexStream = new stream.Duplex();
    duplexStream.doWrite("test", "utf8", () => {});
    arktest.assertNE(duplexStream, undefined);
}

// this test case will be completed future
function testStreamDuplexDoWritev001() {
    let duplexStream = new stream.Duplex();
    duplexStream.doWritev(["test"], () => {});
    arktest.assertNE(duplexStream, undefined);
}
