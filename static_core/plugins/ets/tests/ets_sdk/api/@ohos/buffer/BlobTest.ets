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

    suite.addTest("Test get Blob slice from empty String array", testEmptyStringSlice)
    suite.addTest("Test get Blob slice from Chinese String array without options", testChineseStringSlice)
    suite.addTest("Test get Blob slice from Chinese and English String array without options", testEnglishStringSlice)
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
    suite.addTest("Test get Blob constructor ArrayString", testBlobToArrayString)
    suite.addTest("Test get Blob constructor ArrayBuffer", testBlobToArrayArrayBuffer)
    suite.addTest("Test get Blob constructor Uint8Array", testBlobToArrayUint8Array)
    suite.addTest("Test get Blob constructor Uint8ClampedArray", testBlobToArrayUint8ClampedArray)
    suite.addTest("Test get Blob constructor Int16Array", testBlobToArrayInt16Array)
    suite.addTest("Test get Blob constructor Uint16Array", testBlobToArrayUint16Array)
    suite.addTest("Test get Blob constructor Uint8Array with options", testBlobToArrayUint8ArrayOptions)

    return suite.run()
}

function testEmptyStringSlice() {
    let src: FixedArray<string> = []
    let blob = new buffer.Blob(src)
    assertEQ(blob.type, "")
    assertEQ(blob.size, 0)
}

function testChineseStringSlice() {
    let sources: FixedArray<string> = ["你好", "世界", "测试"];
    let blob = new buffer.Blob(sources);
    let size = blob.size;
    assertEQ(blob.type, "")
    assertEQ(size, 18)
    let text = await blob.text()
    assertEQ(text, "你好世界测试")
}

function testEnglishStringSlice() {
    let sources: FixedArray<string> = ["测试", "$#", "c"];
    let blob = new buffer.Blob(sources)
    let size = blob.size;
    assertEQ(blob.type, "")
    assertEQ(size, 9)
    let text = await blob.text()
    assertEQ(text, "测试$#c")
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
    let src: FixedArray<Int8Array> = [new Int8Array([97, 98, 99, 97, 98])]
    let blob = new buffer.Blob(src)
    assertEQ(blob.size, 5)
    assertEQ(blob.type, "")
    let text = await blob.text()
    assertEQ(text, "abcab")
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
    let src: FixedArray<Int8Array> = [new Int8Array([97, 98, 99]),
                            new Int8Array([100, 101]),
                            new Int8Array([102, 103]),
                            new Int8Array([97, 98, 100]),
                            new Int8Array([110, 120, 130, 140])]
    let blob = new buffer.Blob(src).slice(0, 1)
    assertEQ(blob.type, "")
    assertEQ(blob.size, 3)
    let text = await blob.text()
    assertEQ(text, "abc")
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

function testBlobToArrayString() {
    let sources = ["测试", "$#", "c"];
    let blob = new buffer.Blob(sources)
    let size = blob.size;
    assertEQ(blob.type, "")
    assertEQ(size, 9)
    let text = await blob.text()
    assertEQ(text, "测试$#c")
}

function testBlobToArrayArrayBuffer() {
    let arr = new ArrayBuffer(3);
    let uint8arr = new Uint8Array(arr);
    uint8arr[0] = 97;
    uint8arr[1] = 98;
    uint8arr[2] = 99;
    let blob = new buffer.Blob([arr]);
    let size = blob.size;
    assertEQ(blob.type, "")
    assertEQ(size, 3)
    let text = await blob.text()
    assertEQ(text, "abc")
}

function testBlobToArrayUint8Array() {
    let uint8arr1 = new Uint8Array(3);
    uint8arr1[0] = 97;
    uint8arr1[1] = 98;
    uint8arr1[2] = 99;
    let blob = new buffer.Blob([uint8arr1]);
    let size = blob.size;
    assertEQ(blob.type, "")
    assertEQ(size, 3)
    let text = await blob.text()
    assertEQ(text, "abc")
}

function testBlobToArrayUint8ClampedArray() {
    let uintc8 = new Uint8ClampedArray(3);
    uintc8 [0] = 100;
    uintc8 [1] = 101;
    uintc8 [2] = 102;
    let blob = new buffer.Blob([uintc8]);
    let size = blob.size;
    assertEQ(blob.type, "")
    assertEQ(size, 3)
    let text = await blob.text()
    assertEQ(text, "def")
}

function testBlobToArrayInt16Array() {
    let int16 = new Int16Array(2);
    int16 [0] = 100;
    int16 [1] = 101;
    let blob = new buffer.Blob([int16]);
    let size = blob.size;
    assertEQ(blob.type, "")
    assertEQ(size, 4)
}

function testBlobToArrayUint16Array() {
    let uint16 = new Uint16Array(5);
    uint16[0] = 72;
    uint16[1] = 101;
    uint16[2] = 108;
    uint16[3] = 108;
    uint16[4] = 111;
    let blob = new buffer.Blob([uint16]);
    let size = blob.size;
    assertEQ(blob.type, "")
    assertEQ(size, 10)
}

function testBlobToArrayUint8ArrayOptions() {
    let o1: buffer.BlobOptions = { type: "new type", endings: "transparent" };
    let uint8arr1 = new Uint8Array(3);
    uint8arr1[0] = 97;
    uint8arr1[1] = 98;
    uint8arr1[2] = 99;
    let blob = new buffer.Blob([uint8arr1], o1);
    let size = blob.size;
    assertEQ(blob.type, "new type")
    assertEQ(size, 3)
    let text = await blob.text()
    assertEQ(text, "abc")
}
