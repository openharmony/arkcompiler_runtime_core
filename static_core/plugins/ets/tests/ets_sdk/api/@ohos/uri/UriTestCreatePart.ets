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

function testUriCreateFromParts001() {
  let route = uri.URI.createFromParts("http", "", "fragment");
  assertEQ(route.toString(), "http:#fragment");
}

function testUriCreateFromParts002() {
  let route = uri.URI.createFromParts("mailto", "no body", "top");
  assertEQ(route.toString(), "mailto:no%20body#top");
}

function testUriCreateFromParts003() {
  let route = uri.URI.createFromParts("foo", "", "");
  assertEQ(route.toString(), "foo:");
  try {
    uri.URI.createFromParts("", "", "");
  } catch (err: BusinessError) {
    assertEQ(err.toString(), "BusinessError: Syntax Error. Invalid Uri string: The Scheme the first character must be a letter.");
    assertEQ(err.code, 10200002);
    assertEQ(err.message, "Syntax Error. Invalid Uri string: The Scheme the first character must be a letter.");
  }
}

function testUriCreateFromParts004() {
  let scheme = "https";
  let ssp = "www.test.com";
  let fragment = "fragment";
  let route = uri.URI.createFromParts(scheme, ssp, fragment);
  assertEQ(route.toString(), "https:www.test.com#fragment");
}


function testTildeUnderline001() {
  let uriTest = new uri.URI('http://www.bian~_cheng.net/inde~_x?param=10~_!*&para~_m1=20!-~_');
  assertEQ(uriTest.host, 'www.bian~_cheng.net');
  assertEQ(uriTest.path, '/inde~_x');
  assertEQ(uriTest.query, 'param=10~_!*&para~_m1=20!-~_');
  assertEQ(uriTest.authority, 'www.bian~_cheng.net');
  assertEQ(uriTest.ssp, '//www.bian~_cheng.net/inde~_x?param=10~_!*&para~_m1=20!-~_');
  assertEQ(uriTest.encodedPath, '/inde~_x');
  assertEQ(uriTest.encodedQuery, 'param=10~_!*&para~_m1=20!-~_');
  assertEQ(uriTest.encodedAuthority, 'www.bian~_cheng.net');
  assertEQ(uriTest.encodedSSP, '//www.bian~_cheng.net/inde~_x?param=10~_!*&para~_m1=20!-~_');
}

function testTildeUnderline002() {
  let uriTest = new uri.URI('http://www.bian_cheng.net/inde_x?param=10_!*&para_m1=20!-_');
  assertEQ(uriTest.host, 'www.bian_cheng.net');
  assertEQ(uriTest.path, '/inde_x');
  assertEQ(uriTest.query, 'param=10_!*&para_m1=20!-_');
  assertEQ(uriTest.authority, 'www.bian_cheng.net');
  assertEQ(uriTest.ssp, '//www.bian_cheng.net/inde_x?param=10_!*&para_m1=20!-_');
  assertEQ(uriTest.encodedPath, '/inde_x');
  assertEQ(uriTest.encodedQuery, 'param=10_!*&para_m1=20!-_');
  assertEQ(uriTest.encodedAuthority, 'www.bian_cheng.net');
  assertEQ(uriTest.encodedSSP, '//www.bian_cheng.net/inde_x?param=10_!*&para_m1=20!-_');
}

function testTildeUnderline003() {
  let uriTest = new uri.URI('http://www.bian~cheng.net/inde~x?param=10~!*&para~m1=20!-~');
  assertEQ(uriTest.host, 'www.bian~cheng.net');
  assertEQ(uriTest.path, '/inde~x');
  assertEQ(uriTest.query, 'param=10~!*&para~m1=20!-~');
  assertEQ(uriTest.authority, 'www.bian~cheng.net');
  assertEQ(uriTest.ssp, '//www.bian~cheng.net/inde~x?param=10~!*&para~m1=20!-~');
  assertEQ(uriTest.encodedPath, '/inde~x');
  assertEQ(uriTest.encodedQuery, 'param=10~!*&para~m1=20!-~');
  assertEQ(uriTest.encodedAuthority, 'www.bian~cheng.net');
  assertEQ(uriTest.encodedSSP, '//www.bian~cheng.net/inde~x?param=10~!*&para~m1=20!-~');
}

function testAddQueryJson001() {
  let uriTest = new uri.URI('aaa://drive/page?need=false&bbb=true');
  let newUri = uriTest.addQueryValue("data", '{"type": 0, "from": "from"}');
  assertEQ(newUri.toString(), 'aaa://drive/page?need=false&bbb=true&data=%7B%22type%22%3A%200%2C%20%22from%22%3A%20%22from%22%7D');
  assertEQ(newUri.query, 'need=false&bbb=true&data={"type": 0, "from": "from"}');
  assertEQ(newUri.getQueryValue('data'), '{"type": 0, "from": "from"}');
  assertEQ(newUri.encodedQuery, 'need=false&bbb=true&data=%7B%22type%22%3A%200%2C%20%22from%22%3A%20%22from%22%7D');
}

