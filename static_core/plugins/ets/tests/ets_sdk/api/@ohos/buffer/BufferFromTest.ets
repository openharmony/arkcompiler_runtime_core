/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import buffer from '@ohos.buffer'

function main(): int {
    const suite = new arktest.ArkTestsuite('Buffer from test')
    suite.addTest('from double[] basic', testFromDoubleArray)
    suite.addTest('from double[] truncate', testFromDoubleArrayTruncate)
    suite.addTest('from double[] empty', testFromDoubleArrayEmpty)

    suite.addTest('from Buffer copy', testFromBufferCopy)
    suite.addTest('from Buffer copy respects byteOffset', testFromBufferCopyByteOffset)
    suite.addTest('from Buffer copy not shared', testFromBufferCopyNotShared)
    suite.addTest('from Buffer empty', testFromBufferEmpty)
    suite.addTest('from Uint8Array shared', testFromUint8ArrayShared)
    suite.addTest('from Uint8Array view byteOffset', testFromUint8ArrayViewByteOffset)

    suite.addTest('from Object string + encoding', testFromObjectStringEnc)
    suite.addTest('from Object string + offset ignored', testFromObjectStringOffset)
    suite.addTest('from Object string + hex encoding', testFromObjectStringHex)
    suite.addTest('from Object ArrayBuffer + offset + length', testFromObjectArrayBufferOffsetLength)
    suite.addTest('from Object ArrayBuffer + encoding', testFromObjectArrayBufferEnc)
    suite.addTest('from Object ArrayBUffer shared', testFromObjectArrayBufferShared)
    suite.addTest('from Object ArrayBuffer offset boundary', testFromObjectArrayBufferOffsetBoundary)
    suite.addTest('from Object ArrayBuffer negative offset throws', testFromObjectArrayBufferNegOffset)
    suite.addTest('from Object Uint8Array shared', testFromObjectUint8ArrayShared)
    suite.addTest('from Object Uint8Array view byteOffset', testFromObjectUint8ArrayViewByteOffset)
    suite.addTest('from Object Uint8Array with length', testFromObjectUint8ArrayWithLength)
    suite.addTest('from Object invalid encoding throws', testFromObjectInvalidEncoding)
    suite.addTest('from Object invalid type throws', testFromObjectInvalidType)

    return suite.run()
}

function testFromDoubleArray(): void {
    const buf = buffer.from([1, 2, 3])
    arktest.assertEQ(buf.length, 3)
    arktest.assertEQ(buf.byteOffset, 0)
    arktest.assertEQ(buf[0], 1)
    arktest.assertEQ(buf[1], 2)
    arktest.assertEQ(buf[2], 3)
}

function testFromDoubleArrayTruncate(): void {
    const buf = buffer.from([256, -1])
    arktest.assertEQ(buf.length, 2)
    arktest.assertEQ(buf.byteOffset, 0)
    arktest.assertEQ(buf[0], 0)
    arktest.assertEQ(buf[1], 255)
}

function testFromDoubleArrayEmpty(): void {
    const buf = buffer.from([])
    arktest.assertEQ(buf.length, 0)
    arktest.assertEQ(buf.byteOffset, 0)
}

function testFromBufferCopy(): void {
    const src = buffer.from([1, 2, 3])
    const buf = buffer.from(src)
    arktest.assertEQ(buf.length, 3)
    arktest.assertEQ(buf.byteOffset, 0)
    arktest.assertEQ(buf[0], 1)
    arktest.assertEQ(buf[1], 2)
    arktest.assertEQ(buf[2], 3)
}

function testFromBufferCopyByteOffset(): void {
    const ab = new ArrayBuffer(10)
    const u8 = new Uint8Array(ab)
    for (let i = 0; i < 10; i++) { u8[i] = i + 1 }
    const src = buffer.from(ab, 3, 4)
    const buf = buffer.from(src)
    arktest.assertEQ(buf.length, 4)
    arktest.assertEQ(buf.byteOffset, 0)
    arktest.assertEQ(buf[0], 4)
    arktest.assertEQ(buf[1], 5)
    arktest.assertEQ(buf[2], 6)
    arktest.assertEQ(buf[3], 7)
}

function testFromBufferCopyNotShared(): void {
    const ab = new ArrayBuffer(10)
    const src = new buffer.Buffer(ab)
    const buf = buffer.from(src)

    arktest.assertEQ(src[0], 0)
    buf[0] = 99
    arktest.assertEQ(src[0], 0)
    arktest.assertNE(buf.buffer, src.buffer)
}

function testFromBufferEmpty(): void {
    const src = buffer.from([] as double[])
    const buf = buffer.from(src)
    arktest.assertEQ(buf.length, 0)
    arktest.assertEQ(buf.byteOffset, 0)
}

function testFromUint8ArrayShared(): void {
    const ab = ArrayBuffer.from([1, 2, 3, 4, 5])
    const u8 = new Uint8Array(ab)
    const buf = buffer.from(u8)
    arktest.assertEQ(buf.length, 5)
    arktest.assertEQ(buf.byteOffset, 0)

    arktest.assertEQ(u8[0], 1)
    buf[0] = 99
    arktest.assertEQ(u8[0], 99)
    arktest.assertEQ(buf.buffer, ab)
}

