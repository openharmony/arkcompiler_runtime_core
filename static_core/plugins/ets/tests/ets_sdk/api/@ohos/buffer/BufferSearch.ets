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

import buffer from "@ohos.buffer";

function main(): int {
    const suite = new ArkTestsuite("Buffer search methods")
    
    suite.addTest("Test includes with numbers", testIncludesNumbers)
    suite.addTest("Test includes with strings", testIncludesStrings)
    suite.addTest("Test includes with buffers", testIncludesBuffers)
    suite.addTest("Test indexOf with numbers", testIndexOfNumbers)
    suite.addTest("Test indexOf with strings", testIndexOfStrings)
    suite.addTest("Test lastIndexOf with numbers", testLastIndexOfNumbers)
    suite.addTest("Test lastIndexOf with strings", testLastIndexOfStrings)
    suite.addTest("Test indexOf and lastIndexOf with buffers", testIndexOfLastIndexOfBuffers)
    suite.addTest("Test indexOf and lastIndexOf with OutOfBounds", testIndexOfOutOfBounds)
    suite.addTest("Test indexOf and lastIndexOf with OutOfBounds", testLastIndexOfOutOfBounds)
    suite.addTest("Test indexOf and lastIndexOf with buffers", testIndexOfPartialMatch)
    suite.addTest("Test indexOf and lastIndexOf with buffers", testLastIndexOfPartialMatch)
    suite.addTest("Test lastIndexOf with base64", testLastIndexOfBase64)
    
    return suite.run()
}

function testIncludesNumbers() {
    const buf = buffer.from([10, 20, 30, 40, 50])
    assertEQ(buf.includes(30), true)
    assertEQ(buf.includes(60), false)
}

function testIncludesStrings() {
    const buf = buffer.from("hello world", "utf8")
    assertEQ(buf.includes("world"), true)
    assertEQ(buf.includes("test"), false)
}

function testIncludesBuffers() {
    const buf = buffer.from([5, 10, 15, 20, 25])
    const subBuf = buffer.from([10, 15])
    assertEQ(buf.includes(subBuf), true)
    assertEQ(buf.includes(buffer.from([30])), false)
}

function testIndexOfNumbers() {
    const buf = buffer.from([1, 2, 3, 4, 5, 3, 7])
    assertEQ(buf.indexOf(3), 2)
    assertEQ(buf.indexOf(3, 3), 5)
    assertEQ(buf.indexOf(8), -1)
}

function testIndexOfStrings() {
    const buf = buffer.from("hello world", "utf8")
    assertEQ(buf.indexOf("world"), 6)
    assertEQ(buf.indexOf("hello"), 0)
    assertEQ(buf.indexOf("test"), -1)
}

function testLastIndexOfNumbers() {
    const buf = buffer.from([1, 2, 3, 4, 3, 5])
    assertEQ(buf.lastIndexOf(3), 4)
    assertEQ(buf.lastIndexOf(3, 3), 2)
    assertEQ(buf.lastIndexOf(6), -1)
}

function testLastIndexOfStrings() {
    const buf = buffer.from("banana", "utf8");
    assertEQ(buf.lastIndexOf("a"), 5)
    assertEQ(buf.lastIndexOf("an"), 3)
    assertEQ(buf.lastIndexOf("z"), -1)
}

function testIndexOfLastIndexOfBuffers() {
    const buf = buffer.from([10, 20, 30, 40, 20, 50])
    const searchBuf = buffer.from([20])
    assertEQ(buf.indexOf(searchBuf), 1)
    assertEQ(buf.lastIndexOf(searchBuf), 4)
}

function testIndexOfOutOfBounds() {
    const buf = buffer.from([10, 20, 30, 40, 50])
    assertEQ(buf.indexOf(30, 10), -1)
    assertEQ(buf.indexOf(30, -100), 2)
}

function testLastIndexOfOutOfBounds() {
    const buf = buffer.from([10, 20, 30, 40, 50])
    assertEQ(buf.lastIndexOf(30, 100), 2)
    assertEQ(buf.lastIndexOf(30, -100), -1)
}

function testIndexOfPartialMatch() {
    const buf = buffer.from("banana", "utf8")
    assertEQ(buf.indexOf("nan"), 2)
    assertEQ(buf.indexOf("na"), 2)
    assertEQ(buf.indexOf("anaa"), -1)
}

function testLastIndexOfPartialMatch() {
    const buf = buffer.from("banana", "utf8")
    assertEQ(buf.lastIndexOf("nan"), 2)
    assertEQ(buf.lastIndexOf("na"), 4)
    assertEQ(buf.lastIndexOf("anaa"), -1)
}

function testLastIndexOfBase64() {
    let buf = buffer.from("Hello World")
    let value = "World"
    const index = buf.lastIndexOf(value, 1, "base64")
    assertEQ(index, -1)
}
