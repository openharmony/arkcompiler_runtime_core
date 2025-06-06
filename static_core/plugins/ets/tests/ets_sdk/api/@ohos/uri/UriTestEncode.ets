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

function testUriEncodedUserInfo001() {
    let route = new uri.URI('http://username:name@www.test.com/');
    assertEQ(route.encodedUserInfo, "username:name");
}

function testUriEncodedUserInfo002() {
    let route = new uri.URI('http://username:@www.test.com/');
    assertEQ(route.encodedUserInfo, "username:");
}

function testUriEncodedUserInfo003() {
    let route = new uri.URI('http://username:my%20name@www.test.com/');
    assertEQ(route.encodedUserInfo, "username:my%20name");
}

function testUriEncodedUserInfo004() {
    let route = new uri.URI('http://www.test.com');
    assertEQ(route.encodedUserInfo, null);
    let mailRoute = new uri.URI('mailto:body');
    assertEQ(mailRoute.encodedUserInfo, null);
}

function testUriEncodedPath001() {
    let route = new uri.URI('http://www.test.com/file/image.jpg');
    assertEQ(route.encodedPath, "/file/image.jpg");
}

function testUriEncodedPath002() {
    let route = new uri.URI('http://www.test.com/');
    assertEQ(route.encodedPath, "/");
}

function testUriEncodedPath003() {
    let route = new uri.URI('http://www.test.com');
    assertEQ(route.encodedPath, null);
    let mailRoute = new uri.URI('mailto:body');
    assertEQ(mailRoute.encodedUserInfo, null);
}

function testUriEncodedPath004() {
    let route = new uri.URI('http://www.test.com/my%20file');
    assertEQ(route.encodedPath, "/my%20file");
}


function testUriEncodedFragment001() {
    let route = new uri.URI('mailto:body#top');
    assertEQ(route.encodedFragment, "top");
}

function testUriEncodedFragment002() {
    let route = new uri.URI('mailto:body#');
    assertEQ(route.encodedFragment, null);
}

function testUriEncodedFragment003() {
    let route = new uri.URI('mailto:body#te%20st');
    assertEQ(route.encodedFragment, "te%20st");
}

function testUriEncodedFragment004() {
    let route = new uri.URI('mailto:body');
    assertEQ(route.encodedFragment, null);
}

function testUriEncodedAuthority001() {
    let route = new uri.URI('https://www.test.com/search?query=name&query=my');
    assertEQ(route.encodedAuthority, "www.test.com");
}

function testUriEncodedAuthority002() {
    let route = new uri.URI('http://username:my%20name@www.test.com/');
    assertEQ(route.encodedAuthority, "username:my%20name@www.test.com");
}

function testUriEncodedAuthority003() {
    let route = new uri.URI('mailto:body#top');
    assertEQ(route.encodedAuthority, null);
}

function testUriEncodedAuthority004() {
    let route = new uri.URI('https://www.test.com:8080/');
    assertEQ(route.encodedAuthority, "www.test.com:8080");
}

function testUriEncodedSSP001() {
    let route = new uri.URI('http://username:name@www.test.com/path?query=example');
    assertEQ(route.encodedSSP, "//username:name@www.test.com/path?query=example");
}

function testUriEncodedSSP002() {
    let route = new uri.URI('http://username:my%20name@www.test.com');
    assertEQ(route.encodedSSP, "//username:my%20name@www.test.com");
    let route1 = new uri.URI('http://username:my_20name@www.test.com');
    assertEQ(route1.encodedSSP, "//username:my_20name@www.test.com");
}

function testUriEncodedSSP003() {
    let route = new uri.URI('https://www.test.com/');
    assertEQ(route.encodedSSP, "//www.test.com/");
}

function main(): int {
    let suite = new ArkTestsuite("URI XTS tests");
    suite.addTest("testUriEncodedUserInfo001", testUriEncodedUserInfo001);
    suite.addTest("testUriEncodedUserInfo002", testUriEncodedUserInfo002);
    suite.addTest("testUriEncodedUserInfo003", testUriEncodedUserInfo003);
    suite.addTest("testUriEncodedUserInfo004", testUriEncodedUserInfo004);

    suite.addTest("testUriEncodedPath001", testUriEncodedPath001);
    suite.addTest("testUriEncodedPath002", testUriEncodedPath002);
    suite.addTest("testUriEncodedPath003", testUriEncodedPath003);
    suite.addTest("testUriEncodedPath004", testUriEncodedPath004);


    suite.addTest("testUriEncodedFragment001", testUriEncodedFragment001);
    suite.addTest("testUriEncodedFragment002", testUriEncodedFragment002);
    suite.addTest("testUriEncodedFragment003", testUriEncodedFragment003);
    suite.addTest("testUriEncodedFragment004", testUriEncodedFragment004);

    suite.addTest("testUriEncodedAuthority001", testUriEncodedAuthority001);
    suite.addTest("testUriEncodedAuthority002", testUriEncodedAuthority002);
    suite.addTest("testUriEncodedAuthority003", testUriEncodedAuthority003);
    suite.addTest("testUriEncodedAuthority004", testUriEncodedAuthority004);

    suite.addTest("testUriEncodedSSP001", testUriEncodedSSP001);
    suite.addTest("testUriEncodedSSP002", testUriEncodedSSP002);
    suite.addTest("testUriEncodedSSP003", testUriEncodedSSP003);
    //passed
    return suite.run();
}