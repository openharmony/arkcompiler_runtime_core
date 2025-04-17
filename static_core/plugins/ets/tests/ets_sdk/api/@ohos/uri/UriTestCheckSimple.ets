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
 
function testCheckIsAbsolute001() {
  let gaogao = new uri.URI('f/tp://username:password@www.baidu.com:88/path?query#fagment');
  let res = gaogao.checkIsAbsolute();
  assertFalse(res);
}

function testCheckIsAbsolute002() {
  let gaogao = new uri.URI('ftp://username:password@www.baidu.com:88/path?query#fagment');
  let res = gaogao.checkIsAbsolute();
  assertTrue(res);
}

function testCheckIsAbsolute003() {
  let gaogao = new uri.URI('htt/p://username:password@www.baidu.com:88/path?query#fagment');
  let res = gaogao.checkIsAbsolute();
  assertFalse(res);
}

function testCheckIsAbsolute004() {
  let gaogao = new uri.URI('https://username:password@www.baidu.com:88/path?query#fagment');
  let res = gaogao.checkIsAbsolute();
  assertTrue(res);
}

function testCheckIsAbsolute005() {
  let gaogao = new uri.URI('http1://username:password@www.baidu.com:88/path?query#fagment');
  let res = gaogao.checkIsAbsolute();
  assertTrue(res);
}

function testCheckRelative001() {
  let route = new uri.URI('http://www.test.com/images/pic.jpg');
  let res = route.checkRelative();
  assertFalse(res);
}

function testCheckRelative002() {
  let route = new uri.URI('/images/pic.jpg');
  let res = route.checkRelative();
  assertTrue(res);
}

function testCheckRelative003() {
  let route = new uri.URI('https://www.test.com/images/logo.jpg');
  let res = route.checkRelative();
  assertFalse(res);
}

function testCheckRelative004() {
  let route = new uri.URI('aaa');
  let res = route.checkRelative();
  assertTrue(res);
}

function testCheckRelative005() {
  let mailtoRoute = new uri.URI('mailto:user@test.com');
  assertFalse(mailtoRoute.checkRelative());
}

function testCheckOpaque001() {
  let route = new uri.URI('http://www.test.com/images/pic.jpg');
  let res = route.checkOpaque();
  assertFalse(res);
}

function testCheckOpaque002() {
  let route = new uri.URI('images/pic.jpg');
  let res = route.checkOpaque();
  assertFalse(res);
}

function testCheckOpaque003() {
  let route = new uri.URI('mailto:user@test.com');
  assertTrue(route.checkOpaque());
}

function testCheckOpaque004() {
  let route = new uri.URI('tel:1234567890');
  let res = route.checkOpaque();
  assertTrue(res);
}

function testCheckHierarchical001() {
  let route = new uri.URI('http://www.test.com/images/pic.jpg');
  let res = route.checkHierarchical();
  assertTrue(res);
}

function testCheckHierarchical002() {
  let route = new uri.URI('mailto:test@test.com');
  let res = route.checkHierarchical();
  assertFalse(res);
}

function testCheckHierarchical003() {
  let route = new uri.URI('images/pic.jpg');
  let res = route.checkHierarchical();
  assertTrue(res);
}

function testCheckHierarchical004() {
  let route = new uri.URI('path');
  assertTrue(route.checkHierarchical());
  let testUri = new uri.URI('path:');
  assertFalse(testUri.checkHierarchical());
}

function main(): int {
  let suite = new ArkTestsuite("URI XTS tests");
  
  suite.addTest("testCheckIsAbsolute001", testCheckIsAbsolute001);
  suite.addTest("testCheckIsAbsolute002", testCheckIsAbsolute002);
  suite.addTest("testCheckIsAbsolute003", testCheckIsAbsolute003);
  suite.addTest("testCheckIsAbsolute004", testCheckIsAbsolute004);
  suite.addTest("testCheckIsAbsolute005", testCheckIsAbsolute005);

  suite.addTest("testCheckRelative001", testCheckRelative001);
  suite.addTest("testCheckRelative002", testCheckRelative002);
  suite.addTest("testCheckRelative003", testCheckRelative003);
  suite.addTest("testCheckRelative004", testCheckRelative004);
  suite.addTest("testCheckRelative005", testCheckRelative005);

  suite.addTest("testCheckOpaque001", testCheckOpaque001);
  suite.addTest("testCheckOpaque002", testCheckOpaque002);
  suite.addTest("testCheckOpaque003", testCheckOpaque003);
  suite.addTest("testCheckOpaque004", testCheckOpaque004);

  suite.addTest("testCheckHierarchical001", testCheckHierarchical001);
  suite.addTest("testCheckHierarchical002", testCheckHierarchical002);
  suite.addTest("testCheckHierarchical003", testCheckHierarchical003);
  suite.addTest("testCheckHierarchical004", testCheckHierarchical004);


  //passed
  return suite.run();
}