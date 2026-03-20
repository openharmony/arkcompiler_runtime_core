/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
    arktest.assertEQ(route.encodedUserInfo, 'username:name');
}

function testUriEncodedUserInfo002() {
    let route = new uri.URI('http://username:@www.test.com/');
    arktest.assertEQ(route.encodedUserInfo, 'username:');
}

function testUriEncodedUserInfo003() {
    let route = new uri.URI('http://username:my%20name@www.test.com/');
    arktest.assertEQ(route.encodedUserInfo, 'username:my%20name');
}

function testUriEncodedUserInfo004() {
    let route = new uri.URI('http://www.test.com');  // CC-OFF(G.OTH.05) test logic
    arktest.assertEQ(route.encodedUserInfo, null);
    let mailRoute = new uri.URI('mailto:body');
    arktest.assertEQ(mailRoute.encodedUserInfo, null);
}

function testUriEncodedPath001() {
    let route = new uri.URI('http://www.test.com/file/image.jpg');  // CC-OFF(G.OTH.05) test logic
    arktest.assertEQ(route.encodedPath, '/file/image.jpg');
}

function testUriEncodedPath002() {
    let route = new uri.URI('http://www.test.com/');  // CC-OFF(G.OTH.05) test logic
    arktest.assertEQ(route.encodedPath, '/');
}

function testUriEncodedPath003() {
    let route = new uri.URI('http://www.test.com');  // CC-OFF(G.OTH.05) test logic
    arktest.assertEQ(route.encodedPath, null);
    let mailRoute = new uri.URI('mailto:body');
    arktest.assertEQ(mailRoute.encodedUserInfo, null);
}

function testUriEncodedPath004() {
    let route = new uri.URI('http://www.test.com/my%20file');  // CC-OFF(G.OTH.05) test logic
    arktest.assertEQ(route.encodedPath, '/my%20file');
}


function testUriEncodedFragment001() {
    let route = new uri.URI('mailto:body#top');
    arktest.assertEQ(route.encodedFragment, 'top');
}

function testUriEncodedFragment002() {
    let route = new uri.URI('mailto:body#');
    arktest.assertEQ(route.encodedFragment, null);
}

function testUriEncodedFragment003() {
    let route = new uri.URI('mailto:body#te%20st');
    arktest.assertEQ(route.encodedFragment, 'te%20st');
}

function testUriEncodedFragment004() {
    let route = new uri.URI('mailto:body');
    arktest.assertEQ(route.encodedFragment, null);
}

function testUriEncodedAuthority001() {
    let route = new uri.URI('https://www.test.com/search?query=name&query=my');  // CC-OFF(G.OTH.05) test logic
    arktest.assertEQ(route.encodedAuthority, 'www.test.com');
}

function testUriEncodedAuthority002() {
    let route = new uri.URI('http://username:my%20name@www.test.com/');
    arktest.assertEQ(route.encodedAuthority, 'username:my%20name@www.test.com');
}

function testUriEncodedAuthority003() {
    let route = new uri.URI('mailto:body#top');
    arktest.assertEQ(route.encodedAuthority, null);
}

function testUriEncodedAuthority004() {
    let route = new uri.URI('https://www.test.com:8080/');  // CC-OFF(G.OTH.05) test logic
    arktest.assertEQ(route.encodedAuthority, 'www.test.com:8080');
}

function testUriEncodedSSP001() {
    let route = new uri.URI('http://username:name@www.test.com/path?query=example');
    arktest.assertEQ(route.encodedSSP, '//username:name@www.test.com/path?query=example');
}

function testUriEncodedSSP002() {
    let route = new uri.URI('http://username:my%20name@www.test.com');
    arktest.assertEQ(route.encodedSSP, '//username:my%20name@www.test.com');
    let route1 = new uri.URI('http://username:my_20name@www.test.com');
    arktest.assertEQ(route1.encodedSSP, '//username:my_20name@www.test.com');
}

function testUriEncodedSSP003() {
    let route = new uri.URI('https://www.test.com/');  // CC-OFF(G.OTH.05) test logic
    arktest.assertEQ(route.encodedSSP, '//www.test.com/');
}

function testUriEncodedUserInfo005() {
    let route = new uri.URI('http://user%20test:pass%20test@www.test.com/');
    arktest.assertEQ(route.encodedUserInfo, 'user%20test:pass%20test');
}

function testUriEncodedUserInfo006() {
    let route = new uri.URI('http://user%40domain:pass%3F@www.test.com/');
    arktest.assertEQ(route.encodedUserInfo, 'user%40domain:pass%3F');
}

function testUriEncodedUserInfo007() {
    let route = new uri.URI('http://user%5B%5D:pass%7B%7D@www.test.com/');
    arktest.assertEQ(route.encodedUserInfo, 'user%5B%5D:pass%7B%7D');
}

function testUriEncodedUserInfo008() {
    let route = new uri.URI('http://user%2Fslash:pass%2Fslash@www.test.com/');
    arktest.assertEQ(route.encodedUserInfo, 'user%2Fslash:pass%2Fslash');
}

function testUriEncodedUserInfo009() {
    let route = new uri.URI('http://user%3Asemicol:pass%3Asemicol@www.test.com/');
    arktest.assertEQ(route.encodedUserInfo, 'user%3Asemicol:pass%3Asemicol');
}

