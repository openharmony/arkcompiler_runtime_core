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

function main(): int {
    const suite = new arktest.ArkTestsuite("Buffer API tests")
    suite.addTest("Test buffer toString", testToString)
    return suite.run()
}