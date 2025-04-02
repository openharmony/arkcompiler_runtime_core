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

/**
 * Check created resizable ArrayBuffer with various length and maxByteLength parameters
 */
function createResizableArrayBuffer(): void {
    let a = new ArrayBuffer(10)
    // ArrayBuffer is resizable only if `maxByteLength` was passed
    assertFalse(a.resizable)
    assertEQ(a.byteLength, 10)
    assertFalse(a.detached)

    a = new ArrayBuffer(10, 10)
    assertTrue(a.resizable)
    assertEQ(a.byteLength, 10)
    assertFalse(a.detached)

    a = new ArrayBuffer(0, 10)
    assertTrue(a.resizable)
    assertEQ(a.byteLength, 0)
    assertFalse(a.detached)

    const errorCheck = (e: Error | Exception) => { return (e instanceof RangeError) }
    expectThrow((): void throws => { let a1 = new ArrayBuffer(11, -10) }, errorCheck)
    expectThrow((): void throws => { let a1 = new ArrayBuffer(11, 10) }, errorCheck)
}

function checkCreatedArrayBuffer(a: ArrayBuffer, bytes: byte[]): void {
    assertEQ(a.byteLength, bytes.length)
    assertFalse(a.resizable)
    assertFalse(a.detached)

    for (let idx = 0; idx < bytes.length; idx++) {
        assertEQ(bytes[idx], a.at(idx))
    }
}

/**
 * Check ArrayBuffer created using various factory methods
 */
function createArrayBufferFrom(): void {
    let bytes = new byte[0]
    let a = ArrayBuffer.from(bytes)
    checkCreatedArrayBuffer(a, bytes)

    bytes = [1, 2, 3]
    a = ArrayBuffer.from(bytes)
    checkCreatedArrayBuffer(a, bytes)

    a = ArrayBuffer.from(new Uint8Array(a))
    checkCreatedArrayBuffer(a, bytes)

    a = ArrayBuffer.from(a, 0, bytes.length)
    checkCreatedArrayBuffer(a, bytes)

    a = ArrayBuffer.from(a, 1, 1)
    const expectedBytes: byte[] = [bytes[1]]
    checkCreatedArrayBuffer(a, expectedBytes)
}

/**
 * Check resize() method of ArrayBuffer with various capacities and lengths
 */
function resizeArrayBuffer(): void {
    let a = new ArrayBuffer(10)
    let errorCheck = (e: Error | Exception) => { return (e instanceof TypeError) }
    expectThrow((): void throws => { a.resize(5) }, errorCheck)

    a = new ArrayBuffer(10, 10);

    // Shrink length
    a.resize(5);
    assertEQ(a.byteLength, 5)

    errorCheck = (e: Error | Exception) => { return (e instanceof IndexOutOfBoundsError) }
    expectThrow((): void throws => { a.set(5, 0 as byte) }, errorCheck)
    expectThrow((): void throws => { a.at(5) }, errorCheck)

    // Return previous length
    a.resize(10);
    assertEQ(a.byteLength, 10)
    const VALUE = 42 as byte
    a.set(5, VALUE)
    assertEQ(a.at(5), VALUE)

    errorCheck = (e: Error | Exception) => { return (e instanceof RangeError) }
    expectThrow((): void throws => { a.resize(11) }, errorCheck)
    expectThrow((): void throws => { a.resize(-11) }, errorCheck)

    a = new ArrayBuffer(10, 12)
    errorCheck = (e: Error | Exception) => { return (e instanceof IndexOutOfBoundsError) }
    expectThrow((): void throws => { a.set(10, 0 as byte) }, errorCheck)
    expectThrow((): void throws => { a.at(10) }, errorCheck)

    a.resize(11)
    a.set(10, VALUE)
    assertEQ(a.at(10), VALUE)
}

function testManagedAccessors(): void {
    const REDUCED_LENGTH = 5
    const FULL_LENGTH = REDUCED_LENGTH * 2
    let a = new ArrayBuffer(FULL_LENGTH, FULL_LENGTH)
    const expectedBytes = new byte[a.getByteLength()]
    for (let i = 0; i < a.getByteLength(); i++) {
        expectedBytes[i] = i as byte
    }

    for (let i = 0; i < a.getByteLength(); i++) {
        a.set(i, expectedBytes[i])
        assertEQ(a.at(i), expectedBytes[i])
    }

    // Shrink length and check that non-shrinked bytes were not changed
    a.resize(REDUCED_LENGTH)
    for (let i = 0; i < a.getByteLength(); i++) {
        assertEQ(a.at(i), expectedBytes[i])
    }

    // Return the initial length, check that only previously kept (5) bytes were not changed
    a.resize(FULL_LENGTH)
    for (let i = 0; i < REDUCED_LENGTH; i++) {
        assertEQ(a.at(i), expectedBytes[i])
    }
}

function main(): int {
    const suite = new ArkTestsuite("ArrayBuffer properties")

    suite.addTest("Resizable ArrayBuffer creation", createResizableArrayBuffer)
    suite.addTest("ArrayBuffer factory methods", createArrayBufferFrom)
    suite.addTest("ArrayBuffer resize", resizeArrayBuffer)
    suite.addTest("Test at / set methods of managed ArrayBuffer instance", testManagedAccessors)

    return suite.run()
}
