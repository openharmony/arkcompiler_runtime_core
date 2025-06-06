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
import { BusinessError } from "@ohos.base";

function testUriGetLastSegment001() {
  let route = new uri.URI('content://com.test.uri/files/image.jpg');
  let lastSegment = route.getLastSegment();
  assertEQ(lastSegment, "image.jpg");
}

function testUriGetLastSegment002() {
  let route = new uri.URI('content://com.test.uri/');
  let lastSegment = route.getLastSegment();
  assertEQ(lastSegment, "");
}

function testUriGetLastSegment003() {
  let route = new uri.URI('content://com.test.uri');
  let lastSegment = route.getLastSegment();
  assertEQ(lastSegment, "");
}

function testUriGetLastSegment004() {
  let route = new uri.URI('content://com.test.uri/files/');
  let lastSegment = route.getLastSegment();
  assertEQ(lastSegment, "files");
}

function testUriGetLastSegment005() {
  let route = new uri.URI('content://com.test.uri/files/my%20file.jpg');
  let lastSegment = route.getLastSegment();
  assertEQ(lastSegment, "my file.jpg");
}

function testUriGetLastSegment006() {
  let route = new uri.URI('content://com.test.uri/files/my+file.jpg');
  let lastSegment = route.getLastSegment();
  assertEQ(lastSegment, "my+file.jpg");
}

function testUriGetLastSegment007() {
  try {
    let route = new uri.URI('content://com.test.uri/files/a a');
    let segments = route.getSegment();
  } catch (err: BusinessError) {
    assertEQ(err.toString(), "BusinessError: Syntax Error. Invalid Uri string.");
    assertEQ(err.code, 10200002);
    assertEQ(err.message, "Syntax Error. Invalid Uri string.");
  }
}

function testUriGetSegment001() {
  let route = new uri.URI('content://com.test.uri/files/images');
  let segments = route.getSegment();
  assertEQ(segments.length, 2);
  assertEQ(segments[0], "files");
  assertEQ(segments[1], "images");
}

function testUriGetSegment002() {
  let route = new uri.URI('content://com.test.uri/');
  let segments = route.getSegment();
  assertEQ(segments.length, 0);
}

function testUriGetSegment003() {
  let route = new uri.URI('content://com.test.uri/files/');
  let segments = route.getSegment();
  assertEQ(segments.length, 1);
}

function testUriGetSegment004() {
  let route = new uri.URI('content://com.test.uri');
  let segments = route.getSegment();
  assertEQ(segments.length, 0);
  let mailRoute = new uri.URI('mailto:people');
  assertEQ(mailRoute.getSegment().length, 0);
}

function testUriGetSegment005() {
  let route = new uri.URI('content://com.test.uri/files/a%20a/b%20b');
  let segments = route.getSegment();
  assertEQ(segments[0], "files");
  assertEQ(segments[1], "a a");
  assertEQ(segments[2], "b b");
}

function testUriGetSegment006() {
  let route = new uri.URI('http://www.test.com/path/to/image.jpg');
  let segments = route.getSegment();
  assertEQ(segments[2], "image.jpg");
}

function testUriAddSegment001() {
  let route = new uri.URI('http://www.test.com/');
  let newRoute = route.addSegment("files").addSegment("image.jpg");
  assertEQ(newRoute.toString(), "http://www.test.com/files/image.jpg");
  assertEQ(newRoute.getLastSegment(), "image.jpg");
}

function testUriAddSegment002() {
  let route1 = new uri.URI('http://www.test.com');
  let newRoute1 = route1.addSegment("files").addSegment("my image.jpg");
  assertEQ(newRoute1.toString(), "http://www.test.com/files/my%20image.jpg");
  let route2 = new uri.URI('http://www.test.com');
  let newRoute2 = route2.addSegment("files").addSegment("my+image.jpg");
  assertEQ(newRoute2.toString(), "http://www.test.com/files/my%2Bimage.jpg");
  let route3 = new uri.URI('http://www.test.com');
  let newRoute3 = route3.addSegment("files").addSegment("my%20image.jpg");
  assertEQ(newRoute3.toString(), "http://www.test.com/files/my%2520image.jpg");
}

function testUriAddSegment003() {
  let route = new uri.URI('http://www.test.com');
  let newRoute = route.addSegment("files");
  assertEQ(newRoute.getLastSegment(), "files");
  assertEQ(newRoute.getSegment().length, 1);
}

function testUriAddEncodedSegment001() {
  let route = new uri.URI('http://www.test.com');
  let newRoute = route.addEncodedSegment("files").addEncodedSegment("image.jpg");
  assertEQ(newRoute.getSegment()[1], "image.jpg");
}

