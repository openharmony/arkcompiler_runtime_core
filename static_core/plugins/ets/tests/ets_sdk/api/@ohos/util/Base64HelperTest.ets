/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

import {util} from "@ohos.util";

async function main(): Promise<int> {
    const suite = new arktest.ArkTestsuite('Base64Helper Test');

    testEncodeDecodeBasic();
    testEncodeDecodeBasicSync();
    testUrlSafe();
    testMime();
    testEncodeToStringSyncMime();
    testMimeUrlSafe();
    testDecodeWithNoise();
    testEmptyEncode();
    testEmptyDecode();
    testBase64WithoutPadding();
    suite.addTest('Test encodeSync with MIME and MINE_URL_SAFE', testEncodeSyncMineAndMineUrlSafe);
    suite.addTest('Test encode and decode with no options', testNoOptionsEncodeDecode);
    suite.addTest('Test encodeSync and decodeSync with no options', testNoOptionsEncodeDecodeSync);
    suite.addTest('Test all Type round-trip sync', testAllTypesRoundTripSync);
    suite.addTest('Test all Type decode with Uint8Array input', testAllTypesDecodeUint8Array);
    suite.addTest('Test MIME removeNewlines', testMimeRemoveNewlines);

    await testEncodeDecodeBasicAsync();
    await testEncodeDecodeAsyncBytes();
    await testUrlSafeAsync();
    await testDecodeWithNoiseAsync();
    suite.addAsyncTest('Test async encode and decode with no options', testNoOptionsEncodeDecodeAsync);
    suite.addAsyncTest('Test async encode and decode Bytes with no options', testNoOptionsEncodeDecodeAsyncBytes);
    suite.addAsyncTest('Test all Type round-trip async', testAllTypesRoundTripAsync);

    return await suite.runAsync();
}

function assertUint8ArraysEqual(actual: Uint8Array, expected: Uint8Array): void {
    arktest.assertEQ(actual.length, expected.length);
    for (let i = 0; i < expected.length; i++) {
        arktest.assertEQ(actual[i], expected[i]);
    }
}

function testEncodeDecodeBasic() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 101, 108, 108, 111]); // "Hello"
    let encodedStr: String = helper.encodeToStringSync(input, util.Type.BASIC);
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.BASIC);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testEncodeDecodeBasicSync() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([87, 111, 114, 108, 100]); // "World"
    let encodedBytes: Uint8Array = helper.encodeSync(input, util.Type.BASIC);
    let encodedStr: String = "";
    for (let i: int = 0; i < encodedBytes.length; i++) {
        encodedStr += String.fromCharCode(encodedBytes[i].toInt());
    }
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.BASIC);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testUrlSafe() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([250, 251, 252, 253, 254, 255]);
    let encodedStr: String = helper.encodeToStringSync(input, util.Type.BASIC_URL_SAFE);
    arktest.assertEQ(encodedStr.indexOf('+'), -1);
    arktest.assertEQ(encodedStr.indexOf('/'),-1);
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.BASIC_URL_SAFE);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testMime() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array(60);
    for (let i: int = 0; i < 60; i++) {
        input[i] = i;
    }
    let encodedStr: String = helper.encodeToStringSync(input, util.Type.MIME);
    arktest.assertNE(encodedStr.indexOf("\r\n"), -1.0);
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.MIME);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testEncodeToStringSyncMime() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([66, 97, 115, 101, 54, 52, 32, 78, 111, 100, 101, 46, 106, 115, 54, 52, 32,
        78, 111, 100, 101, 46, 106, 115, 115, 54, 52, 32, 78, 111, 100, 101, 46, 106, 115, 115, 115, 54, 52,
        32, 78, 111, 100, 101, 46, 106, 115, 105, 111, 110, 102, 114, 111, 109, 111, 116, 104, 101, 114, 97,
        110, 105, 109, 97, 108, 115, 119, 104, 105, 99, 104, 105, 115, 97, 108]);
    let encodedStr: String = helper.encodeToStringSync(input, util.Type.MIME);
    arktest.assertEQ(encodedStr.indexOf("\r\n"), 76);
    arktest.assertEQ(encodedStr.lastIndexOf("\r\n"), 102);
    arktest.assertEQ(encodedStr, "QmFzZTY0IE5vZGUuanM2NCBOb2RlLmpzczY0IE5vZGUuanNzczY0IE5vZGUuanNpb25mcm9tb3Ro\r\nZXJhbmltYWxzd2hpY2hpc2Fs\r\n");
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.MIME);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testMimeUrlSafe() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array(80);
    for (let i: int = 0; i < 80; i++) {
        input[i] = i;
    }
    let encodedStr: string = helper.encodeToStringSync(input, util.Type.MIME_URL_SAFE);
    arktest.assertNE(encodedStr.indexOf("\r\n"), -1.0); 
    arktest.assertEQ(encodedStr.indexOf("+"), -1);
    arktest.assertEQ(encodedStr.indexOf("/"), -1);
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.MIME_URL_SAFE);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testDecodeWithNoise() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 105]); // "Hi"
    let encodedStr: String = helper.encodeToStringSync(input, util.Type.BASIC);
    let noisy: String = "!!" + encodedStr + "##";
    let decoded: Uint8Array = helper.decodeSync(noisy, util.Type.BASIC);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testEmptyEncode() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array(0);
    let encodedStr: string = helper.encodeToStringSync(input, util.Type.BASIC);
    arktest.assertEQ(encodedStr, "");
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.BASIC);
    arktest.assertEQ(decoded.length, 0);
}

