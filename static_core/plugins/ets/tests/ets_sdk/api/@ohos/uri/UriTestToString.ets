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
import {uri} from "@ohos.uri";

function testNormalize005() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/./path/./aa/bb/cc?query#fagment');
  let res = gaogao.normalize();
  assertEQ(res.path, "/path/aa/bb/cc");
  assertEQ(res.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path/aa/bb/cc?query#fagment");
}


function testToString001() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path/.././../aa/bb/cc?query#fagment');
  let res = gaogao.toString();
  assertEQ(res, "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path/.././../aa/bb/cc?query#fagment");
}

function testToString002() {
  let gaogao = new uri.URI('htt1p://gg:gaogao@[::192.9.5.5]:99/path/66path1?query#fagment');
  let res = gaogao.toString();
  assertEQ(res, "htt1p://gg:gaogao@[::192.9.5.5]:99/path/66path1?query#fagment");
}

function testToString003() {
  let gaogao = new uri.URI('ftp://username:password@www.baidu.com:88/path?query#fagment');
  let res = gaogao.toString();
  assertEQ(res, "ftp://username:password@www.baidu.com:88/path?query#fagment");
}

function testToString004() {
  let gaogao = new uri.URI('http://gg:gao你好gao@199.98.55.44:99/path/p你好ath?qu你好ery#fag你好ment');
  let res = gaogao.toString();
  assertEQ(res, 'http://gg:gao%E4%BD%A0%E5%A5%BDgao@199.98.55.44:99/path/' +
        'p%E4%BD%A0%E5%A5%BDath?qu%E4%BD%A0%E5%A5%BDery#fag%E4%BD%A0%E5%A5%BDment');
}

function testToString005() {
  let gaogao = new uri.URI('http://gg:gaogao@199.98.55.44:99/path/path?query#fagment');
  let res = gaogao.toString();
  assertEQ(res, "http://gg:gaogao@199.98.55.44:99/path/path?query#fagment");
}
 

function ToStringTest001() {
  let uri = new uri.UriEntry("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/aa/bb/cc?query#fagment");
  assertEQ(uri.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/aa/bb/cc?query#fagment");
}

function ToStringTest002() {
  let uri = new uri.UriEntry("htt1p://gg:gaogao@[::192.9.5.5]:99/path/66path1?query#fagment");
  assertEQ(uri.toString(), "htt1p://gg:gaogao@[::192.9.5.5]:99/path/66path1?query#fagment");
}

function ToStringTest003() {
  let uri = new uri.UriEntry("ftp://username:www.baidu.com/path?query#fagment");
  assertEQ(uri.toString(), "ftp://username:www.baidu.com/path?query#fagment");
}

function main(): int {
  let suite = new ArkTestsuite("URI XTS tests");
  
  suite.addTest("testNormalize005", testNormalize005);
  
  suite.addTest("testToString001", testToString001);
  suite.addTest("testToString002", testToString002);
  suite.addTest("testToString003", testToString003);
  suite.addTest("testToString004", testToString004);
  suite.addTest("testToString005", testToString005);
  suite.addTest("ToStringTest001", ToStringTest001);
  suite.addTest("ToStringTest002", ToStringTest002);
  suite.addTest("ToStringTest003", ToStringTest003);
  //passed
  return suite.run();
}