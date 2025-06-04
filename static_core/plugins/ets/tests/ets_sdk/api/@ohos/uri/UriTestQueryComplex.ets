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

function testUriGetQueryNames001() {
  let route = new uri.URI('https://www.test.com?param1=value1&param2=value2');
  let paramNames = route.getQueryNames();
  assertEQ(paramNames.length, 2);
  assertEQ(paramNames.values().next().value, "param1");
}

function testUriGetQueryNames002() {
  let route = new uri.URI('https://www.test.com');
  let paramNames = route.getQueryNames();
  assertEQ(paramNames.length, 0);
}

function testUriGetQueryNames003() {
  let route = new uri.URI('https://www.test.com?param1=value1&&param3=value3');
  let paramNames = route.getQueryNames();
  let values = paramNames.values();
  assertEQ(paramNames.length, 3);
  assertEQ(values.next().value, "param1");
  assertEQ(values.next().value, "");
  assertEQ(values.next().value, "param3");
}

function testUriGetQueryNames004() {
  let route = new uri.URI('https://www.test.com?param1=value1&=value2&param3=value3');
  let paramNames = route.getQueryNames();
  assertEQ(paramNames.length, 3);
  let values = paramNames.values();
  assertEQ(paramNames.length, 3);
  assertEQ(values.next().value, "param1");
  assertEQ(values.next().value, "");
  assertEQ(values.next().value, "param3");
}

function testUriGetQueryNames005() {
  let route = new uri.URI('https://www.test.com?param1=value1&&&a&b=v&param3');
  let paramNames = route.getQueryNames();
  assertEQ(paramNames.length, 5);
  let values = paramNames.values();
  assertEQ(values.next().value, "param1");
  assertEQ(values.next().value, "");
  assertEQ(values.next().value, "a");
  assertEQ(values.next().value, "b");
  assertEQ(values.next().value, "param3");
}

function testUriGetQueryNames006() {
  let route = new uri.URI('https://www.test.com?param%201=value1');
  let paramNames = route.getQueryNames();
  assertEQ(paramNames.length, 1);
  assertEQ(paramNames.values().next().value, "param 1");
  let route1 = new uri.URI('https://www.test.com?param+1=value1');
  let paramNames1 = route1.getQueryNames();
  assertEQ(paramNames1.length, 1);
  assertEQ(paramNames1.values().next().value, "param+1");
}

function testUriGetQueryValues001() {
  let route = new uri.URI('https://www.test.com/search?query=name&sort=asc&filter=f');
  let values = route.getQueryValues("query");
  assertEQ(values.length, 1);
}

function testUriGetQueryValues002() {
  let route = new uri.URI('https://www.test.com/search?query=name');
  let values = route.getQueryValues("query");
  assertEQ(values[0], "name");
}

function testUriGetQueryValues003() {
  let emptyParameter1 = new uri.URI('https://www.test.com/search?query=');
  let result1 = emptyParameter1.getQueryValues("query");
  assertEQ(result1.length, 1);
  assertEQ(result1[0], "");
  let emptyParameter2 = new uri.URI('https://www.test.com/search?=value&');
  let result2 = emptyParameter2.getQueryValues("");
  assertEQ(result2[0], "value");
  assertEQ(result2[1], "");
  let emptyParameter3 = new uri.URI('https://www.test.com?');
  let result3 = emptyParameter3.getQueryValues("");
  assertEQ(result3.length, 0);
  let emptyParameter4 = new uri.URI('https://www.test.com?param');
  let result4 = emptyParameter4.getQueryValues("param");
  assertEQ(result4.length, 1);
  assertEQ(result4[0], "");
  let emptyParameter5 = new uri.URI('https://www.test.com?param1=value1&&param3=value3');
  let result5 = emptyParameter5.getQueryValues("");
  assertEQ(result5[0], "");
}

function testUriGetQueryValues004() {
  let route = new uri.URI('https://www.test.com/search');
  let values = route.getQueryValues("query");
  assertEQ(values.length, 0);
}