function testEmptyDecode() {
    let helper = new util.Base64Helper();
    let encodedStr = "";
    let decoded: Uint8Array = helper.decodeSync(encodedStr, util.Type.BASIC);
    arktest.assertEQ(decoded.length, 0);
}

function testBase64WithoutPadding() {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([1, 2, 3, 4, 5, 6]);
    let encodedStr: string = helper.encodeToStringSync(input, util.Type.BASIC);
    let encodedWithoutPadding = encodedStr.replace(new RegExp("=+$"), "");
    
    let decoded: Uint8Array = helper.decodeSync(encodedWithoutPadding, util.Type.BASIC);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testNoOptionsEncodeDecode(): void {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 101, 108, 108, 111]); // "Hello"
    let encodedStr: String = helper.encodeToStringSync(input);
    let decoded: Uint8Array = helper.decodeSync(encodedStr);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testNoOptionsEncodeDecodeSync(): void {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([87, 111, 114, 108, 100]); // "World"
    let encodedBytes: Uint8Array = helper.encodeSync(input);
    let encodedStr: String = "";
    for (let i: int = 0; i < encodedBytes.length; i++) {
        encodedStr += String.fromCharCode(encodedBytes[i].toInt());
    }
    let decoded: Uint8Array = helper.decodeSync(encodedStr);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testEncodeSyncMineAndMineUrlSafe():void{
    let helper = new util.Base64Helper();
    let input = new Uint8Array([115, 49, 51]);
    arktest.expectError(() => {
		helper.encodeSync(input, util.Type.MIME);
	}, 'Parameter error. The target encoding type option must be BASIC or BASIC_URL_SAFE.');
    arktest.expectError(() => {
		helper.encodeSync(input, util.Type.MIME_URL_SAFE);
	}, 'Parameter error. The target encoding type option must be BASIC or BASIC_URL_SAFE.');
}

async function testEncodeDecodeBasicAsync(): Promise<void> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 101, 108, 108, 111]); // "Hello"
    let encodedStr: string = await helper.encodeToString(input, util.Type.BASIC);
    let decoded: Uint8Array = await helper.decode(encodedStr, util.Type.BASIC);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

async function testEncodeDecodeAsyncBytes(): Promise<void> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([87, 111, 114, 108, 100]); // "World"
    let encodedBytes: Uint8Array = await helper.encode(input, util.Type.BASIC);
    let encodedStr: string = "";
    for (let i: int = 0; i < encodedBytes.length; i++) {
        encodedStr += String.fromCharCode(encodedBytes[i].toInt());
    }
    let decoded: Uint8Array = await helper.decode(encodedStr, util.Type.BASIC);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

async function testUrlSafeAsync(): Promise<void> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([250, 251, 252, 253, 254, 255]);
    let encodedStr: string = await helper.encodeToString(input, util.Type.BASIC_URL_SAFE);
    arktest.assertEQ(encodedStr.indexOf('+'), -1);
    arktest.assertEQ(encodedStr.indexOf('/'), -1);
    let decoded: Uint8Array = await helper.decode(encodedStr, util.Type.BASIC_URL_SAFE);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

