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

import buffer from "@ohos.buffer"


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
    suite.addTest("Test buffer.from with string and base64 encoding", testFromStringWithBase64Encoding)
    suite.addTest("Test buffer.from with string with invalid length", testFromStringWithInvalidLength)

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
    suite.addTest("Test Buffer.copy with params", testBufferCopyWithParams)
    suite.addTest("Test Buffer.copy with undefined", testBufferCopyWithUndefined)
    suite.addTest("Test entries", testEntries)
    suite.addTest("Test equals", testEquals)
    suite.addTest("Test fill", testFill)
    suite.addTest("Test keys", testKeys)
    suite.addTest("Test subarray", testSubarray)
    suite.addTest("Test swap16", testSwap16)
    suite.addTest("Test swap32", testSwap32)
    suite.addTest("Test swap64", testSwap64)
    suite.addTest("Test values", testValues)

    suite.addTest("Test Int8 read/write", testInt8ReadWrite)
    suite.addTest("Test Int16 BE/LE read/write", testInt16ReadWrite)
    suite.addTest("Test Int32 BE/LE read/write", testInt32ReadWrite)
    suite.addTest("Test UInt8 read/write", testUInt8ReadWrite)
    suite.addTest("Test UInt16 BE/LE read/write", testUInt16ReadWrite)
    suite.addTest("Test UInt32 BE/LE read/write", testUInt32ReadWrite)
    suite.addTest("Test Float BE/LE read/write", testFloatReadWrite)
    suite.addTest("Test Double BE/LE read/write", testDoubleReadWrite)
    suite.addTest("Test BigInt64 BE/LE read/write", testBigInt64ReadWrite)
    suite.addTest("Test BigUInt64 BE/LE read/write", testBigUInt64ReadWrite)
    suite.addTest("Test variable length Int BE/LE read/write", testVarLengthIntReadWrite)
    suite.addTest("Test variable length UInt BE/LE read/write", testVarLengthUIntReadWrite)
    suite.addTest("Test string write", testStringWrite)
    suite.addTest("Test string write base64", testBufferWriteStringBase64)
    suite.addTest("Test string write single parameter", testBufferWriteStringSingleParam)
    suite.addTest("Test string write with infinity length",  testBufferWriteStringInfinity)
    suite.addTest("Test string write wrong offset", testBufferWriteStringWrongOffset)
    suite.addTest("Test string write out range offset", testBufferWriteStringOutRangeOffset)
    suite.addTest("Test string write out range length", testBufferWriteStringOutRangeLength)

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
    const int8Array = new Int8Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(int8Array), 4, "int8Array byteLength is not 4")

    const uint8Array = new Uint8Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(uint8Array), 4, "uint8Array byteLength is not 4")

    const uint8ClampedArray = new Uint8ClampedArray([1, 2, 3, 4])
    assertEQ(buffer.byteLength(uint8ClampedArray), 4, "uint8ClampedArray byteLength is not 4")

    const int16Array = new Int16Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(int16Array), 8, "int16Array byteLength is not 8")

    const uint16Array = new Uint16Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(uint16Array), 8, "uint16Array byteLength is not 8")

    const int32Array = new Int32Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(int32Array), 16, "int32Array byteLength is not 16")

    const uint32Array = new Uint32Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(uint32Array), 16, "uint32Array byteLength is not 16")

    const float32Array = new Float32Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(float32Array), 16, "float32Array byteLength is not 16")

    const float64Array = new Float64Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(float64Array), 32, "float64Array byteLength is not 32")

    const bigInt64Array = new BigInt64Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(bigInt64Array), 32, "bigInt64Array byteLength is not 32")

    const bigUint64Array = new BigUint64Array([1, 2, 3, 4])
    assertEQ(buffer.byteLength(bigUint64Array), 32, "bigUint64Array byteLength is not 32")
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