function testFromUint8ArrayViewByteOffset(): void {
    const ab = ArrayBuffer.from([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    const u8 = new Uint8Array(ab, 3, 5)
    arktest.assertEQ(u8[0], 4)
    arktest.assertEQ(u8[4], 8)

    const buf = buffer.from(u8)
    arktest.assertEQ(buf.length, 5)
    arktest.assertEQ(buf.byteOffset, 3)
    arktest.assertEQ(buf[0], 4)
    arktest.assertEQ(buf[4], 8)
}

function testFromObjectStringEnc(): void {
    const buf = buffer.from('hello' as Object, 'utf8', 5)
    arktest.assertEQ(buf.length, 5)
    arktest.assertEQ(buf.byteOffset, 0)
    arktest.assertEQ(buf[0], 0x68)
    arktest.assertEQ(buf[4], 0x6F)
    arktest.assertEQ(buf.toString('utf8'), 'hello')
}

function testFromObjectStringOffset(): void {
    const buf = buffer.from('hello' as Object, 0, 3)
    arktest.assertEQ(buf.length, 5)
    arktest.assertEQ(buf.byteOffset, 0)
    arktest.assertEQ(buf[0], 0x68)
    arktest.assertEQ(buf[4], 0x6F)
    arktest.assertEQ(buf.toString('utf8'), 'hello')
}

function testFromObjectStringHex(): void {
    const buf = buffer.from('68656c6c6f' as Object, 'hex', 5)
    arktest.assertEQ(buf.length, 5)
    arktest.assertEQ(buf.byteOffset, 0)
    arktest.assertEQ(buf[0], 0x68)
    arktest.assertEQ(buf[4], 0x6F)
    arktest.assertEQ(buf.toString('utf8'), 'hello')
}

function testFromObjectArrayBufferOffsetLength(): void {
    const ab = ArrayBuffer.from([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    const u8 = new Uint8Array(ab)
    arktest.assertEQ(u8[0], 1)
    arktest.assertEQ(u8[1], 2)
    arktest.assertEQ(u8[2], 3)

    const buf = buffer.from(ab as Object, 2, 3)
    arktest.assertEQ(buf.length, 3)
    arktest.assertEQ(buf.byteOffset, 2)
    arktest.assertEQ(buf[0], 3)
    arktest.assertEQ(buf[1], 4)
    arktest.assertEQ(buf[2], 5)
}

function testFromObjectArrayBufferEnc(): void {
    const ab = ArrayBuffer.from([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    const u8 = new Uint8Array(ab)
    const buf = buffer.from(ab as Object, 'utf8', 5)
    arktest.assertEQ(buf.length, 5)
    arktest.assertEQ(buf.byteOffset, 0)
    arktest.assertEQ(buf[0], 1)
}

function testFromObjectArrayBufferShared(): void {
    const ab = ArrayBuffer.from([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    const u8 = new Uint8Array(ab)
    const buf = buffer.from(ab as Object, 0, 10)

    arktest.assertEQ(u8[0], 1)
    arktest.assertEQ(u8[1], 2)
    buf[0] = 101
    buf[1] = 102
    arktest.assertEQ(u8[0], 101)
    arktest.assertEQ(u8[1], 102)
    arktest.assertEQ(buf.buffer, ab)
}

function testFromObjectArrayBufferOffsetBoundary(): void {
    const ab = new ArrayBuffer(10)
    arktest.expectThrow(() => {
        buffer.from(ab as Object, 8, 5)
    }, (e) => e.code == 10200001)
    arktest.expectThrow(() => {
        buffer.from(ab as Object, 0, 100)
    }, (e) => e.code == 10200001)
}

function testFromObjectArrayBufferNegOffset(): void {
    const ab = new ArrayBuffer(10)
    arktest.expectThrow(() => {
        buffer.from(ab as Object, -1, 5)
    }, (e) => e.code == 10200001)
}

function testFromObjectUint8ArrayShared(): void {
    const ab = ArrayBuffer.from([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    const u8 = new Uint8Array(ab, 0, 10)
    const buf = buffer.from(u8 as Object, 0, 5)
    arktest.assertEQ(buf.length, 10)
    arktest.assertEQ(buf.byteOffset, 0)

    arktest.assertEQ(u8[0], 1)
    buf[0] = 99
    arktest.assertEQ(u8[0], 99)
    arktest.assertEQ(buf.buffer, ab)
}

function testFromObjectUint8ArrayViewByteOffset(): void {
    const ab = ArrayBuffer.from([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    const u8 = new Uint8Array(ab, 3, 3)
    arktest.assertEQ(u8[0], 4)
    arktest.assertEQ(u8[1], 5)
    arktest.assertEQ(u8[2], 6)

    const buf = buffer.from(u8 as Object, 3, 3)
    arktest.assertEQ(buf.length, 3)
    arktest.assertEQ(buf.byteOffset, 3)
    arktest.assertEQ(buf[0], 4)
    arktest.assertEQ(buf[1], 5)
    arktest.assertEQ(buf[2], 6)
}

function testFromObjectUint8ArrayWithLength(): void {
    const ab = ArrayBuffer.from([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    const u8 = new Uint8Array(ab, 3, 5)
    arktest.assertEQ(u8.length, 5)
    arktest.assertEQ(u8[0], 4)
    arktest.assertEQ(u8[1], 5)
    arktest.assertEQ(u8[2], 6)

    const buf = buffer.from(u8 as Object, 0, 3)
    arktest.assertEQ(buf.length, 5)
    arktest.assertEQ(buf.byteOffset, 3)
    arktest.assertEQ(buf[0], 4)
    arktest.assertEQ(buf[1], 5)
    arktest.assertEQ(buf[2], 6)
}

function testFromObjectInvalidEncoding(): void {
    arktest.expectThrow(() => {
        buffer.from('hello' as Object, 'invalid_enc', 5)
    }, (e) => e.code == 401)
}

function testFromObjectInvalidType(): void {
    arktest.expectThrow(() => {
        buffer.from(new Float64Array(5) as Object, 0, 5)
    }, (e) => e.code == 401)
}