async function testDecodeWithNoiseAsync(): Promise<void> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 105]); // "Hi"
    let encodedStr: string = await helper.encodeToString(input, util.Type.BASIC);
    let noisy: string = "!!" + encodedStr + "##";
    let decoded: Uint8Array = await helper.decode(noisy, util.Type.BASIC);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

async function testNoOptionsEncodeDecodeAsync(): Promise<void> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 101, 108, 108, 111]); // "Hello"
    let encodedStr: string = await helper.encodeToString(input);
    let decoded: Uint8Array = await helper.decode(encodedStr);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

async function testNoOptionsEncodeDecodeAsyncBytes(): Promise<void> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([87, 111, 114, 108, 100]); // "World"
    let encodedBytes: Uint8Array = await helper.encode(input);
    let encodedStr: string = "";
    for (let i: int = 0; i < encodedBytes.length; i++) {
        encodedStr += String.fromCharCode(encodedBytes[i].toInt());
    }
    let decoded: Uint8Array = await helper.decode(encodedStr);
    arktest.assertEQ(decoded.length, input.length);
    assertUint8ArraysEqual(decoded, input);
}

function testAllTypesRoundTripSync(): void {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([0, 1, 2, 250, 251, 252, 127, 128, 255, 100, 200]);

    // BASIC
    let basicStr: string = helper.encodeToStringSync(input, util.Type.BASIC);
    let basicDec: Uint8Array = helper.decodeSync(basicStr, util.Type.BASIC);
    assertUint8ArraysEqual(basicDec, input);

    // BASIC_URL_SAFE
    let urlSafeStr: string = helper.encodeToStringSync(input, util.Type.BASIC_URL_SAFE);
    arktest.assertEQ(urlSafeStr.indexOf('+'), -1);
    arktest.assertEQ(urlSafeStr.indexOf('/'), -1);
    let urlSafeDec: Uint8Array = helper.decodeSync(urlSafeStr, util.Type.BASIC_URL_SAFE);
    assertUint8ArraysEqual(urlSafeDec, input);

    // MIME
    let mimeStr: string = helper.encodeToStringSync(input, util.Type.MIME);
    let mimeDec: Uint8Array = helper.decodeSync(mimeStr, util.Type.MIME);
    assertUint8ArraysEqual(mimeDec, input);

    // MIME_URL_SAFE
    let mimeUrlStr: string = helper.encodeToStringSync(input, util.Type.MIME_URL_SAFE);
    let mimeUrlDec: Uint8Array = helper.decodeSync(mimeUrlStr, util.Type.MIME_URL_SAFE);
    assertUint8ArraysEqual(mimeUrlDec, input);
}

function testAllTypesDecodeUint8Array(): void {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([72, 101, 108, 108, 111]); // 'Hello'

    let basicStr: string = helper.encodeToStringSync(input, util.Type.BASIC);
    let basicBytes: Uint8Array = new Uint8Array(basicStr.length);
    for (let i: int = 0; i < basicStr.length; i++) {
        basicBytes[i] = basicStr.charCodeAt(i);
    }
    let dec1: Uint8Array = helper.decodeSync(basicBytes, util.Type.BASIC);
    assertUint8ArraysEqual(dec1, input);

    let urlStr: string = helper.encodeToStringSync(input, util.Type.BASIC_URL_SAFE);
    let urlBytes: Uint8Array = new Uint8Array(urlStr.length);
    for (let i: int = 0; i < urlStr.length; i++) {
        urlBytes[i] = urlStr.charCodeAt(i);
    }
    let dec2: Uint8Array = helper.decodeSync(urlBytes, util.Type.BASIC_URL_SAFE);
    assertUint8ArraysEqual(dec2, input);

    let mimeStr: string = helper.encodeToStringSync(input, util.Type.MIME);
    let mimeBytes: Uint8Array = new Uint8Array(mimeStr.length);
    for (let i: int = 0; i < mimeStr.length; i++) {
        mimeBytes[i] = mimeStr.charCodeAt(i);
    }
    let dec3: Uint8Array = helper.decodeSync(mimeBytes, util.Type.MIME);
    assertUint8ArraysEqual(dec3, input);

    let mimeUrlStr: string = helper.encodeToStringSync(input, util.Type.MIME_URL_SAFE);
    let mimeUrlBytes: Uint8Array = new Uint8Array(mimeUrlStr.length);
    for (let i: int = 0; i < mimeUrlStr.length; i++) {
        mimeUrlBytes[i] = mimeUrlStr.charCodeAt(i);
    }
    let dec4: Uint8Array = helper.decodeSync(mimeUrlBytes, util.Type.MIME_URL_SAFE);
    assertUint8ArraysEqual(dec4, input);
}

