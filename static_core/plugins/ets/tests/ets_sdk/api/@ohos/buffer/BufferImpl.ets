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

import { buffer } from "api/@ohos.buffer.ets"


function main(): int {
    const suite = new ArkTestsuite("Buffer API tests")

    suite.addTest("Test basic string encodings", testBasicStringEncodings)
    suite.addTest("Test Unicode string handling", testUnicodeString)
    suite.addTest("Test partial buffer reading", testPartialBufferReading)
    suite.addTest("Test mixed content handling", testMixedContent)
    suite.addTest("Test numbers handling", testNumbersHandling)
    suite.addTest("Test special characters", testSpecialCharacters)
    suite.addTest("Test edge cases", testEdgeCases)
    suite.addTest("Test all supported encodings", testAllSupportedEncodings)

    suite.addTest("Test byteLength with ASCII string", testByteLengthAscii)
    suite.addTest("Test byteLength with UTF8 string", testByteLengthUtf8)
    suite.addTest("Test byteLength with special characters", testByteLengthSpecialChars)
    suite.addTest("Test byteLength with TypedArrays", testByteLengthTypedArrays)
    suite.addTest("Test byteLength with Buffer and Views", testByteLengthBufferAndViews)

    suite.addTest("Test buffer.from with number array", testFromArray)
    suite.addTest("Test buffer.from with ArrayBuffer", testFromArrayBuffer)
    suite.addTest("Test buffer.from with Buffer/Uint8Array", testFromBufferOrUint8Array)
    suite.addTest("Test buffer.from with string", testFromString)

    suite.addTest("Test alloc", testAlloc)
    suite.addTest("Test allocUninitialized", testAllocUninitialized)
    suite.addTest("Test allocUninitializedFromPool", testAllocUninitializedFromPool)
    suite.addTest("Test compare", testCompare)
    suite.addTest("Test concat", testConcat)
    suite.addTest("Test isBuffer", testIsBuffer)
    suite.addTest("Test isEncoding", testIsEncoding)
    suite.addTest("Test transcode", testTranscode)
    suite.addTest("Test Buffer.compare", testBufferCompare)
    suite.addTest("Test Buffer.copy", testBufferCopy)
    suite.addTest("Test entries", testEntries)
    suite.addTest("Test equals", testEquals)
    suite.addTest("Test fill", testFill)
    suite.addTest("Test keys", testKeys)
    suite.addTest("Test subarray", testSubarray)
    suite.addTest("Test swap16", testSwap16)
    suite.addTest("Test swap32", testSwap32)
    suite.addTest("Test swap64", testSwap64)
    suite.addTest("Test values", testValues)

    return suite.run()
}

// byteLength tests
function testByteLengthAscii() {
    const testStr = 'ABC123'

    // Test different ASCII-compatible encodings
    assertEQ(buffer.byteLength(testStr, 'ascii'), 6)
    assertEQ(buffer.byteLength(testStr, 'utf8'), 6)
    assertEQ(buffer.byteLength(testStr, 'utf-8'), 6)
    assertEQ(buffer.byteLength(testStr, 'latin1'), 6)
    assertEQ(buffer.byteLength(testStr, 'binary'), 6)
}

function testByteLengthUtf8() {
    const testStr = 'ABC123'

    // Test UTF-16 variants
    assertEQ(buffer.byteLength(testStr, 'utf16le'), 12)
    assertEQ(buffer.byteLength(testStr, 'ucs2'), 12)
    assertEQ(buffer.byteLength(testStr, 'ucs-2'), 12)
}

function testByteLengthSpecialChars() {
    const specialStr = '你好，世界🌍'

    assertEQ(buffer.byteLength(specialStr, 'utf8'), 19)
    assertEQ(buffer.byteLength(specialStr, 'utf16le'), 38)
    assertEQ(buffer.byteLength(specialStr, 'ascii'), 19)
}

function testByteLengthTypedArrays() {
    // Test with different TypedArrays
    const int8Array = new Int8Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(int8Array), 4)
}

function testByteLengthBufferAndViews() {
    const bufferData = buffer.from('Hello, world!')
    assertEQ(buffer.byteLength(bufferData), 13)
    assertEQ(buffer.byteLength(bufferData, 'utf8'), 13)

    const arrayBuffer = new ArrayBuffer(16)
    const dataView = new DataView(arrayBuffer)
    assertEQ(buffer.byteLength(dataView), 16)
    assertEQ(buffer.byteLength(arrayBuffer), 16)
}

// buffer.from tests
function testFromArray() {
    const arr = [72, 101, 108, 108, 111] // "Hello" in ASCII
    const buf = buffer.from(arr)
    assertEQ(buf.length, 5)
    assertEQ(buf.toString('ascii'), "Hello")

    const singular = [256, 300, -1, 255]
    const truncatedBuf = buffer.from(singular)
    assertEQ(truncatedBuf.length, 4)
    // Values should be truncated to fit in 0-255 range
    assertEQ(truncatedBuf.at(0), 0) // 256 -> 0
    assertEQ(truncatedBuf.at(1), 44) // 300 -> 44
    assertEQ(truncatedBuf.at(2), -1) // -1 -> 255
    assertEQ(truncatedBuf.at(3), -1)
}

