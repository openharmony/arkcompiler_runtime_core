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
    const suite = new ArkTestsuite("Buffer Blob API tests")

    suite.addTest("Test create Blob from String array without options", testBlobFromStringArray)
    suite.addTest("Test create Blob from ArrayBuffer without options", testBlobFromArrayBuffer)
    suite.addTest("Test create Blob from TypedArray without options", testBlobFromTypedArray)
    suite.addTest("Test create Blob from DataView without options", testBlobFromDataView)
    suite.addTest("Test create Blob from other Blob without options", testBlobFromBlob)
    suite.addTest("Test create Blob from String array with options", testBlobWithOptions)

    suite.addTest("Test get Blob slice from String array without options", testStringArraySlice)
    suite.addTest("Test get Blob slice from ArrayBuffer without options", testArrayBufferSlice)
    suite.addTest("Test get Blob slice from TypedArray without options", testTypedArraySlice)
    suite.addTest("Test get Blob slice from DataView without options", testDataViewSlice)
    suite.addTest("Test get Blob slice from other Blob without options", testBlobSlice)
    suite.addTest("Test get Blob toArrayBuffer", testBlobToArrayBuffer)

    return suite.run()
}

function testBlobFromStringArray() {
    let src: FixedArray<string> = ["1", "2", "3", "4", "5"]
    let blob = new buffer.Blob(src)
    assertEQ(blob.type, "")
    assertEQ(blob.size, 5)
    let text = await blob.text()
    assertEQ(text, "12345")
}

function testBlobFromArrayBuffer() {
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer

    let src: FixedArray<ArrayBuffer> = [arrayBuffer]
    let blob = new buffer.Blob(src)
    assertEQ(blob.type, "")
    assertEQ(blob.size, 5)
    let text = await blob.text()
    assertEQ(text, "Hello")
}

function testBlobFromTypedArray() {
    let src: FixedArray<Int8Array> = [new Int8Array([1, 2, 3, 4, 5])]
    let blob = new buffer.Blob(src)
    assertEQ(blob.size, 5)
    assertEQ(blob.type, "")
    let text = await blob.text()
    assertEQ(text, "12345")
}

function testBlobFromDataView() {
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer
    const dataView = new DataView(arrayBuffer)
    let src: FixedArray<DataView> = [dataView, dataView, dataView]
    let blob = new buffer.Blob(src)
    assertEQ(blob.type, "")
    assertEQ(blob.size, 15)
    let text = await blob.text()
    assertEQ(text, "HelloHelloHello")
}

function testBlobFromBlob() {
    let src = new buffer.Blob(["1", "2", "3", "4", "5"])
    let blob = new buffer.Blob([src] as FixedArray<buffer.Blob>)
    assertEQ(blob.type, "")
    assertEQ(blob.size, 5)
    let text = await blob.text()
    assertEQ(text, "12345")
}

function testBlobWithOptions() {
    const blobParts: FixedArray<string> = ['<q id="a"><span id="b">hey!</span></q>'];
    const blob = new buffer.Blob(blobParts, { type: "text/html" } as buffer.BlobOptions);
    assertEQ(blob.type, "text/html")
    let text = await blob.text()
    assertEQ(text, '<q id="a"><span id="b">hey!</span></q>')
}

function testStringArraySlice() {
    let src: FixedArray<string> = ["1", "2", "3", "4", "5"]
    let blob = new buffer.Blob(src).slice(0,3)
    assertEQ(blob.type, '')
    assertEQ(blob.size, 3)
    let text = await blob.text()
    assertEQ(text, "123")
}

function testArrayBufferSlice() {
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer
    let src: FixedArray<ArrayBuffer> = [arrayBuffer, arrayBuffer, arrayBuffer, arrayBuffer]
    let blob = new buffer.Blob(src).slice(1,2)
    assertEQ(blob.type, '')
    assertEQ(blob.size, 5)
    let text = await blob.text()
    assertEQ(text, "Hello")
}

function testTypedArraySlice() {
    let src: FixedArray<Int8Array> = [new Int8Array([1, 2, 3]), 
                            new Int8Array([ 4, 5]),
                            new Int8Array([6, 7]),
                            new Int8Array([8, 9, 10]),
                            new Int8Array([11, 12, 13, 14])]
    let blob = new buffer.Blob(src).slice(2,4)
    assertEQ(blob.type, "")
    assertEQ(blob.size, 5)
    let text = await blob.text()
    assertEQ(text, "678910")
}

function testDataViewSlice() {
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer
    let src: FixedArray<DataView> = [new DataView(arrayBuffer)]
    let blob = new buffer.Blob(src).slice()
    assertEQ(blob.type, '')
    assertEQ(blob.size, 5)
    let text = await blob.text()
    assertEQ(text, "Hello")
}

function testBlobSlice() {
    let src1 = new buffer.Blob(["1", "2", "3", "4", "5"])
    let src2 = new buffer.Blob([new Int8Array([1, 2, 3, 4, 5])] as FixedArray<Int8Array>)
    let src3 = new buffer.Blob(["1", "2", "3", "4", "5"])
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer
    let src4 = new buffer.Blob([arrayBuffer, arrayBuffer] as FixedArray<ArrayBuffer>)

    let blob = new buffer.Blob([src1, src2, src3, src4] as FixedArray<buffer.Blob>).slice(2)

    assertEQ(blob.type, '')
    assertEQ(blob.size, 15)
    let text = await blob.text()
    assertEQ(text, "12345HelloHello")
}

function testBlobToArrayBuffer() {
    const arr = new Uint8Array([72, 101, 108, 108, 111])
    const arrayBuffer = arr.buffer as ArrayBuffer
    let src: FixedArray<DataView> = [new DataView(arrayBuffer)]
    let blob = new buffer.Blob(src)
    let arrayBufferFromBlob = await blob.arrayBuffer()
    let text = await blob.text()
    assertEQ(text, "Hello")
    assertEQ(ArrayBuffer.stringify(arrayBufferFromBlob, 'utf8', 0, arrayBufferFromBlob.getByteLength()), text)
}