function testFromStringWithBase64Encoding() {
    let buf = buffer.from('abc', 'base64');
    assertEQ(buf.toString(), 'i·');
    assertEQ(buf.length as int, 2);
    assertEQ(buf.toString('base64'), 'abc=');
}

function testFromStringWithInvalidLength() {
    let buf = buffer.from(new String('this is a test'), 'utf8', -1);
    assertEQ(buf.length as int, 14);
    assertEQ(buf.toString(), 'this is a test');
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
    assertEQ(testBuffer.toString('base64url'), 'VGVzdDEyMw')
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

function testEmptyAlloc() {
    const escape = '\0'
    let buf = buffer.alloc(2, escape, 'utf8')
    assertEQ(encodeURI(buf.toString('utf8')), '%00%00')
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
    // invariant:
    //     X = transcode(B, S, T)
    //     B = transcode(X, T, S)
    const checkInvariant = (b: buffer.Buffer, s: buffer.BufferEncoding, t: buffer.BufferEncoding) => {
        let x = buffer.transcode(b, s, t);
        let b2 = buffer.transcode(x, t, s);
        assertEQ(b.length, b2.length, `Invariant violated: length of ${s} -> ${t} != length of ${t} -> ${s}`);
        assertEQ(b.toString(s), b2.toString(s), `Invariant violated: ${s} -> ${t} != ${t} -> ${s}`);
    }

    // 1. ascii → hex
    let b1 = buffer.from("Hello, World!", "ascii");
    let b1_hex = buffer.transcode(b1, "ascii", "hex");
    assertEQ(b1_hex.toString("hex"), "48656c6c6f2c20576f726c6421", "Case (1) failed: ascii -> hex");
    checkInvariant(b1, "ascii", "hex");

    // 2. hex → base64
    let b2 = buffer.from("48656c6c6f2c20576f726c6421", "hex");
    let b2_b64 = buffer.transcode(b2, "hex", "base64");
    assertEQ(b2_b64.toString("base64"), "SGVsbG8sIFdvcmxkIQ==", "Case (2) failed: hex -> base64");
    checkInvariant(b2, "hex", "base64");

    // 3. base64 → utf8
    let b3 = buffer.from("SGVsbG8sIFdvcmxkIQ==", "base64");
    let b3_utf8 = buffer.transcode(b3, "base64", "utf8");
    assertEQ(b3_utf8.toString("utf8"), "Hello, World!", "Case (3) failed: base64 -> utf8");
    checkInvariant(b3, "base64", "utf8");

    // 4. utf8 → hex
    let b4 = buffer.from("Привет", "utf8");
    let b4_hex = buffer.transcode(b4, "utf8", "hex");
    assertEQ(b4_hex.toString("hex"), "d09fd180d0b8d0b2d0b5d182", "Case (4) failed: utf8 -> hex");
    checkInvariant(b4, "utf8", "hex");

    // 5. latin1 → utf8
    let b5 = buffer.from("abc", "latin1");
    let b5_utf8 = buffer.transcode(b5, "latin1", "utf8");
    assertEQ(b5_utf8.toString("utf8"), "abc", "Case (5) failed: latin1 -> utf8");
    checkInvariant(b5, "latin1", "utf8");

    // 6. ascii → base64url
    let b6 = buffer.from("Hello-World_", "ascii");
    let b6_b64url = buffer.transcode(b6, "ascii", "base64url");
    assertEQ(b6_b64url.toString("base64url"), "SGVsbG8tV29ybGRf", "Case (6) failed: ascii -> base64url");
    checkInvariant(b6, "ascii", "base64url");

    // 7. base64url → base64
    let b7 = buffer.from("SGVsbG8tV29ybGRf", "base64url");
    let b7_b64 = buffer.transcode(b7, "base64url", "base64");
    assertEQ(b7_b64.toString("base64"), "SGVsbG8tV29ybGRf", "Case (7) failed: base64url -> base64");
    checkInvariant(b7, "base64url", "base64");

    // 8. hex → ascii
    let b8 = buffer.from("48656c6c6f", "hex");
    let b8_ascii = buffer.transcode(b8, "hex", "ascii");
    assertEQ(b8_ascii.toString("ascii"), "Hello", "Case (8) failed: hex -> ascii");
    checkInvariant(b8, "hex", "ascii");

    // 9. base64 → hex
    let b9 = buffer.from("4pyTIMOgIGV4YW1wbGU=", "base64");
    let b9_hex = buffer.transcode(b9, "base64", "hex");
    assertEQ(b9_hex.toString("hex"), "e29c9320c3a0206578616d706c65", "Case (9) failed: base64 -> hex");
    checkInvariant(b9, "base64", "hex");
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

function testBufferCopyWithParams() {
    let b1 = buffer.from("abcdefg")
    let b2 = buffer.from("1235789")
    let num = b1.copy(b2, 2,1,3)
    assertEQ(num, 2)
    assertEQ(b2.toString(), "12bc789")
}

function testBufferCopyWithUndefined() {
    let buf1 = buffer.allocUninitializedFromPool(26);
    let buf2 = buffer.allocUninitializedFromPool(26).fill('!');
    for (let i = 0; i < 26; i++) {
        buf1.writeInt8(i + 97, i);
    }
    buf1.copy(buf2, undefined, 1);
    assertEQ(buf2.toString(), "bcdefghijklmnopqrstuvwxyz!")
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
    let buf = buffer.alloc(5, 'a');
    let subBuf = buf.subarray(0, 3);
    assertEQ(subBuf.length, 3);
    assertEQ(subBuf.toString('ascii'), 'aaa');

    buf = buffer.from("1236")
    subBuf = buf.subarray(0, 3)
    assertEQ(subBuf.toString(), "123")

    subBuf = buf.subarray(undefined)
    assertEQ(subBuf.toString(), "1236")

    subBuf = buf.subarray(3, 4)
    assertEQ(subBuf.toString(), "6")

    subBuf = buf.subarray(-3, 0)
    assertEQ(subBuf.length, 0)

    subBuf = buf.subarray(6, 9)
    assertEQ(subBuf.length, 0)

    let buf1 = buffer.allocUninitializedFromPool(26)
    for (let i = 0; i < 26; ++i) {
        buf1.writeInt8(i + 97, i)
    }

    let buf2 = buf1.subarray(-1)
    let res = buf2.toString('ascii', 0, buf2.length)
    assertEQ(res, '')

    buf2 = buf1.subarray(10, -1)
    res = buf2.toString('ascii', 0, buf2.length)
    assertEQ(res, '')

    buf2 = buf1.subarray(2, 1)
    res = buf2.toString('ascii', 0, buf2.length)
    assertEQ(res, '')

    buf2 = buf1.subarray(2, 2)
    res = buf2.toString('ascii', 0, buf2.length)
    assertEQ(res, '')
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

function testInt8ReadWrite() {
    const buf = buffer.from(new Uint8Array(4))

    // Test positive values
    buf.writeInt8(127, 0)
    assertEQ(buf.readInt8(0), 127)

    // Test negative values
    buf.writeInt8(-128, 1)
    assertEQ(buf.readInt8(1), -128)

    // Test zero
    buf.writeInt8(0, 2)
    assertEQ(buf.readInt8(2), 0)

    // Test overflow
    buf.writeInt8(255, 3) // 255 should be interpreted as -1
    assertEQ(buf.readInt8(3), -1)
}

function testInt16ReadWrite() {
    const buf = buffer.from(new Uint8Array(8))

    // Test Big Endian
    buf.writeInt16BE(32767, 0)
    assertEQ(buf.readInt16BE(0), 32767)

    buf.writeInt16BE(-32768, 2)
    assertEQ(buf.readInt16BE(2), -32768)

    // Test Little Endian
    buf.writeInt16LE(32767, 4)
    assertEQ(buf.readInt16LE(4), 32767)

    buf.writeInt16LE(-32768, 6)
    assertEQ(buf.readInt16LE(6), -32768)
}

function testInt32ReadWrite() {
    const buf = buffer.from(new Uint8Array(8))

    // Test Big Endian
    buf.writeInt32BE(2147483647, 0)
    assertEQ(buf.readInt32BE(0), 2147483647)

    // Test Little Endian
    buf.writeInt32LE(-2147483648, 4)
    assertEQ(buf.readInt32LE(4), -2147483648)
}

function testUInt8ReadWrite() {
    const buf = buffer.from(new Uint8Array(3))

    // Test max value
    buf.writeUInt8(255, 0)
    assertEQ(buf.readUInt8(0), 255)

    // Test zero
    buf.writeUInt8(0, 1)
    assertEQ(buf.readUInt8(1), 0)

    // Test mid value
    buf.writeUInt8(128, 2)
    assertEQ(buf.readUInt8(2), 128)
}

function testUInt16ReadWrite() {
    const buf = buffer.from(new Uint8Array(4))

    // Test Big Endian
    buf.writeUInt16BE(65535, 0)
    assertEQ(buf.readUInt16BE(0), 65535)

    // Test Little Endian
    buf.writeUInt16LE(65535, 2)
    assertEQ(buf.readUInt16LE(2), 65535)
}

function testUInt32ReadWrite() {
    const buf = buffer.from(new Uint8Array(8))

    // Test Big Endian
    buf.writeUInt32BE(4294967295, 0)
    assertEQ(buf.readUInt32BE(0), 4294967295)

    // Test Little Endian
    buf.writeUInt32LE(4294967295, 4)
    assertEQ(buf.readUInt32LE(4), 4294967295)
}

function testFloatReadWrite() {
    const buf = buffer.from(new Uint8Array(8))

    // Test Big Endian
    const testFloat = 3.14159
    buf.writeFloatBE(testFloat, 0)
    // Using approx comparison due to float precision
    assertDoubleEQ(buf.readFloatBE(0), testFloat, 0.00001)

    // Test Little Endian
    buf.writeFloatLE(testFloat, 4)
    assertDoubleEQ(buf.readFloatLE(4), testFloat, 0.00001)
}

function testDoubleReadWrite() {
    const buf = buffer.from(new Uint8Array(16))

    // Test Big Endian
    const testDouble = Math.PI
    buf.writeDoubleBE(testDouble, 0)
    assertEQ(buf.readDoubleBE(0), testDouble)

    // Test Little Endian
    buf.writeDoubleLE(testDouble, 8)
    assertEQ(buf.readDoubleLE(8), testDouble)
}

function testBigInt64ReadWrite() {
    const buf = buffer.from(new Uint8Array(16))

    // Test Big Endian
    const max64 = new BigInt("9223372036854775807")
    const min64 = new BigInt("-9223372036854775808")

    buf.writeBigInt64BE(max64, 0)
    assertEQ(buf.readBigInt64BE(0).toString(), max64.toString())

    // Test Little Endian
    buf.writeBigInt64LE(min64, 8)
    assertEQ(buf.readBigInt64LE(8).toString(), min64.toString())
}

function testBigUInt64ReadWrite() {
    const buf = buffer.from(new Uint8Array(16))

    // Test Big Endian
    const maxU64 = new BigInt("18446744073709551615")

    buf.writeBigUInt64BE(maxU64, 0)
    assertEQ(buf.readBigUInt64BE(0).toString(), maxU64.toString())

    // Test Little Endian
    buf.writeBigUInt64LE(maxU64, 8)
    assertEQ(buf.readBigUInt64LE(8).toString(), maxU64.toString())
}

function testVarLengthIntReadWrite() {
    const buf = buffer.from(new Uint8Array(12))

    // Test 2-byte values
    buf.writeIntBE(0x1234, 0, 2)
    assertEQ(buf.readIntBE(0, 2), 0x1234)

    buf.writeIntLE(0x1234, 2, 2)
    assertEQ(buf.readIntLE(2, 2), 0x1234)

    // Test 3-byte values
    buf.writeIntBE(0x123456, 4, 3)
    assertEQ(buf.readIntBE(4, 3), 0x123456)

    buf.writeIntLE(0x123456, 7, 3)
    assertEQ(buf.readIntLE(7, 3), 0x123456)

    // Test negative values
    const negValue = -0x1234;
    buf.writeIntBE(negValue, 10, 2)
    assertEQ(buf.readIntBE(10, 2), negValue)
}

function testVarLengthUIntReadWrite() {
    const buf = buffer.from(new Uint8Array(12))

    // Test 2-byte values
    buf.writeUIntBE(0xFFFF, 0, 2)
    assertEQ(buf.readUIntBE(0, 2), 0xFFFF)

    buf.writeUIntLE(0xFFFF, 2, 2)
    assertEQ(buf.readUIntLE(2, 2), 0xFFFF)

    // Test 3-byte values
    buf.writeUIntBE(0xFEDCBA, 4, 3)
    assertEQ(buf.readUIntBE(4, 3), 0xFEDCBA)

    buf.writeUIntLE(0xFEDCBA, 7, 3)
    assertEQ(buf.readUIntLE(7, 3), 0xFEDCBA)
}

function testStringWrite() {
    const buf = buffer.from(new Uint8Array(20))

    // Test basic ASCII write
    const str = "Hello"
    const bytesWritten = buf.write(str, 0, str.length, 'utf8')
    assertEQ(bytesWritten, str.length)
    assertEQ(buf.toString('utf8', 0, str.length), str)

    // Test write with offset
    const str2 = "World"
    const bytesWritten2 = buf.write(str2, 10, str2.length, 'utf8')
    assertEQ(bytesWritten2, str2.length)
    assertEQ(buf.toString('utf8', 10, 10 + str2.length), str2)

    // Test partial write
    const longStr = "ThisIsALongString"
    const bytesWritten3 = buf.write(longStr, 0, 5, 'utf8')
    assertEQ(bytesWritten3, 5)
    assertEQ(buf.toString('utf8', 0, 5), "ThisI")
}

function testBufferWriteStringBase64() {
    let buf = buffer.alloc(10)
    let res = buf.write("abcdefg", 3, 1, 'base64')
    assertEQ(res, 1)
    assertEQ(buf.toString('base64'), "AAAAaQAAAAAAAA==")
}

function testBufferWriteStringSingleParam() {
    const str = "abcdefg"
    let buf = buffer.alloc(10)
    let res = buf.write(str)
    assertEQ(res, str.length)
    assertEQ(buf.toString('utf8', 0, str.length), str)
}

function testBufferWriteStringInfinity() {
    const str = "abcdefg"
    let buf = buffer.alloc(10)
    expectError(() => {
        buf.write(str, 0, Infinity);
    }, "The value of \"length\" is out of range. It must be >= 0 and <= 10. Received value is: Infinity");
}

function testBufferWriteStringWrongOffset() {
    const str = "abc"
    let buf = buffer.alloc(10)
    expectError(() => {
		buf.write(str, -12)
    }, "The value of \"offset\" is out of range. It must be >= 0 and <= 10. Received value is: -12");
}

function testBufferWriteStringOutRangeOffset() {
    let buf = buffer.alloc(10);
    expectError(() => {
        buf.write("abc", -Infinity, -Infinity, 'utf8');
    }, "The value of \"offset\" is out of range. It must be >= 0 and <= 10. Received value is: -Infinity");
}

function testBufferWriteStringOutRangeLength() {
    let buf = buffer.alloc(10);
    expectError(() => {
        buf.write("abcdefg", 3, -1);
    }, "The value of \"length\" is out of range. It must be >= 0 and <= 10. Received value is: -1");
}