function testFromArrayBuffer() {
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer

    const buf1 = buffer.from(arrayBuffer)
    assertEQ(buf1.length, 5)
    assertEQ(buf1.toString('ascii'), "Hello")

    const buf2 = buffer.from(arrayBuffer, 1)
    assertEQ(buf2.length, 4)
    assertEQ(buf2.toString('ascii'), "ello")

    const buf3 = buffer.from(arrayBuffer, 1, 2)
    assertEQ(buf3.length, 2)
    assertEQ(buf3.toString('ascii'), "el")
}

function testFromBufferOrUint8Array() {
    const originalBuf = buffer.from([72, 101, 108, 108, 111])
    const copiedBuf = buffer.from(originalBuf)
    assertEQ(copiedBuf.length, originalBuf.length)
    assertEQ(copiedBuf.toString(), originalBuf.toString())

    const uint8Arr = new Uint8Array([72, 101, 108, 108, 111])
    const bufFromUint8 = buffer.from(uint8Arr)
    assertEQ(bufFromUint8.length, uint8Arr.length)
    assertEQ(bufFromUint8.toString('ascii'), "Hello")
}

function testFromString() {
    const str = "Hello, World!"
    const buf1 = buffer.from(str)
    assertEQ(buf1.toString(), str)

    const buf2 = buffer.from(str, 'ascii')
    assertEQ(buf2.toString('ascii'), str)

    const specialStr = '你好，世界🌍'
    const buf4 = buffer.from(specialStr)
    assertEQ(buf4.toString(), specialStr)
}

// Buffer.toString() tests
function testBasicStringEncodings() {
    const asciiStr = 'Hello, World!'
    const asciiBuffer = buffer.from(asciiStr)
    assertEQ(asciiBuffer.toString('ascii'), 'Hello, World!')
    assertEQ(asciiBuffer.toString('utf8'), 'Hello, World!')
    assertEQ(asciiBuffer.toString('utf-8'), 'Hello, World!')
    assertEQ(asciiBuffer.toString('base64'), 'SGVsbG8sIFdvcmxkIQ==')
    assertEQ(asciiBuffer.toString('hex'), '48656c6c6f2c20576f726c6421')
}

function testUnicodeString() {
    const unicodeStr = '你好，世界🌍'
    const unicodeBuffer = buffer.from(unicodeStr)
    assertEQ(unicodeBuffer.toString('utf8'), '你好，世界🌍')
    assertEQ(unicodeBuffer.toString('base64'), '5L2g5aW977yM5LiW55WM8J+MjQ==')
}

function testPartialBufferReading() {
    const asciiStr = 'Hello, World!'
    const asciiBuffer = buffer.from(asciiStr)
    assertEQ(asciiBuffer.toString('utf8', 0, 5), 'Hello')
}

function testMixedContent() {
    const mixedStr = 'Hi, 世界!'
    const mixedBuffer = buffer.from(mixedStr)
    assertEQ(mixedBuffer.toString('utf8'), 'Hi, 世界!')
    assertEQ(mixedBuffer.toString('ascii'), 'Hi, 世界!')
    assertEQ(mixedBuffer.toString('base64'), 'SGksIOS4lueVjCE=')
}

function testNumbersHandling() {
    const numbers = '12345'
    const numbersBuffer = buffer.from(numbers)
    assertEQ(numbersBuffer.toString('utf8'), '12345')
    assertEQ(numbersBuffer.toString('ascii'), '12345')
    assertEQ(numbersBuffer.toString('hex'), '3132333435')
}

function testSpecialCharacters() {
    const specialChars = '!@#$%^&*()'
    const specialBuffer = buffer.from(specialChars)
    assertEQ(specialBuffer.toString('utf8'), '!@#$%^&*()')
    assertEQ(specialBuffer.toString('ascii'), '!@#$%^&*()')
    assertEQ(specialBuffer.toString('base64'), 'IUAjJCVeJiooKQ==')
}

function testEdgeCases() {
    // Empty buffer
    const emptyBuffer = buffer.from('')
    assertEQ(emptyBuffer.toString('utf8'), '')
    // Null bytes
    const nullBuffer = buffer.from('\0\0\0')
    assertEQ(nullBuffer.toString('utf8'), '\0\0\0')
    // Large buffer
    const largeStr = 'A'.repeat(1000)
    const largeBuffer = buffer.from(largeStr)
    assertEQ(largeBuffer.toString('utf8', 0, 50), 'A'.repeat(50))
}

function testAllSupportedEncodings() {
    const testStr = 'Test123'
    const testBuffer = buffer.from(testStr)
    assertEQ(testBuffer.toString('ascii'), 'Test123')
    assertEQ(testBuffer.toString('utf8'), 'Test123')
    assertEQ(testBuffer.toString('utf-8'), 'Test123')
    assertEQ(testBuffer.toString('base64'), 'VGVzdDEyMw==')
    assertEQ(testBuffer.toString('base64url'), 'VGVzdDEyMw==')
    assertEQ(testBuffer.toString('hex'), '54657374313233')
}

