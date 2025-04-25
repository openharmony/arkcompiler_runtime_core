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

function testUriClearQuery001() {
  let route = new uri.URI('https://www.test.com');
  let newRoute = route.clearQuery();
  assertEQ(newRoute.toString(), "https://www.test.com");
}

function testUriClearQuery002() {
  let route = new uri.URI('https://www.test.com?param1=value1');
  let newRoute = route.clearQuery();
  assertEQ(newRoute.toString(), "https://www.test.com");
}

function testUriClearQuery003() {
  let route = new uri.URI('https://www.test.com?param1=value1%');
  let newRoute = route.clearQuery();
  assertEQ(newRoute.toString(), "https://www.test.com");
}

function testUriClearQuery004() {
  let route = new uri.URI('https://www.test.com?param1=value1&param1=value2');
  let newRoute = route.clearQuery();
  assertEQ(newRoute.toString(), "https://www.test.com");
}

function testUriUriEncodedQuery001() {
  let route = new uri.URI('https://www.test.com/search?query=name&query=my');
  assertEQ(route.encodedQuery, "query=name&query=my");
}

function testUriUriEncodedQuery002() {
  let route = new uri.URI('https://www.test.com/search?');
  assertEQ(route.encodedQuery, null);
}

function testUriUriEncodedQuery003() {
  let route = new uri.URI('https://www.test.com/');
  assertEQ(route.encodedQuery, null);
}

function testUriUriEncodedQuery004() {
  let route = new uri.URI('https://www.test.com/search?query=my%20test');
  assertEQ(route.encodedQuery, "query=my%20test");
}

function main(): int {
  let suite = new ArkTestsuite("URI XTS tests");


  suite.addTest("testUriClearQuery001", testUriClearQuery001);
  suite.addTest("testUriClearQuery002", testUriClearQuery002);
  suite.addTest("testUriClearQuery003", testUriClearQuery003);
  suite.addTest("testUriClearQuery004", testUriClearQuery004);

  
  suite.addTest("testUriUriEncodedQuery001", testUriUriEncodedQuery001);
  suite.addTest("testUriUriEncodedQuery002", testUriUriEncodedQuery002);
  suite.addTest("testUriUriEncodedQuery003", testUriUriEncodedQuery003);
  suite.addTest("testUriUriEncodedQuery004", testUriUriEncodedQuery004);
  //passed
  return suite.run();
}