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

import {util} from "api/@ohos.util.ets";

function main(): void {
    testEncodeDecodeBasic();
    testEncodeDecodeBasicSync();
    testUrlSafe();
    testMime();
    testMimeUrlSafe();
    testDecodeWithNoise();
    testEmptyEncode();
    testEmptyDecode();
    testBase64WithoutPadding();

    await testEncodeDecodeBasicAsync();
    await testEncodeDecodeAsyncBytes();
    await testUrlSafeAsync();
    await testDecodeWithNoiseAsync();
}

function assertUint8ArraysEqual(actual: Uint8Array, expected: Uint8Array): void {
    assertEQ(actual.length, expected.length);
    for (let i = 0; i < expected.length; i++) {
        assertEQ(actual[i], expected[i]);
    }
}

function testEncodeDecodeBasic() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 101, 108, 108, 111]); // "Hello"
    let encodedStr: String = helper.encodeToStringSync(input, util.Type.BASIC);
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.BASIC);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testEncodeDecodeBasicSync() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([87, 111, 114, 108, 100]); // "World"
    let encodedBytes: Uint8Array = helper.encodeSync(input, util.Type.BASIC);
    let encodedStr: String = "";
    for (let i: int = 0; i < encodedBytes.length; i++) {
        encodedStr += String.fromCharCode(encodedBytes[i]);
    }
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.BASIC);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testUrlSafe() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([250, 251, 252, 253, 254, 255]);
    let encodedStr: String = helper.encodeToStringSync(input, util.Type.BASIC_URL_SAFE);
    assertEQ(encodedStr.indexOf('+'), -1);
    assertEQ(encodedStr.indexOf('/'),-1);
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.BASIC_URL_SAFE);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testMime() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array(60);
    for (let i: int = 0; i < 60; i++) {
        input[i] = i;
    }
    let encodedStr: String = helper.encodeToStringSync(input, util.Type.MIME);
    assertNE(encodedStr.indexOf("\r\n"), -1.0);
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.MIME);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testMimeUrlSafe() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array(80);
    for (let i: int = 0; i < 80; i++) {
        input[i] = i;
    }
    let encodedStr: string = helper.encodeToStringSync(input, util.Type.MIME_URL_SAFE);
    assertNE(encodedStr.indexOf("\r\n"), -1.0); 
    assertEQ(encodedStr.indexOf("+"), -1);
    assertEQ(encodedStr.indexOf("/"), -1);
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.MIME_URL_SAFE);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testDecodeWithNoise() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 105]); // "Hi"
    let encodedStr: String = helper.encodeToStringSync(input, util.Type.BASIC);
    let noisy: String = "!!" + encodedStr + "##";
    let decoded: Uint8Array = helper.decodeSync(noisy, util.Type.BASIC);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testEmptyEncode() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array(0);
    let encodedStr: string = helper.encodeToStringSync(input, util.Type.BASIC);
    assertEQ(encodedStr, "");
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.BASIC);
    assertEQ(decoded.length, 0);
}

function testEmptyDecode() {
    let helper = new util.Base64Helper();
    let encodedStr = "";
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.BASIC);
    assertEQ(decoded.length, 0);
}

function testBase64WithoutPadding() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([1, 2, 3, 4, 5, 6]);
    let encodedStr: string = helper.encodeToStringSync(input, util.Type.BASIC);
    let encodedWithoutPadding = encodedStr.replace(new RegExp("=+$"), "");
    
    let decoded: Uint8Array = helper.decodeSync(encodedWithoutPadding, util.Type.BASIC);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

async function testEncodeDecodeBasicAsync(): Promise<int> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 101, 108, 108, 111]); // "Hello"
    let encodedStr: string = await helper.encodeToString(input, util.Type.BASIC);
    let decoded: Uint8Array = await helper.decode(encodedStr, util.Type.BASIC);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

async function testEncodeDecodeAsyncBytes(): Promise<void> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([87, 111, 114, 108, 100]); // "World"
    let encodedBytes: Uint8Array = await helper.encode(input, util.Type.BASIC);
    let encodedStr: string = "";
    for (let i: int = 0; i < encodedBytes.length; i++) {
        encodedStr += String.fromCharCode(encodedBytes[i]);
    }
    let decoded: Uint8Array = await helper.decode(encodedStr, util.Type.BASIC);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

async function testUrlSafeAsync(): Promise<void> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([250, 251, 252, 253, 254, 255]);
    let encodedStr: string = await helper.encodeToString(input, util.Type.BASIC_URL_SAFE);
    assertEQ(encodedStr.indexOf('+'), -1);
    assertEQ(encodedStr.indexOf('/'), -1);
    let decoded: Uint8Array = await helper.decode(encodedStr, util.Type.BASIC_URL_SAFE);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

async function testDecodeWithNoiseAsync(): Promise<void> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 105]); // "Hi"
    let encodedStr: string = await helper.encodeToString(input, util.Type.BASIC);
    let noisy: string = "!!" + encodedStr + "##";
    let decoded: Uint8Array = await helper.decode(noisy, util.Type.BASIC);
    assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}