function testUriGetQueryValues005() {
  let route = new uri.URI('https://www.test.com/search?query=my%20query');
  let values = route.getQueryValues("query");
  assertEQ(values[0], "my query");
}

function testUriGetQueryValues006() {
  let route = new uri.URI('https://www.test.com/search?query=name&query=my');
  let values = route.getQueryValues("query");
  assertEQ(values[0], "name");
  assertEQ(values[1], "my");
}

function testUriGetQueryValues007() {
  let route = new uri.URI('https://www.test.com?query=hello+world');
  let values = route.getQueryValues("query");
  assertEQ(values[0], "hello+world");
}

function testUriGetQueryValues008() {
  let route1 = new uri.URI('https://www.test.com?param_g=value_1');
  let values1 = route1.getQueryValues("param_g");
  assertEQ(values1[0], "value_1");
  assertEQ(route1.getQueryValue('param_g'), "value_1");
}

function testUriGetBooleanQueryValue001() {
  let route = new uri.URI('https://www.test.com/search?active=true');
  let isActive = route.getBooleanQueryValue("active", false);
  assertTrue(isActive);
}

function testUriGetBooleanQueryValue002() {
  let route = new uri.URI('https://www.test.com/search');
  let isActive = route.getBooleanQueryValue("active", false);
  assertFalse(isActive);
}

function testUriGetBooleanQueryValue003() {
  let route = new uri.URI('https://www.test.com/search?active=name');
  let isActive = route.getBooleanQueryValue("active", false);
  assertTrue(isActive);
}

function testUriGetBooleanQueryValue004() {
  let route = new uri.URI('https://www.test.com/search');
  let isActive = route.getBooleanQueryValue("active", true);
  assertTrue(isActive);
}

function testUriGetBooleanQueryValue005() {
  let route = new uri.URI('https://www.test.com/search?active=true&active=false');
  let isActive = route.getBooleanQueryValue("active", false);
  assertTrue(isActive);
  let uriInstance1 = new uri.URI("https://www.test.com/search?active=aa&active=false");
  assertTrue(uriInstance1.getBooleanQueryValue("active", false));
  let uriInstance2 = new uri.URI("https://www.test.com/search?active=0");
  assertFalse(uriInstance2.getBooleanQueryValue("active", true));
}

function main(): int {
  let suite = new ArkTestsuite("URI XTS tests");
  
  suite.addTest("testUriGetQueryNames001", testUriGetQueryNames001);
  suite.addTest("testUriGetQueryNames002", testUriGetQueryNames002);
  suite.addTest("testUriGetQueryNames003", testUriGetQueryNames003);
  suite.addTest("testUriGetQueryNames004", testUriGetQueryNames004);
  suite.addTest("testUriGetQueryNames005", testUriGetQueryNames005);
  suite.addTest("testUriGetQueryNames006", testUriGetQueryNames006);

  suite.addTest("testUriGetQueryValues001", testUriGetQueryValues001);
  suite.addTest("testUriGetQueryValues002", testUriGetQueryValues002);
  suite.addTest("testUriGetQueryValues003", testUriGetQueryValues003);
  suite.addTest("testUriGetQueryValues004", testUriGetQueryValues004);
  suite.addTest("testUriGetQueryValues005", testUriGetQueryValues005);
  suite.addTest("testUriGetQueryValues006", testUriGetQueryValues006);
  suite.addTest("testUriGetQueryValues007", testUriGetQueryValues007);
  suite.addTest("testUriGetQueryValues008", testUriGetQueryValues008);

  suite.addTest("testUriGetBooleanQueryValue001", testUriGetBooleanQueryValue001);
  suite.addTest("testUriGetBooleanQueryValue002", testUriGetBooleanQueryValue002);
  suite.addTest("testUriGetBooleanQueryValue003", testUriGetBooleanQueryValue003);
  suite.addTest("testUriGetBooleanQueryValue004", testUriGetBooleanQueryValue004);
  suite.addTest("testUriGetBooleanQueryValue005", testUriGetBooleanQueryValue005);

  //passed
  return suite.run();
}