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

    suite.addTest("Create a transform stream", testStreamTransformConstructor001);
    suite.addTest("Test stream transform read", testStreamTransformRead001);
    suite.addTest("Test stream transform resume", testStreamTransformResume001);
    suite.addTest("Test stream transform pause", testStreamTransformPause001);
    suite.addTest("Test stream transform setEncoding", testStreamTransformSetEncoding001);
    suite.addTest("Test stream transform isPaused", testStreamTransformIsPaused001);
    suite.addTest("Test stream transform pipe", testStreamTransformPipe001);
    suite.addTest("Test stream transform unpipe", testStreamTransformUnpipe001);
    suite.addTest("Test stream transform on", testStreamTransformOn001);
    suite.addTest("Test stream transform off", testStreamTransformOff001);
    suite.addTest("Test stream transform doInitialize", testStreamTransformDoInitialize001);
    suite.addTest("Test stream transform doRead", testStreamTransformDoRead001);
    suite.addTest("Test stream transform write", testStreamTransformWrite001);
    suite.addTest("Test stream transform end", testStreamTransformEnd001);
    suite.addTest("Test stream transform setDefaultEncoding", testStreamTransformSetDefaultEncoding001);
    suite.addTest("Test stream transform crok", testStreamTransformCrok001);
    suite.addTest("Test stream transform uncrok", testStreamTransformUncrok001);
    suite.addTest("Test stream transform doWrite", testStreamTransformDoWrite001);
    suite.addTest("Test stream transform doWritev", testStreamTransformDoWritev001);
    suite.addTest("Test stream transform doTransform", testStreamTransformDoTransform001);
    suite.addTest("Test stream transform doFlush", testStreamTransformDoFlush001);

    return suite.run()
}

// this test case will be completed future
function testStreamTransformConstructor001() {
    let transform = new stream.Transform();
    assertEQ(typeof transform, "object");
    assertNE(transform, undefined);
}

// this test case will be completed future
function testStreamTransformRead001() {
    let transformStream = new stream.Transform();
    let ret = transformStream.read(1);
    assertEQ(ret, null);
}

// this test case will be completed future
function testStreamTransformResume001() {
    let transformStream = new stream.Transform();
    let ret: stream.Readable = transformStream.resume();
    assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamTransformPause001() {
    let transformStream = new stream.Transform();
    let ret: stream.Readable = transformStream.pause();
    assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamTransformSetEncoding001() {
    let transformStream = new stream.Transform();
    let ret: boolean = transformStream.setEncoding("utf8");
    assertTrue(ret);
}

// this test case will be completed future
function testStreamTransformIsPaused001() {
    let transformStream = new stream.Transform();
    let ret: boolean = transformStream.isPaused();
    assertFalse(ret);
}

// this test case will be completed future
function testStreamTransformPipe001() {
    let transformStream = new stream.Transform();
    let writable = new stream.Writable();
    let ret: stream.Writable = transformStream.pipe(writable);
    assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamTransformUnpipe001() {
    let transformStream = new stream.Transform();
    let writable = new stream.Writable();
    let ret: stream.Readable = transformStream.unpipe(writable);
    assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamTransformOn001() {
    let transformStream = new stream.Transform();
    let callback: CallbackFnType = (...params: Object[]) => {};
    transformStream.on("data", callback);
    assertTrue(true);
}

// this test case will be completed future
function testStreamTransformOff001() {
    let transformStream = new stream.Transform();
    let callback: CallbackFnType = (...params: Object[]) => {};
    transformStream.off("data", callback);
    assertTrue(true);
}

// this test case will be completed future
function testStreamTransformDoInitialize001() {
    let transformStream = new stream.Transform();
    transformStream.doInitialize(() => {});
    assertTrue(true);
}

// this test case will be completed future
function testStreamTransformDoRead001() {
    let transformStream = new stream.Transform();
    transformStream.doRead(1);
    assertTrue(true);
}

// this test case will be completed future
function testStreamTransformWrite001() {
    let transformStream = new stream.Transform();
    let ret: boolean = transformStream.write("data", "utf-8", () => {});
    assertFalse(ret);
}

// this test case will be completed future
function testStreamTransformEnd001() {
    let transformStream = new stream.Transform();
    let ret: stream.Writable = transformStream.end("data", "utf-8", () => {});
    assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamTransformSetDefaultEncoding001() {
    let transformStream = new stream.Transform();
    let ret: boolean = transformStream.setDefaultEncoding("utf-8");
    assertFalse(ret);
}

// this test case will be completed future
function testStreamTransformCrok001() {
    let transformStream = new stream.Transform();
    let ret: boolean = transformStream.cork();
    assertFalse(ret);
}

// this test case will be completed future
function testStreamTransformUncrok001() {
    let transformStream = new stream.Transform();
    let ret: boolean = transformStream.uncork();
    assertFalse(ret);
}

// this test case will be completed future
function testStreamTransformDoWrite001() {
    let transformStream = new stream.Transform();
    transformStream.doWrite("test", "utf8", () => {});
    assertTrue(true);
}

// this test case will be completed future
function testStreamTransformDoWritev001() {
    let transformStream = new stream.Transform();
    transformStream.doWritev(["test"], () => {});
    assertTrue(true);
}

// this test case will be completed future
function testStreamTransformDoTransform001() {
    let transformStream = new stream.Transform();
    transformStream.doTransform("test", "utf8", () => {});
    assertTrue(true);
}

// this test case will be completed future
function testStreamTransformDoFlush001() {
    let transformStream = new stream.Transform();
    transformStream.doFlush(() => {});
    assertTrue(true);
}