function testSpecialCharacters001() {
  let uriStr = "hta{}_b- c|~d!a$b&c=d+e,;'()*vvtp://www.bi an'|{}()$~,;*!_-+:80/index?par(,;/) '_$:?+a{}|m=10-!*&pa ra~m1=20!-~#par(,;/) '_$:?+a{}|m=10-!*&para~m1=20!-~";
  let newUri = new uri.URI(uriStr);
  assertEQ(newUri.scheme, "hta{}_b- c|~d!a$b&c=d+e,;'()*vvtp");
  assertEQ(newUri.getSegment()[0], "index");
  assertEQ(newUri.query, "par(,;/) '_$:?+a{}|m=10-!*&pa ra~m1=20!-~");
  assertEQ(newUri.ssp, "//www.bi an'|{}()$~,;*!_-+:80/index?par(,;/) '_$:?+a{}|m=10-!*&pa ra~m1=20!-~");
  assertEQ(newUri.authority, "www.bi an'|{}()$~,;*!_-+:80");
  assertEQ(newUri.fragment, "par(,;/) '_$:?+a{}|m=10-!*&para~m1=20!-~");
  assertEQ(newUri.host, "www.bi an'|{}()$~,;*!_-+");
  assertEQ(newUri.userInfo, null);
  assertEQ(newUri.port, "80");
}

function testspecialPort001() {
  let uriStr = "http://publickinfo:-1";
  let newUri = new uri.URI(uriStr);
  assertEQ(newUri.toString(), "http://publickinfo:-1");
  assertEQ(newUri.port, "-1");
  assertEQ(newUri.host, null);
  assertEQ(newUri.authority, "publickinfo:-1");
}

function testspecialPort002() {
  let uriStr = "http://publickinfo:2147483647";
  let newUri = new uri.URI(uriStr);
  assertEQ(newUri.toString(), "http://publickinfo:2147483647");
  assertEQ(newUri.port, "2147483647");
  assertEQ(newUri.host, 'publickinfo');
  assertEQ(newUri.authority, "publickinfo:2147483647");
}

function testspecialPort003() {
  let uriStr = "http://publickinfo:600000000332";
  let newUri = new uri.URI(uriStr);
  assertEQ(newUri.toString(), "http://publickinfo:600000000332");
  assertEQ(newUri.port, "-1");
  assertEQ(newUri.host, null);
  assertEQ(newUri.authority, "publickinfo:600000000332");
}

function testspecialPort004() {
  let uriStr = "http://publickinfo:0";
  let newUri = new uri.URI(uriStr);
  assertEQ(newUri.toString(), "http://publickinfo:0");
  assertEQ(newUri.port, "0");
  assertEQ(newUri.host, "publickinfo");
  assertEQ(newUri.authority, "publickinfo:0");
}

function testUriConstruction032() {
  try {
    let that = new uri.URI('ht/tp://username:pas sword@host:8080/di[rectory/file?foo=1&bar=2');
  } catch (err: BusinessError) {
    assertEQ(err.toString(), "BusinessError: Syntax Error. Invalid Uri string: The SpecialPath does not conform to the rule.");
    assertEQ(err.code, 10200002);
    assertEQ(err.message, "Syntax Error. Invalid Uri string: The SpecialPath does not conform to the rule.");
  }
}

function testUriConstruction033() {
  try {
    let that = new uri.URI('http://username:password@[::]:8080/directory/file?Query#abc^de');
  } catch (err: BusinessError) {
    assertEQ(err.toString(), "BusinessError: Syntax Error. Invalid Uri string: The Fragment does not conform to the rule.");
    assertEQ(err.code, 10200002);
    assertEQ(err.message, "Syntax Error. Invalid Uri string: The Fragment does not conform to the rule.");
  }
}

function testUriConstruction034() {
  try {
    let route = new uri.URI('content://com.test.uri/files/a^a');
    let segments = route.getSegment();
  } catch (err: BusinessError) {
    assertEQ(err.toString(), "BusinessError: Syntax Error. Invalid Uri string: The Path does not conform to the rule.");
    assertEQ(err.code, 10200002);
    assertEQ(err.message, "Syntax Error. Invalid Uri string: The Path does not conform to the rule.");
  }
}




function main(): int {
  let suite = new ArkTestsuite("URI XTS tests");


  suite.addTest("testUriCreateFromParts001", testUriCreateFromParts001);
  suite.addTest("testUriCreateFromParts002", testUriCreateFromParts002);
  suite.addTest("testUriCreateFromParts003", testUriCreateFromParts003);
  suite.addTest("testUriCreateFromParts004", testUriCreateFromParts004);


  // suite.addTest("testGetBooleanQueryValuePara001", testGetBooleanQueryValuePara001);
  // suite.addTest("testGetBooleanQueryValuePara002", testGetBooleanQueryValuePara002);

  suite.addTest("testTildeUnderline001", testTildeUnderline001);
  suite.addTest("testTildeUnderline002", testTildeUnderline002);
  suite.addTest("testTildeUnderline003", testTildeUnderline003);

  suite.addTest("testAddQueryJson001", testAddQueryJson001);

  suite.addTest("testSpecialCharacters001", testSpecialCharacters001);

  suite.addTest("testspecialPort001", testspecialPort001);
  suite.addTest("testspecialPort002", testspecialPort002);
  suite.addTest("testspecialPort003", testspecialPort003);
  suite.addTest("testspecialPort004", testspecialPort004);

  suite.addTest("testUriConstruction032", testUriConstruction032);
  suite.addTest("testUriConstruction033", testUriConstruction033);
  suite.addTest("testUriConstruction034", testUriConstruction034);

 
  //passed
  return suite.run();
}