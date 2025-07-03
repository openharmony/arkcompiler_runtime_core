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

function main(): int {
    const suite = new ArkTestsuite("StringDecoder API tests");

    suite.addTest(" Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite003);
    suite.addTest(" Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite004);
    suite.addTest(" Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite005);
    suite.addTest("Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are", testwrite006);
    suite.addTest("Returns any remaining input stored in the internal buffer as a string.", testend001);
    suite.addTest("Test ENCODER and DOCODER.", test_Decode_Test_001)
    suite.addTest("Test ENCODER and DOCODER.", test_Decode_Test_002)
    suite.addTest("Test ENCODER and DOCODER.", test_Decode_Test_003)
    suite.addTest("Test ENCODER and DOCODER.", testArrayScenarios0600)
    suite.addTest("Test ENCODER and DOCODER.", testArrayScenarios0700)

    return suite.run();
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_00013
 * @tc.name: testwrite003
 * @tc.desc:  Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite003() {
    let decoder = new util.StringDecoder('big5');
    let input = new Uint8Array([167, 65, 166, 110]);
    const decoded = decoder.write(input);
    assertEQ(decoded, '你好');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0012
 * @tc.name: testwrite004
 * @tc.desc:  Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite004() {
    let decoder = new util.StringDecoder('iso-8859-2');
    let input = new Uint8Array([0X61, 0X62, 0X63]);
    const decoded = decoder.write(input);
    assertEQ(decoded, 'abc');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0010
 * @tc.name: testwrite005
 * @tc.desc:  Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite005() {
    let decoder = new util.StringDecoder('koi8-r');
    let input = new Uint8Array([97, 98, 99]);
    const decoded = decoder.write(input);
    assertEQ(decoded, 'abc');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0011
 * @tc.name: testwrite006
 * @tc.desc: Returns a decoded string, ensuring that any incomplete multibyte characters at the end of the Uint8Array are
 * comitted from the returned string and stored in an internal buffer.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testwrite006() {
    let decoder = new util.StringDecoder('windows-1250');
    let input = new Uint8Array([0X61, 0X62, 0X63]);
    const decoded = decoder.write(input);
    assertEQ(decoded, 'abc');
}

/**
 * @tc.number: SUB_COMMONLIBRARY_ETSUTILS_UTIL_0003
 * @tc.name: testend001
 * @tc.desc: Returns any remaining input stored in the internal buffer as a string.
 * @tc.size: MediumTest
 * @tc.type: Function
 * @tc.level: Level 2
 */
function testend001() {
    let decoder = new util.StringDecoder('macintosh');
    let input = new Uint8Array([0X61]);
    const decoded = decoder.end(input);
    assertEQ(decoded, 'a');
}


/**
 * @tc.number     : SUB_COMMONLIBRARY_UTIL_BASE_STRINGDECODER_0100
 * @tc.name       : test_Decode_Test_001
 * @tc.desc       : Test ENCODER and DOCODER.
 * @tc.size       : MediumTest
 * @tc.type       : Function
 * @tc.level      : Level 0
 */
function test_Decode_Test_001() {
    const knownEncodings = ['utf-8', 'ibm866', 'iso-8859-2', 'iso-8859-3', 'iso-8859-4', 'iso-8859-5', 'iso-8859-6',
        'iso-8859-7', 'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13', 'iso-8859-14', 'iso-8859-15',
        'koi8-r', 'koi8-u', 'macintosh', 'windows-874', 'windows-1250', 'windows-1251', 'windows-1252', 'windows-1253',
        'windows-1254', 'windows-1255', 'windows-1256', 'windows-1257', 'windows-1258', 'x-mac-cyrillic', 'gbk',
        'gb18030', 'big5', 'euc-jp', 'iso-2022-jp', 'shift_jis', 'euc-kr'];

    for(let item of knownEncodings) {
        let decoder = new util.StringDecoder(item)
        let input1 = new Uint8Array([72, 101, 108, 108, 111, 87, 111, 114, 108, 100, 49, 50, 51, 63])
        let input2 = 'HelloWorld123?HelloWorld123?'
        const decoded1 = decoder.write(input1)
        const decoded2 = decoder.write(input2)
        if (item === 'utf-16be') {
            let input2 = 'HelloWorld123?HelloWorld123?'
        }
        assertEQ(decoded1, 'HelloWorld123?')
        assertEQ(decoded2, 'HelloWorld123?HelloWorld123?')

        let input3 = input1.slice(0, 5)
        let input33 = input1.slice(5)
        const decoded3 = decoder.write(input3)
        const decodedend3 = decoder.write(input33)
        assertEQ(decoded3, 'Hello')
        assertEQ(decodedend3, 'World123?')

        let input4 = input1.slice(0, 1)
        let input44 = input1.slice(1)
        const decoded4 = decoder.write(input4)
        const decodedend4 = decoder.write(input44)
        assertEQ(decoded4, 'H')
        assertEQ(decodedend4, 'elloWorld123?')
    }
}

/**
 * @tc.number     : SUB_COMMONLIBRARY_UTIL_BASE_STRINGDECODER_0200
 * @tc.name       : test_Decode_Test_002
 * @tc.desc       : Test ENCODER and DOCODER.
 * @tc.size       : MediumTest
 * @tc.type       : Function
 * @tc.level      : Level 0
 */
function test_Decode_Test_002() {
    let that = new util.TextEncoder('utf-16be')
    let result = that.encodeInto('HelLoWorLd123?')
    let decoder = new util.StringDecoder('utf-16be')
    const decoded1 = decoder.write(result)
    assertEQ(decoded1, 'HelLoWorLd123?')
}

/**
 * @tc.number     : SUB_COMMONLIBRARY_UTIL_BASE_STRINGDECODER_0300
 * @tc.name       : test_Decode_Test_003
 * @tc.desc       : Test ENCODER and DOCODER.
 * @tc.size       : MediumTest
 * @tc.type       : Function
 * @tc.level      : Level 1
 */
function test_Decode_Test_003() {
    let that = new util.TextEncoder('utf-16le')
    let result = that.encodeInto('HelLoWorLd123?')
    let decoder = new util.StringDecoder('utf-16le')
    const decoded1 = decoder.write(result)
    assertEQ(decoded1, 'HelLoWorLd123?')
}

/**
 * @tc.number     : SUB_COMMONLIBRARY_UTIL_BASE_STRINGDECODER_0900
 * @tc.name       : testArrayScenarios0600
 * @tc.desc       : Test ENCODER and DOCODER.
 * @tc.size       : MediumTest
 * @tc.type       : Function
 * @tc.level      : Level 1
 */
function testArrayScenarios0600() {
    const TAG = 'testArrayScenarios0600 == '
    let encodingArr =
    ['utf-8', 'UTF-8', 'GBK', 'GB2312', 'GB18030', 'gb18030', 'ibm866', 'iso-8859-1', 'iso-8859-2', 'iso-8859-3',
        'iso-8859-4', 'iso-8859-5', 'iso-8859-6', 'iso-8859-7',
        'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13', 'iso-8859-14', 'iso-8859-15', 'koi8-r', 'koi8-u',
        'macintosh', 'windows-874', 'windows-1250', 'windows-1251', 'windows-1252', 'windows-1253', 'windows-1254',
        'windows-1255', 'windows-1256', 'windows-1257', 'windows-1258', 'x-mac-cyrillic', 'gbk', 'big5',
        'euc-jp', 'iso-2022-jp', 'shift_jis', 'euc-kr']
    let uint8Array = new Uint8Array([0, 0, 0, 0, 0, 72, 101, 108, 108, 111, 87, 111, 114, 108, 100, 49, 50, 51, 63])

    for(let item of encodingArr) {
        let decoder = new util.StringDecoder(item)
        let decoded1 = decoder.write(uint8Array)
        let urlcode = encodeURI(decoded1)
        assertEQ(urlcode, '%00%00%00%00%00HelloWorld123?')
    }

    let decoder1 = new util.StringDecoder('utf-16be')
    let decodedBe = decoder1.write(uint8Array)
    let urlcodeBe = encodeURI(decodedBe)
    assertEQ(urlcodeBe, '%00%00H%E6%95%AC%E6%B1%AF%E5%9D%AF%E7%89%AC%E6%90%B1%E3%88%B3')

    let decoder2 = new util.StringDecoder('utf-16le')
    let decodedLe = decoder2.write(uint8Array)
    let urlcodeLe = encodeURI(decodedLe)
    assertEQ(urlcodeLe, '%00%00%E4%A0%80%E6%B1%A5%E6%BD%AC%E6%BD%97%E6%B1%B2%E3%85%A4%E3%8C%B2')
}

/**
 * @tc.number     : SUB_COMMONLIBRARY_UTIL_BASE_STRINGDECODER_1000
 * @tc.name       : testArrayScenarios0700
 * @tc.desc       : Test ENCODER and DOCODER.
 * @tc.size       : MediumTest
 * @tc.type       : Function
 * @tc.level      : Level 1
 */
function testArrayScenarios0700() {
    const TAG = 'testArrayScenarios0700 == '
    let encodingArr =
    ['utf-8', 'UTF-8', 'GBK', 'GB2312', 'GB18030', 'gb18030', 'ibm866', 'iso-8859-1', 'iso-8859-2', 'iso-8859-3',
        'iso-8859-4', 'iso-8859-5', 'iso-8859-6', 'iso-8859-7',
        'iso-8859-8', 'iso-8859-8-i', 'iso-8859-10', 'iso-8859-13', 'iso-8859-14', 'iso-8859-15', 'koi8-r', 'koi8-u',
        'macintosh', 'windows-874', 'windows-1250', 'windows-1251', 'windows-1252', 'windows-1253', 'windows-1254',
        'windows-1255', 'windows-1256', 'windows-1257', 'windows-1258', 'x-mac-cyrillic', 'gbk', 'big5',
        'euc-jp', 'iso-2022-jp', 'shift_jis', 'euc-kr']
    let uint8Array = new Uint8Array([72, 101, 108, 108, 111, 87, 111, 114, 108, 100, 49, 50, 51, 63, 0, 0, 0, 0, 0])

    for(let item of encodingArr) {
        let decoder = new util.StringDecoder(item)
        let decoded1 = decoder.end(uint8Array.slice(14))
        let urlcode = encodeURI(decoded1)
        assertEQ(urlcode, '%00%00%00%00%00')
    }

    let decoder1 = new util.StringDecoder('utf-16be')
    let decodedBe = decoder1.end(uint8Array.slice(14))
    let urlcodeBe = encodeURI(decodedBe)
    assertEQ(urlcodeBe, '%00%00%EF%BF%BD')

    let decoder2 = new util.StringDecoder('utf-16le')
    let decodedLe = decoder2.end(uint8Array.slice(14))
    let urlcodeLe = encodeURI(decodedLe)
    assertEQ(urlcodeLe, '%00%00%EF%BF%BD')
}
