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

import {util} from "@ohos.util";

function NumberEqual(resStr:number, expect:number)
{
    return resStr === expect;
}

function main() {
    const suite = new ArkTestsuite("TextDecoder API tests");
    
    suite.addTest("test_utf8_hello", test_utf8_hello);
    suite.addTest("test_utf8_hello_prefix", test_utf8_hello_prefix);
    suite.addTest("test_utf8_hello_ignoreBOM", test_utf8_hello_ignoreBOM);
    suite.addTest("test_utf8_hello_ignoreBOM_prefix", test_utf8_hello_ignoreBOM_prefix);
    suite.addTest("test_utf8_hello_strip", test_utf8_hello_strip);
    suite.addTest("test_utf8_empty", test_utf8_empty);
    suite.addTest("test_decodeToString_empty_para", test_decodeToString_empty_para);
    suite.addTest("test_utf8_encoding", test_utf8_encoding);
    suite.addTest("test_utf8_options", test_utf8_options);
    suite.addTest("test_utf8_ignoreBOM", test_utf8_ignoreBOM);
    suite.addTest("test_utf16be_encoding", test_utf16be_encoding);
    suite.addTest("test_utf16le_ignoreBOM", test_utf16le_ignoreBOM);
    return suite.run()
}

function test_utf8_hello_ignoreBOM_prefix()
{
    let textDecoder = util.TextDecoder.create('utf-8', {ignoreBOM:true});
    let uint8 = new Uint8Array([0xEF,0xBB,0xBF,228,189,160,0,0,229,165,189,0]);
    let resStr = textDecoder.decodeToString(uint8);
    let expect = '你\u0000\u0000好\u0000';
    assertEQ(resStr, expect);
}

function test_utf8_hello_prefix()
{
    let textDecoder = util.TextDecoder.create('utf-8', {ignoreBOM:false});
    let uint8 = new Uint8Array([0xEF,0xBB,0xBF,228,189,160,0,0,229,165,189,0]);
    let resStr = textDecoder.decodeToString(uint8);
    let expect = '\uFEFF你\u0000\u0000好\u0000';
    assertEQ(resStr, expect);
    assertEQ(resStr.length, 6);
}

function test_utf8_hello()
{
    let textDecoder = util.TextDecoder.create('utf-8', {ignoreBOM:false});
    let uint8 = new Uint8Array([228,189,160,0,0,229,165,189,0]);
    let resStr = textDecoder.decodeToString(uint8);
    let expect = '你\u0000\u0000好\u0000';
    assertEQ(resStr, expect);
    assertEQ(resStr.length, 5);
}

function test_utf8_hello_ignoreBOM()
{
    let textDecoder = util.TextDecoder.create('utf-8', {ignoreBOM:true});
    let uint8 = new Uint8Array([228,189,160,0,0,229,165,189,0]);
    let resStr = textDecoder.decodeToString(uint8);
    let expect = '你\u0000\u0000好\u0000';
    assertEQ(resStr, expect);
    assertEQ(resStr.length, 5);
}

function test_utf8_hello_strip()
{
    let textDecoder = util.TextDecoder.create('utf-8', {ignoreBOM:false});
    let uint8 = new Uint8Array([228,189,160,229]);
    let retStr = textDecoder.decodeToString(uint8, {stream: true});
    let expect = '你';
    assertEQ(retStr, expect);
    let uint8arr = new Uint8Array([165,189]);
    let resStr = textDecoder.decodeToString(uint8arr, {stream: true});
    let expe = '好'
    assertEQ(resStr, expe);
}

function test_utf8_empty()
{
    let textDecoder = util.TextDecoder.create('utf-8', {ignoreBOM:false});
    let uint8 = new Uint8Array();
    let resStr = textDecoder.decodeToString(uint8);
    let expect = ''; 
    assertEQ(resStr, expect);
}

function test_decodeToString_empty_para()
{
    let textDecode = util.TextDecoder.create('utf-8');
    let uint8 = new Uint8Array([0xE4, 0xBD, 0xA0]);
    let resStr = textDecode.decodeToString(uint8, {});
    let expect = '你';
    assertEQ(resStr, expect);
}

function test_utf8_encoding()
{
    let textDecoderOptions : util.TextDecoderOptions = {
        fatal: false,
        ignoreBOM : true
    };
    let textDecode = util.TextDecoder.create('utf-8', textDecoderOptions);
    let retStr = textDecode.encoding;
    assertEQ(retStr, 'utf-8');
}

function test_utf8_options()
{
    let textDecoderOptions : util.TextDecoderOptions = {
        fatal: false,
        ignoreBOM : false
    };
    let textDecode = util.TextDecoder.create(undefined, textDecoderOptions);
    let encodingStr = textDecode.encoding;
    assertEQ(encodingStr, 'utf-8');
}

function test_utf8_ignoreBOM()
{
    let textDecoderOptions : util.TextDecoderOptions = {
        fatal: false,
        ignoreBOM : true
    };
    let textDecode = util.TextDecoder.create(undefined, textDecoderOptions);
    let encodingStr = textDecode.encoding;
    assertEQ(encodingStr, 'utf-8');
}

function test_utf16be_encoding()
{
    let textDecoderOptions : util.TextDecoderOptions = {
        fatal: false,
        ignoreBOM : false
    };
    let textDecode = util.TextDecoder.create('utf-16be', textDecoderOptions);
    let encodingStr = textDecode.encoding;
    assertEQ(encodingStr, 'utf-16be');
}

function test_utf16le_ignoreBOM()
{
    let textDecoderOptions : util.TextDecoderOptions = {
        fatal: false,
        ignoreBOM : false
    };
    let decodeWithStreamOptions : util.DecodeToStringOptions = {
        stream: false
    };
    let textDecode = util.TextDecoder.create('utf-16le', textDecoderOptions);
    let arr = new Uint8Array(8);
    arr[0] = 0xFF;
    arr[1] = 0xFE;
    arr[2] = 0x61;
    arr[3] = 0x00;
    arr[4] = 0x62;
    arr[5] = 0x00;
    arr[6] = 0x63;
    arr[7] = 0x00;
    let retStr = textDecode.decodeToString(arr, decodeWithStreamOptions);
    let BOM = '\uFEFF';
    let rel = 'abc';
    let re = BOM + rel;
    assertEQ(retStr, re);
}