function testAlloc() {
    const buf = buffer.alloc(10, 'a');
    assertEQ(buf.length, 10);
    assertEQ(buf.toString('ascii'), 'aaaaaaaaaa');

    const buf2 = buffer.alloc(5, 0x1);
    assertEQ(buf2.length, 5);
    assertEQ(buf2.at(0), 0x1);
}

function testAllocUninitialized() {
    const buf = buffer.allocUninitialized(10);
    assertEQ(buf.length, 10);
}

function testAllocUninitializedFromPool() {
    const buf = buffer.allocUninitializedFromPool(10);
    assertEQ(buf.length, 10);
}

function testCompare() {
    const buf1 = buffer.alloc(5, 'a');
    const buf2 = buffer.alloc(5, 'b');
    assertEQ(buffer.compare(buf1, buf2), -1);
    assertEQ(buffer.compare(buf2, buf1), 1);
    assertEQ(buffer.compare(buf1, buf1), 0);
}

function testConcat() {
    const buf1 = buffer.alloc(5, 'a');
    const buf2 = buffer.alloc(5, 'b');
    const concatenated = buffer.concat([buf1, buf2]);
    assertEQ(concatenated.length, 10);
    assertEQ(concatenated.toString('ascii'), 'aaaaabbbbb');
}

function testIsBuffer() {
    const buf = buffer.alloc(5);
    assertEQ(buffer.isBuffer(buf), true);
    assertEQ(buffer.isBuffer({}), false);
}

function testIsEncoding() {
    assertEQ(buffer.isEncoding('utf8'), true);
    assertEQ(buffer.isEncoding('hex'), true);
    assertEQ(buffer.isEncoding('unsupported'), false);
}

function testTranscode() {
    const buf = buffer.alloc(5, 'a');
    const transcoded = buffer.transcode(buf, 'utf8', 'ascii');
    assertEQ(transcoded.toString('ascii'), 'aaaaa');
}

function testBufferCompare() {
    const buf1 = buffer.alloc(5, 'a');
    const buf2 = buffer.alloc(5, 'b');
    assertEQ(buf1.compare(buf2), -1);
    assertEQ(buf2.compare(buf1), 1);
    assertEQ(buf1.compare(buf1), 0);
}

function testBufferCopy() {
    const buf1 = buffer.alloc(5, 'a');
    const buf2 = buffer.alloc(5);
    buf1.copy(buf2);
    assertEQ(buf2.toString('ascii'), 'aaaaa');
}

function testEntries() {
    const buf = buffer.alloc(3, 'a');
    const entries = Array.from(buf.entries());
    assertEQ(entries.length, 3);
    assertEQ(entries[0][1], 97);
}

function testEquals() {
    const buf1 = buffer.alloc(5, 'a');
    const buf2 = buffer.alloc(5, 'a');
    const buf3 = buffer.alloc(5, 'b');
    assertEQ(buf1.equals(buf2), true);
    assertEQ(buf1.equals(buf3), false);
}

function testFill() {
    const buf = buffer.alloc(5);
    buf.fill('b');
    assertEQ(buf.toString('ascii'), 'bbbbb');
}

function testKeys() {
    const buf = buffer.alloc(3, 'a');
    const keys = Array.from(buf.keys());
    assertEQ(keys.length, 3);
    assertEQ(keys[0], 0);
}

function testSubarray() {
    const buf = buffer.alloc(5, 'a');
    const subBuf = buf.subarray(0, 3);
    assertEQ(subBuf.length, 3);
    assertEQ(subBuf.toString('ascii'), 'aaa');
}

function testSwap16() {
    const uint8Array = new Uint8Array([0x1, 0x2, 0x3, 0x4]);
    const buf = buffer.from(uint8Array);
    buf.swap16();
    assertEQ(buf.at(0), 0x2);
    assertEQ(buf.at(1), 0x1);
    assertEQ(buf.at(2), 0x4);
    assertEQ(buf.at(3), 0x3);
}

function testSwap32() {
    const uint8Array = new Uint8Array([0x1, 0x2, 0x3, 0x4]);
    const buf = buffer.from(uint8Array);
    buf.swap32();
    assertEQ(buf.at(0), 0x4);
    assertEQ(buf.at(1), 0x3);
    assertEQ(buf.at(2), 0x2);
    assertEQ(buf.at(3), 0x1);
}

function testSwap64() {
    const uint8Array = new Uint8Array(
        [0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8]
    );
    const buf = buffer.from(uint8Array);
    buf.swap64();
    assertEQ(buf.at(0), 0x8);
    assertEQ(buf.at(1), 0x7);
    assertEQ(buf.at(2), 0x6);
    assertEQ(buf.at(3), 0x5);
    assertEQ(buf.at(4), 0x4);
    assertEQ(buf.at(5), 0x3);
    assertEQ(buf.at(6), 0x2);
    assertEQ(buf.at(7), 0x1);
}

function testValues() {
    const buf = buffer.alloc(3, 'a');
    const values = Array.from(buf.values());
    assertEQ(values.length, 3);
    assertEQ(values[0], 97);
}
