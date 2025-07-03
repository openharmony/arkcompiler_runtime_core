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

    suite.addTest("Create a writable stream", testStreamWritableConstructor001);
    suite.addTest("Test stream writable write", testStreamWritableWrite001);
    suite.addTest("Test stream writable end", testStreamWritableEnd001);
    suite.addTest("Test stream writable setDefaultEncoding", testStreamWritableSetDefaultEncoding001);
    suite.addTest("Test stream writable cork", testStreamWritableCork001);
    suite.addTest("Test stream writable uncork", testStreamWritableUncork001);
    suite.addTest("Test stream writable on", testStreamWritableOn001);
    suite.addTest("Test stream writable off", testStreamWritableOff001);
    suite.addTest("Test stream writable doInitialize", testStreamWritableDoInitialize001);
    suite.addTest("Test stream writable doWrite", testStreamWritableDoWrite001);
    suite.addTest("Test stream writable doWritev", testStreamWritableDoWritev001);

    return suite.run()
}

// this test case will be completed future
function testStreamWritableConstructor001() {
    let writable = new stream.Writable();
    assertEQ(typeof writable, "object");
    assertNE(writable, undefined);
}

// this test case will be completed future
function testStreamWritableWrite001() {
    let writableStream = new stream.Writable();
    let ret: boolean = writableStream.write("test", "utf8", () => {});
    assertFalse(ret);
}

// this test case will be completed future
function testStreamWritableEnd001() {
    let writableStream = new stream.Writable();
    let ret: stream.Writable = writableStream.end("test", "utf8", () => {});
    assertNE(ret, undefined);
}

// this test case will be completed future
function testStreamWritableSetDefaultEncoding001() {
    let writableStream = new stream.Writable();
    let ret: boolean = writableStream.setDefaultEncoding("utf8");
    assertFalse(ret);
}

// this test case will be completed future
function testStreamWritableCork001() {
    let writableStream = new stream.Writable();
    let ret: boolean = writableStream.cork();
    assertFalse(ret);
}

// this test case will be completed future
function testStreamWritableUncork001() {
    let writableStream = new stream.Writable();
    let ret: boolean = writableStream.uncork();
    assertFalse(ret);
}

// this test case will be completed future
function testStreamWritableOn001() {
    let writableStream = new stream.Writable();
    let callback: CallbackFnType = (...params: Object[]) => {};
    writableStream.on("data", callback);
    assertTrue(true);
}

// this test case will be completed future
function testStreamWritableOff001() {
    let writableStream = new stream.Writable();
    let callback: CallbackFnType = (...params: Object[]) => {};
    writableStream.off("data", callback);
    assertTrue(true);
}

// this test case will be completed future
function testStreamWritableDoInitialize001() {
    let writableStream = new stream.Writable();
    writableStream.doInitialize(() => {});
    assertTrue(true);
}

// this test case will be completed future
function testStreamWritableDoWrite001() {
    let writableStream = new stream.Writable();
    writableStream.doWrite("test", "utf8", () => {});
    assertTrue(true);
}

// this test case will be completed future
function testStreamWritableDoWritev001() {
    let writableStream = new stream.Writable();
    writableStream.doWritev(["test"], () => {});
    assertTrue(true);
}
