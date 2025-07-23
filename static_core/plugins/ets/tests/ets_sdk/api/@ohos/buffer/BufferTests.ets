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

function testToString() {
    {
        let buf1 = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf1.writeInt8(i + 97, i);
        }
        const res = buf1.toString('base64'); // YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXo=
        arktest.assertEQ(res, "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXo=");
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', 30); // '';
        arktest.assertEQ(str, '')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', 5, 3); // ''
        arktest.assertEQ(str, '');
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', -1); // abcdefghijklmnopqrstuvwxyz
        arktest.assertEQ(str, 'abcdefghijklmnopqrstuvwxyz');
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', Infinity.toInt()); // ''
        arktest.assertEQ(str, '');
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', -Infinity.toInt()); // abcdefghijklmnopqrstuvwxyz
        arktest.assertEQ(str, 'abcdefghijklmnopqrstuvwxyz')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', undefined, Infinity.toInt()) // abcdefghijklmnopqrstuvwxyz
        arktest.assertEQ(str, 'abcdefghijklmnopqrstuvwxyz')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', 0, -Infinity.toInt()); // ''
        arktest.assertEQ(str, '')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', undefined, -1); // ''
        arktest.assertEQ(str, '')
    }
    {
        const buf = buffer.from('hello');
        const res = buf.toString('base64', 0, -1); // ''
        arktest.assertEQ(res, '')
    }
    {
        const buf = buffer.from('hello');
        const res = buf.toString('base64', 0, Infinity.toInt()); // aGVsbG8=
        arktest.assertEQ(res, 'aGVsbG8=')
    }
    {
        const buf = buffer.from('hello');
        const res = buf.toString('base64', 0, -Infinity.toInt()); // ''
        arktest.assertEQ(res, '')
    }
    {
        let array = new Uint8Array([252, 11, 3, 67, 237, 118, 91, 177, 43]);
        let buf = buffer.from(array);
        let strUrl = buf.toString("base64url");
        let str = buf.toString("base64");
        arktest.assertEQ(strUrl, "_AsDQ-12W7Er");
        arktest.assertEQ(str, "/AsDQ+12W7Er");

        array = new Uint8Array([2, 192, 254, 253, 5, 132, 69]);
        buf = buffer.from(array);
        strUrl = buf.toString("base64url");
        str = buf.toString("base64");
        arktest.assertEQ(String(strUrl), 'AsD-_QWERQ');
        arktest.assertEQ(str, "AsD+/QWERQ==");

        array = new Uint8Array([215, 109, 211, 97, 72, 142, 167, 241]);
        buf = buffer.from(array);
        strUrl = buf.toString("base64url");
        str = buf.toString("base64");
        arktest.assertEQ(strUrl, "123TYUiOp_E");
        arktest.assertEQ(str, "123TYUiOp/E=");

        array = new Uint8Array([252]);
        buf = buffer.from(array);
        strUrl = buf.toString("base64url");
        str = buf.toString("base64");
        arktest.assertEQ(strUrl, "_A");
        arktest.assertEQ(str, "/A==");

        array = new Uint8Array([252, 97]);
        buf = buffer.from(array);
        strUrl = buf.toString("base64url");
        str = buf.toString("base64");
        arktest.assertEQ(strUrl, "_GE");
        arktest.assertEQ(str, "/GE=");

        array = new Uint8Array([252, 97, 142]);
        buf = buffer.from(array);
        strUrl = buf.toString("base64url");
        str = buf.toString("base64");
        arktest.assertEQ(strUrl, "_GGO");
        arktest.assertEQ(str, "/GGO");
    }
}

function testWriteUInt8Error() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeUInt8(0x0304, 1);}, 'The value of "value" is out of range. It must be >= 0 and <= 255. Received value is: 772');
}

function testWriteInt32BEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeInt32BE(3405691582, 0);}, 'The value of "value" is out of range. It must be >= -2147483648 and <= 2147483647. Received value is: 3405691582');
}

function testWriteInt32LEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeInt32LE(3405691582, 0);}, 'The value of "value" is out of range. It must be >= -2147483648 and <= 2147483647. Received value is: 3405691582');
}

function testWriteInt16LEError() {
    let buf = buffer.alloc(5);
    arktest.expectError(() => {buf.writeInt16LE(48879, 0);}, 'The value of "value" is out of range. It must be >= -32768 and <= 32767. Received value is: 48879');
}

function testWriteInt16BEError() {
    let buf = buffer.alloc(5);
    arktest.expectError(() => {buf.writeInt16BE(48879, 0);}, 'The value of "value" is out of range. It must be >= -32768 and <= 32767. Received value is: 48879');
}

function testWriteInt8Error() {
    let buf = buffer.alloc(5);
    arktest.expectError(() => {buf.writeInt8(0x0304, 0);}, 'The value of "value" is out of range. It must be >= -128 and <= 127. Received value is: 772');
}

function testWriteUInt32BEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeUInt32BE(-2, 0);}, 'The value of "value" is out of range. It must be >= 0 and <= 4294967295. Received value is: -2');
}

function testWriteUInt32LEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeUInt32LE(-2, 0);}, 'The value of "value" is out of range. It must be >= 0 and <= 4294967295. Received value is: -2');
}

function testWriteUInt16BEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeUInt16BE(-2, 0);}, 'The value of "value" is out of range. It must be >= 0 and <= 65535. Received value is: -2');
}

function testWriteUInt16LEError() {
    let buf = buffer.allocUninitializedFromPool(4);
    arktest.expectError(() => {buf.writeUInt16LE(-2, 0);}, 'The value of "value" is out of range. It must be >= 0 and <= 65535. Received value is: -2');
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Buffer API tests")
    suite.addTest("Test buffer toString", testToString)
    suite.addTest("Test buffer WriteUInt8 throw value error", testWriteUInt8Error)
    suite.addTest("Test buffer WriteInt32BE throw value error", testWriteInt32BEError)
    suite.addTest("Test buffer WriteInt32LE throw value error", testWriteInt32LEError)
    suite.addTest("Test buffer WriteInt16LE throw value error", testWriteInt16LEError)
    suite.addTest("Test buffer WriteInt16BE throw value error", testWriteInt16BEError)

    suite.addTest("Test buffer WriteInt8 throw value error", testWriteInt8Error)
    suite.addTest("Test buffer WriteUInt32BE throw value error", testWriteUInt32BEError)
    suite.addTest("Test buffer WriteUInt32LE throw value error", testWriteUInt32LEError)
    suite.addTest("Test buffer WriteUInt16BE throw value error", testWriteUInt16BEError)
    suite.addTest("Test buffer WriteUInt16LE throw value error", testWriteUInt16LEError)
    return suite.run()
}