function testUriAddEncodedSegment002() {
  let route1 = new uri.URI('http://www.test.com?param%2B1=value1&param+2=value%202#fra+gemnt');
  let newRoute1 = route1.addSegment("my+files").addEncodedSegment("my+image.jpg");
  assertEQ(newRoute1.toString(), "http://www.test.com/my%2Bfiles/my+image.jpg?param%2B1=value1&param+2=value%202#fra+gemnt");
  assertEQ(newRoute1.encodedUserInfo, null);
  assertEQ(newRoute1.encodedPath, "/my%2Bfiles/my+image.jpg");
  assertEQ(newRoute1.encodedQuery, 'param%2B1=value1&param+2=value%202');
  assertEQ(newRoute1.encodedFragment, "fra+gemnt");
  assertEQ(newRoute1.encodedAuthority, "www.test.com");
  assertEQ(newRoute1.encodedSSP, "//www.test.com/my%2Bfiles/my+image.jpg?param%2B1=value1&param+2=value%202");
  let route2 = new uri.URI('http://www.test.com/');
  let newRoute2 = route2.addSegment("my files").addEncodedSegment("my%20image.jpg");
  assertEQ(newRoute2.toString(), "http://www.test.com/my%20files/my%20image.jpg");
  assertEQ(newRoute2.encodedUserInfo, null);
  assertEQ(newRoute2.encodedPath, "/my%20files/my%20image.jpg");
  assertEQ(newRoute2.encodedQuery, null);
  assertEQ(newRoute2.encodedFragment, null);
  assertEQ(newRoute2.encodedAuthority, "www.test.com");
  assertEQ(newRoute2.encodedSSP, "//www.test.com/my%20files/my%20image.jpg");
  let route3 = new uri.URI('http://username:my%20name@www.test.com/');
  let newRoute3 = route3.addSegment("my%2Bfiles").addEncodedSegment("my%2Bimage.jpg");
  assertEQ(newRoute3.toString(), "http://username:my%20name@www.test.com/my%252Bfiles/my%2Bimage.jpg");
  assertEQ(newRoute3.encodedUserInfo, "username:my%20name");
  assertEQ(newRoute3.encodedPath, "/my%252Bfiles/my%2Bimage.jpg");
  assertEQ(newRoute3.encodedQuery, null);
  assertEQ(newRoute3.encodedFragment, null);
  assertEQ(newRoute3.encodedAuthority, "username:my%20name@www.test.com");
  assertEQ(newRoute3.encodedSSP, "//username:my%20name@www.test.com/my%252Bfiles/my%2Bimage.jpg");
}

function testUriAddEncodedSegment003() {
  try {
    let route = new uri.URI('http://www.test.com/');
    let newRoute = route.addEncodedSegment("my files");
  } catch (err: BusinessError) {
    assertEQ(err.toString(), "BusinessError: Syntax Error. Invalid Uri string.");
    assertEQ(err.code, 10200002);
    assertEQ(err.message, "Syntax Error. Invalid Uri string.");
  }
}

function testUriAddEncodedSegment004() {
  try {
    let route = new uri.URI('http://www.test.com/');
    let newRoute = route.addEncodedSegment("my`files");
  } catch (err: BusinessError) {
    assertEQ(err.toString(), "BusinessError: Syntax Error. Invalid Uri string: The Path does not conform to the rule.");
    assertEQ(err.code, 10200002);
    assertEQ(err.message, "Syntax Error. Invalid Uri string: The Path does not conform to the rule.");
  }
}

function main(): int {
  let suite = new ArkTestsuite("URI XTS tests");
  suite.addTest("testUriGetLastSegment001", testUriGetLastSegment001);
  suite.addTest("testUriGetLastSegment002", testUriGetLastSegment002);
  suite.addTest("testUriGetLastSegment003", testUriGetLastSegment003);
  suite.addTest("testUriGetLastSegment004", testUriGetLastSegment004);
  suite.addTest("testUriGetLastSegment005", testUriGetLastSegment005);
  suite.addTest("testUriGetLastSegment006", testUriGetLastSegment006);
  suite.addTest("testUriGetLastSegment007", testUriGetLastSegment007);

  suite.addTest("testUriGetSegment001", testUriGetSegment001);
  suite.addTest("testUriGetSegment002", testUriGetSegment002);
  suite.addTest("testUriGetSegment003", testUriGetSegment003);
  suite.addTest("testUriGetSegment004", testUriGetSegment004);
  suite.addTest("testUriGetSegment005", testUriGetSegment005);
  suite.addTest("testUriGetSegment006", testUriGetSegment006);

  suite.addTest("testUriAddSegment001", testUriAddSegment001);
  suite.addTest("testUriAddSegment002", testUriAddSegment002);
  suite.addTest("testUriAddSegment003", testUriAddSegment003);

  suite.addTest("testUriAddEncodedSegment001", testUriAddEncodedSegment001);
  suite.addTest("testUriAddEncodedSegment002", testUriAddEncodedSegment002);
  suite.addTest("testUriAddEncodedSegment003", testUriAddEncodedSegment003);

  suite.addTest("testUriAddEncodedSegment004", testUriAddEncodedSegment004);
  
  //passed
  return suite.run();
}