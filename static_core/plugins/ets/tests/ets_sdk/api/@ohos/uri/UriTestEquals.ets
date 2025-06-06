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
import { uri } from "@ohos.uri"

function testEquals001() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment');
  let gaogao1 = gaogao;
  let res = gaogao.equals(gaogao1);
  assertTrue(res);
}

function testEquals002() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment');
  let gaogao1 = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment');
  let res = gaogao.equals(gaogao1);
  assertTrue(res);
}

function testEquals003() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment');
  let gaogao1 = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment123');
  let res = gaogao.equals(gaogao1);
  assertFalse(res);
}

function testEquals004() {
  let gaogao = new uri.URI('http://gg:gaogao@wwwaaa:99/path1?query#fagment');
  let gaogao1 = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment123');
  let res = gaogao.equals(gaogao1);
  assertFalse(res);
}

function testEquals005() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment');
  let gaogao1 = new uri.URI('http://[1:0:0:1:2:1:2:1]/path1?query#fagment123');
  let res = gaogao.equals(gaogao1);
  assertFalse(res);
}

function testEqualsTo001() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment');
  let gaogao1 = gaogao;
  let res = gaogao.equals(gaogao1);
  assertTrue(res);
}

function testEqualsTo003() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment');
  let gaogao1 = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment123');
  let res = gaogao.equals(gaogao1);
  assertFalse(res);
}

function testEqualsTo004() {
  let gaogao = new uri.URI('http://gg:gaogao@wwwaaa:99/path1?query#fagment');
  let gaogao1 = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment123');
  let res = gaogao.equals(gaogao1);
  assertFalse(res);
}

function testEqualsTo005() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment');
  let gaogao1 = new uri.URI('http://[1:0:0:1:2:1:2:1]/path1?query#fagment123');
  let res = gaogao.equalsTo(gaogao1);
  assertFalse(res);
}

function EqualsTest001() {
  let uri = new uri.UriEntry("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment");
  let uri2 = uri;
  assertTrue(uri.equals(uri2));
}

function EqualsTest002() {
  let uri = new uri.UriEntry("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment");
  let uri2 = new uri.UriEntry("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment");
  assertTrue(uri.equals(uri2));
}

function EqualsTest003() {
  let uri = new uri.UriEntry("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment");
  let uri2 = new uri.UriEntry("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path1?query#fagment123");
  assertFalse(uri.equals(uri2));
}

function EqualsTest004() {
  let uri = new uri.UriEntry("http://username:password@www.baidu.com:99/path/path?query#fagment");
  let uri2 = new uri.UriEntry("http://username:password@www.baidu.com:29/path/path?query#fagment");
  assertFalse(uri.equals(uri2));
}

function EqualsTest005() {
  let uri = new uri.UriEntry("http://username:password@www.baidu.com:99/path/path?query#fagment");
  let uri2 = new uri.UriEntry("http://user1name:password@www.baidu.com:99/path/path?query#fagment");
  assertFalse(uri.equals(uri2));
}

function EqualsTest006() {
  let uri = new uri.UriEntry("http://username:password@www.baidu.com:99/path/path?query#fagment");
  let uri2 = new uri.UriEntry("http://username:password@w2ww.baidu.com:99/path/path?query#fagment");
  assertFalse(uri.equals(uri2));
}

function EqualsTest007() {
  let uri = new uri.UriEntry("http://username:password@www.baidu.com:99/path/path?query#fagment");
  let uri2 = new uri.UriEntry("http://username:password@www.baidu.com:99/pa4th/path?query#fagment");
  assertFalse(uri.equals(uri2));
}

function EqualsTest008() {
  let uri = new uri.UriEntry("http://username:password@www.baidu.com:99/path/path?qu4ery#fagment");
  let uri2 = new uri.UriEntry("http://username:password@www.baidu.com:99/path/path?query#fagment");
  assertFalse(uri.equals(uri2));
}

function EqualsTest009() {
  let uri = new uri.UriEntry("http://username:password@www.baidu.com:99/path/path?query#fagment");
  let uri2 = new uri.UriEntry("h4ttp://username:password@www.baidu.com:99/path/path?query#fagment");
  assertFalse(uri.equals(uri2));
}


  function main(): int {
  let suite = new ArkTestsuite("URI XTS tests");

  suite.addTest("testEquals001", testEquals001);
  suite.addTest("testEquals002", testEquals002);
  suite.addTest("testEquals003", testEquals003);
  suite.addTest("testEquals004", testEquals004);
  suite.addTest("testEquals005", testEquals005);


  suite.addTest("testEqualsTo001", testEqualsTo001);
  //    suite.addTest("testEqualsTo002", testEqualsTo002);
  suite.addTest("testEqualsTo003", testEqualsTo003);
  suite.addTest("testEqualsTo004", testEqualsTo004);
  suite.addTest("testEqualsTo005", testEqualsTo005);

  suite.addTest("EqualsTest001", EqualsTest001);
  suite.addTest("EqualsTest002", EqualsTest002);
  suite.addTest("EqualsTest003", EqualsTest003);
  suite.addTest("EqualsTest004", EqualsTest004);
  suite.addTest("EqualsTest005", EqualsTest005);
  suite.addTest("EqualsTest006", EqualsTest006);
  suite.addTest("EqualsTest007", EqualsTest007);
  suite.addTest("EqualsTest008", EqualsTest008);
  suite.addTest("EqualsTest009", EqualsTest009);
  //passed
  return suite.run();
}