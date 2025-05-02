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

import { util } from "@ohos.util";

function main() {
    const suite = new ArkTestsuite("StringDecoder API encoding utf-8 tests");

    suite.addTest("Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite001);
    suite.addTest("Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite002);
    suite.addTest("Returns any remaining input stored in the internal buffer as a string.", testend002);
    suite.addTest("Returns any remaining input stored in the internal buffer as a string.", testend003);
    suite.addTest("Returns any remaining input stored in the internal buffer as a string.", testend004);
    return suite.run();
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0001
 * @tc.name: testwrite001
 * @tc.desc:  Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite001() {
    let decoder = new util.StringDecoder('utf-8');
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const decoded = decoder.write(input);
    assertEQ(decoded, '你好');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0002
 * @tc.name: testwrite002
 * @tc.desc: Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite002() {
    let decoder = new util.StringDecoder('utf-8');
    const decoded = decoder.write("abc");
    assertEQ(decoded, 'abc');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0004
 * @tc.name: testend002
 * @tc.desc: Returns any remaining input stored in the internal buffer as a string.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testend002() {
    let decoder = new util.StringDecoder('utf-8');
    const decoded = decoder.end("abc");
    assertEQ(decoded, 'abc');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0005
 * @tc.name: testend003
 * @tc.desc: Returns any remaining input stored in the internal buffer as a string.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testend003() {
    let decoder = new util.StringDecoder('utf-8');
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const decoded = decoder.end(input);
    assertEQ(decoded, '你好');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_01006
 * @tc.name: testend004
 * @tc.desc: Returns any remaining input stored in the internal buffer as a string.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testend004() {
    let decoder = new util.StringDecoder('utf-8');
    let input = new Uint8Array([0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD]);
    const decoded = decoder.write(input.slice(0, 5));
    const decodeend = decoder.end(input.slice(5));
    assertEQ(decoded, '你');
    assertEQ(decodeend, '好');
}