function testUriEncodedPath005() {
    let route = new uri.URI('http://www.test.com/my%20path');
    arktest.assertEQ(route.encodedPath, '/my%20path');
}

function testUriEncodedPath006() {
    let route = new uri.URI('http://www.test.com/path%3Fquery');
    arktest.assertEQ(route.encodedPath, '/path%3Fquery');
}

function testUriEncodedPath007() {
    let route = new uri.URI('http://www.test.com/path%23fragment');
    arktest.assertEQ(route.encodedPath, '/path%23fragment');
}

function testUriEncodedPath008() {
    let route = new uri.URI('http://www.test.com/path%5Bbracket%5D');
    arktest.assertEQ(route.encodedPath, '/path%5Bbracket%5D');
}

function testUriEncodedPath009() {
    let route = new uri.URI('http://www.test.com/path%3Acolon');
    arktest.assertEQ(route.encodedPath, '/path%3Acolon');
}

function testUriEncodedFragment005() {
    let route = new uri.URI('http://www.test.com#fragment%20with%20space');
    arktest.assertEQ(route.encodedFragment, 'fragment%20with%20space');
}

function testUriEncodedFragment006() {
    let route = new uri.URI('http://www.test.com#fragment%3Fquestion');
    arktest.assertEQ(route.encodedFragment, 'fragment%3Fquestion');
}

function testUriEncodedFragment007() {
    let route = new uri.URI('http://www.test.com#fragment%23hash');
    arktest.assertEQ(route.encodedFragment, 'fragment%23hash');
}

function testUriEncodedUserInfo010() {
    let route = new uri.URI('http://username:my%20%D8%00@www.test.com/');
    arktest.assertEQ(route.encodedUserInfo, 'username:my%20%D8%00');
}

function testUriEncodedUserInfo011() {
    let route = new uri.URI('http://username:my%20%DC%00@www.test.com/');
    arktest.assertEQ(route.encodedUserInfo, 'username:my%20%DC%00');
}

function testUriEncodedUserInfo012() {
    let route = new uri.URI('http://user%20%D8%00:password@www.test.com/');
    arktest.assertEQ(route.encodedUserInfo, 'user%20%D8%00:password');
}

function main(): int {
    let suite = new arktest.ArkTestsuite('URI XTS tests');
    
    suite.addTest('testUriEncodedUserInfo001', testUriEncodedUserInfo001);
    suite.addTest('testUriEncodedUserInfo002', testUriEncodedUserInfo002);
    suite.addTest('testUriEncodedUserInfo003', testUriEncodedUserInfo003);
    suite.addTest('testUriEncodedUserInfo004', testUriEncodedUserInfo004);
    suite.addTest('testUriEncodedUserInfo005', testUriEncodedUserInfo005);
    suite.addTest('testUriEncodedUserInfo006', testUriEncodedUserInfo006);
    suite.addTest('testUriEncodedUserInfo007', testUriEncodedUserInfo007);
    suite.addTest('testUriEncodedUserInfo008', testUriEncodedUserInfo008);
    suite.addTest('testUriEncodedUserInfo009', testUriEncodedUserInfo009);
    suite.addTest('testUriEncodedUserInfo010', testUriEncodedUserInfo010);
    suite.addTest('testUriEncodedUserInfo011', testUriEncodedUserInfo011);
    suite.addTest('testUriEncodedUserInfo012', testUriEncodedUserInfo012);
    
    suite.addTest('testUriEncodedPath001', testUriEncodedPath001);
    suite.addTest('testUriEncodedPath002', testUriEncodedPath002);
    suite.addTest('testUriEncodedPath003', testUriEncodedPath003);
    suite.addTest('testUriEncodedPath004', testUriEncodedPath004);
    suite.addTest('testUriEncodedPath005', testUriEncodedPath005);
    suite.addTest('testUriEncodedPath006', testUriEncodedPath006);
    suite.addTest('testUriEncodedPath007', testUriEncodedPath007);
    suite.addTest('testUriEncodedPath008', testUriEncodedPath008);
    suite.addTest('testUriEncodedPath009', testUriEncodedPath009);
    
    suite.addTest('testUriEncodedFragment001', testUriEncodedFragment001);
    suite.addTest('testUriEncodedFragment002', testUriEncodedFragment002);
    suite.addTest('testUriEncodedFragment003', testUriEncodedFragment003);
    suite.addTest('testUriEncodedFragment004', testUriEncodedFragment004);
    suite.addTest('testUriEncodedFragment005', testUriEncodedFragment005);
    suite.addTest('testUriEncodedFragment006', testUriEncodedFragment006);
    suite.addTest('testUriEncodedFragment007', testUriEncodedFragment007);
    
    suite.addTest('testUriEncodedAuthority001', testUriEncodedAuthority001);
    suite.addTest('testUriEncodedAuthority002', testUriEncodedAuthority002);
    suite.addTest('testUriEncodedAuthority003', testUriEncodedAuthority003);
    suite.addTest('testUriEncodedAuthority004', testUriEncodedAuthority004);
    
    suite.addTest('testUriEncodedSSP001', testUriEncodedSSP001);
    suite.addTest('testUriEncodedSSP002', testUriEncodedSSP002);
    suite.addTest('testUriEncodedSSP003', testUriEncodedSSP003);
    
    return suite.run();
}