async function testAllTypesRoundTripAsync(): Promise<void> {
    let helper = new util.Base64Helper();
    let input = new Uint8Array([0, 1, 2, 250, 251, 252, 127, 128, 255, 100, 200]);

    // BASIC
    let basicStr: string = await helper.encodeToString(input, util.Type.BASIC);
    let basicDec: Uint8Array = await helper.decode(basicStr, util.Type.BASIC);
    assertUint8ArraysEqual(basicDec, input);

    // BASIC_URL_SAFE
    let urlSafeStr: string = await helper.encodeToString(input, util.Type.BASIC_URL_SAFE);
    arktest.assertEQ(urlSafeStr.indexOf('+'), -1);
    arktest.assertEQ(urlSafeStr.indexOf('/'), -1);
    let urlSafeDec: Uint8Array = await helper.decode(urlSafeStr, util.Type.BASIC_URL_SAFE);
    assertUint8ArraysEqual(urlSafeDec, input);

    // MIME
    let mimeStr: string = await helper.encodeToString(input, util.Type.MIME);
    let mimeDec: Uint8Array = await helper.decode(mimeStr, util.Type.MIME);
    assertUint8ArraysEqual(mimeDec, input);

    // MIME_URL_SAFE
    let mimeUrlStr: string = await helper.encodeToString(input, util.Type.MIME_URL_SAFE);
    let mimeUrlDec: Uint8Array = await helper.decode(mimeUrlStr, util.Type.MIME_URL_SAFE);
    assertUint8ArraysEqual(mimeUrlDec, input);
}

function testMimeRemoveNewlines(): void {
    let helper = new util.Base64Helper();

    // Large input to produce multiple \r\n line breaks in MIME encoding
    let input = new Uint8Array(200);
    for (let i: int = 0; i < 200; i++) {
        input[i] = i % 256;
    }

    // MIME: encode produces \r\n, decode must strip them
    let mimeStr: string = helper.encodeToStringSync(input, util.Type.MIME);
    arktest.assertNE(mimeStr.indexOf('\r\n'), -1);
    let mimeDec: Uint8Array = helper.decodeSync(mimeStr, util.Type.MIME);
    assertUint8ArraysEqual(mimeDec, input);

    // MIME_URL_SAFE: same newline behavior + URL-safe chars
    let mimeUrlStr: string = helper.encodeToStringSync(input, util.Type.MIME_URL_SAFE);
    arktest.assertNE(mimeUrlStr.indexOf('\r\n'), -1);
    let mimeUrlDec: Uint8Array = helper.decodeSync(mimeUrlStr, util.Type.MIME_URL_SAFE);
    assertUint8ArraysEqual(mimeUrlDec, input);

    // Verify MIME string with manual \r\n insertion decodes correctly
    let basicStr: string = helper.encodeToStringSync(input, util.Type.BASIC);
    let mimeManual: string = '';
    for (let i: int = 0; i < basicStr.length; i += 76) {
        let end: int = i + 76;
        if (end > basicStr.length) {
            end = basicStr.length;
        }
        mimeManual += basicStr.substring(i, end) + '\r\n';
    }
    let mimeManualDec: Uint8Array = helper.decodeSync(mimeManual, util.Type.MIME);
    assertUint8ArraysEqual(mimeManualDec, input);

    // Verify decoding MIME string as string input (not Uint8Array)
    let mimeDecStr: Uint8Array = helper.decodeSync(mimeStr, util.Type.MIME);
    assertUint8ArraysEqual(mimeDecStr, input);
}
