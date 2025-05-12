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
        assertEQ(res, "YWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXo=");
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', 30); // '';
        assertEQ(str, '')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', 5, 3); // ''
        assertEQ(str, '');
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', -1); // abcdefghijklmnopqrstuvwxyz
        assertEQ(str, 'abcdefghijklmnopqrstuvwxyz');
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', Infinity); // ''
        assertEQ(str, '');
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', -Infinity); // abcdefghijklmnopqrstuvwxyz
        assertEQ(str, 'abcdefghijklmnopqrstuvwxyz')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', undefined, Infinity) // abcdefghijklmnopqrstuvwxyz
        assertEQ(str, 'abcdefghijklmnopqrstuvwxyz')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', 0, -Infinity); // ''
        assertEQ(str, '')
    }
    {
        let buf = buffer.allocUninitializedFromPool(26);
        for (let i = 0; i < 26; i++) {
            buf.writeInt8(i + 97, i);
        }
        const str = buf.toString('utf8', undefined, -1); // ''
        assertEQ(str, '')
    }
    {
        const buf = buffer.from('hello');
        const res = buf.toString('base64', 0, -1); // ''
        assertEQ(res, '')
    }
    {
        const buf = buffer.from('hello');
        const res = buf.toString('base64', 0, Infinity); // aGVsbG8=
        assertEQ(res, 'aGVsbG8=')
    }
    {
        const buf = buffer.from('hello');
        const res = buf.toString('base64', 0, -Infinity); // ''
        assertEQ(res, '')
    }
}

function main(): int {
    const suite = new ArkTestsuite("Buffer API tests")
    suite.addTest("Test buffer toString", testToString)
    return suite.run()
}