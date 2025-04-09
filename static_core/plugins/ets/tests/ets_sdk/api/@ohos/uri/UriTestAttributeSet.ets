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
import { uri } from "@ohos.uri";

let uriObj: uri.URI;

function SetSchemeTest() {
    uriObj = new uri.URI("ht2tp://user@www.1hw.1com:77/path/path23?query#qwer");
    uriObj.scheme = "https:"
    assertEQ(uriObj.toString(), 'https://user@www.1hw.1com:77/path/path23?query#qwer');

    uriObj = new uri.URI("ht2tp://user@www.1hw.1com:77/path/path23?query#qwer");
    uriObj.scheme = "3https:"
    assertEQ(uriObj.toString(), 'ht2tp://user@www.1hw.1com:77/path/path23?query#qwer');

    uriObj = new uri.URI("ht2tp://user@www.1hw.1com:77/path/path23?query#qwer");
    uriObj.scheme = "asffdsa"
    assertEQ(uriObj.toString(), 'asffdsa://user@www.1hw.1com:77/path/path23?query#qwer');

    uriObj = new uri.URI("ht2tp://user@www.1hw.1com:77/path/path23?query#qwer");
    uriObj.scheme = "%asffdsa"
    assertEQ(uriObj.toString(), 'ht2tp://user@www.1hw.1com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://user@www.1hw.1com:77/path/path23?query#qwer");
    uriObj.scheme = ""
    assertEQ(uriObj.toString(), 'http://user@www.1hw.1com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://user@www.1hw.1com:77?query#qwer");
    uriObj.scheme = "testuri006";
    assertEQ(uriObj.scheme, 'testuri006');
}

function SetUserInfoTest() {
    uriObj = new uri.URI("http://user@www.hw.com:77/path/path23?query#qwer");
    uriObj.userInfo = "testuri007"
    assertEQ(uriObj.userInfo, 'testuri007');
    assertEQ(uriObj.toString(), 'http://testuri007@www.hw.com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://www.test.com:77/path/path23?query#qwer");
    uriObj.userInfo = "testuri008"
    assertEQ(uriObj.userInfo, 'testuri008');
    assertEQ(uriObj.toString(), 'http://testuri008@www.test.com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.userInfo = "sji<+s大"
    assertEQ(uriObj.userInfo, 'sji<+s大');
    assertEQ(uriObj.toString(), 'http://sji%3C%2Bs%E5%A4%A7@www.test.com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http:/userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.userInfo = "sji<+s大"
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.userInfo = ""
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.toString(), 'http://www.test.com:77/path/path23?query#qwer');
}

function SetPathTest() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.path = ""
    assertEQ(uriObj.path, null);
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.path = "pathtest/pp"
    assertEQ(uriObj.path, 'pathtest/pp');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/pathtest/pp?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.path = "pathtest/p大p"
    assertEQ(uriObj.path, 'pathtest/p大p');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/pathtest/p%E5%A4%A7p?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77?query#qwer");
    uriObj.path = "/testuri015/p大p/"
    assertEQ(uriObj.path, '/testuri015/p大p/');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri015/p%E5%A4%A7p/?query#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77?query#qwer");
    uriObj.path = "/testuri016/p大p/"
    assertEQ(uriObj.ssp, '//userinfo@www.test.com:77/testuri016/p大p/?query');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri016/p%E5%A4%A7p/?query#qwer');

    uriObj = new uri.URI("http:userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.path = "/testuri017/p大p/"
    assertEQ(uriObj.path, '/testuri017/p大p/');
    assertEQ(uriObj.toString(), 'http:/testuri017/p%E5%A4%A7p/#qwer');
}

function SetQueryTest() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.query = "/testuri018/p大p/=sasf"
    assertEQ(uriObj.query, '/testuri018/p大p/=sasf');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?%2Ftesturi018%2Fp%E5%A4%A7p%2F%3Dsasf#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.query = "key=value&key1=value1"
    assertEQ(uriObj.query, 'key=value&key1=value1');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?key%3Dvalue%26key1%3Dvalue1#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.query = '';
    assertEQ(uriObj.query, null);
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/#qwer');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.query = 'kkey=sal';
    assertEQ(uriObj.query, "kkey=sal");
    assertEQ(uriObj.ssp, "//userinfo@www.test.com:77/testuri017/p大p/?kkey=sal");
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?kkey%3Dsal#qwer');

    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.query = 'kkey=sal&jj=oi';
    assertEQ(uriObj.query, "kkey=sal&jj=oi");
    assertEQ(uriObj.ssp, "/userinfo@www.test.com:77/testuri017/p大p/?kkey=sal&jj=oi");
    assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?kkey%3Dsal%26jj%3Doi#qwer');

}

function SetFragmentTest() {
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.fragment = 'kkey=sal&jj=oi';
    assertEQ(uriObj.fragment, "kkey=sal&jj=oi");
    assertEQ(uriObj.ssp, "/userinfo@www.test.com:77/testuri017/p大p/?query");
    let res = 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#kkey%3Dsal%26jj%3Doi';
    assertEQ(uriObj.toString(), res);

    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.fragment = '';
    assertEQ(uriObj.fragment, null);
    assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query');

    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.fragment = '#jia片段';
    assertEQ(uriObj.fragment, "#jia片段");
    assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#%23jia%E7%89%87%E6%AE%B5');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query");
    uriObj.fragment = '#jia片段f';
    assertEQ(uriObj.fragment, "#jia片段f");
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#%23jia%E7%89%87%E6%AE%B5f');

    uriObj = new uri.URI("http:userinfo@www.test.com:77/testuri017/p大p/?query");
    uriObj.fragment = 'jia片段f';
    assertEQ(uriObj.fragment, "jia片段f");
    assertEQ(uriObj.toString(), 'http:userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#jia%E7%89%87%E6%AE%B5f');

}

function SetAuthorityTest() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.authority = 'www.test11.cn:88';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, "www.test11.cn");
    assertEQ(uriObj.port, "88");
    assertEQ(uriObj.toString(), 'http://www.test11.cn:88/testuri017/p%E5%A4%A7p/?query#asd');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.authority = 'uusc@www.testuc.com:741';
    assertEQ(uriObj.userInfo, "uusc");
    assertEQ(uriObj.host, "www.testuc.com");
    assertEQ(uriObj.port, "741");
    assertEQ(uriObj.authority, "uusc@www.testuc.com:741");
    assertEQ(uriObj.toString(), 'http://uusc@www.testuc.com:741/testuri017/p%E5%A4%A7p/?query#asd');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.authority = '';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, null);
    assertEQ(uriObj.port, "-1");
    assertEQ(uriObj.authority, null);
    assertEQ(uriObj.ssp, '/testuri017/p大p/?query');
    assertEQ(uriObj.toString(), 'http:/testuri017/p%E5%A4%A7p/?query#asd');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.authority = 'usc@';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, null);
    assertEQ(uriObj.port, "-1");
    assertEQ(uriObj.authority, "usc@");
    assertEQ(uriObj.ssp, '//usc@/testuri017/p大p/?query');
    assertEQ(uriObj.toString(), 'http://usc@/testuri017/p%E5%A4%A7p/?query#asd');
}

function SetSspTest() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.ssp = 'usc@asjf:99/path1/ss?key=1';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, null);
    assertEQ(uriObj.port, "-1");
    assertEQ(uriObj.authority, null);
    assertEQ(uriObj.path, null);
    assertEQ(uriObj.ssp, 'usc@asjf:99/path1/ss?key=1');
    assertEQ(uriObj.toString(), 'http:usc@asjf:99/path1/ss?key=1#asd');

    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.ssp = '/usc@asjf:99/path1/ss?key=1';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, null);
    assertEQ(uriObj.path, "/usc@asjf:99/path1/ss");
    assertEQ(uriObj.authority, null);
    assertEQ(uriObj.ssp, '/usc@asjf:99/path1/ss?key=1');
    assertEQ(uriObj.toString(), 'http:/usc@asjf:99/path1/ss?key=1#asd');

    uriObj = new uri.URI("safdcas://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.ssp = '//usc@www.test321.cn:996?key=1';
    assertEQ(uriObj.userInfo, "usc");
    assertEQ(uriObj.host, "www.test321.cn");
    assertEQ(uriObj.path, null);
    assertEQ(uriObj.authority, 'usc@www.test321.cn:996');
    assertEQ(uriObj.ssp, '//usc@www.test321.cn:996?key=1');
    assertEQ(uriObj.toString(), 'safdcas://usc@www.test321.cn:996?key=1#asd');

    uriObj = new uri.URI("safdcas://userinfo@www.test.com:77/testuri017/p大p/?query#asdasf");
    uriObj.ssp = '//www.test321.cn';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, "www.test321.cn");
    assertEQ(uriObj.path, null);
    assertEQ(uriObj.authority, 'www.test321.cn');
    assertEQ(uriObj.ssp, '//www.test321.cn');
    assertEQ(uriObj.toString(), 'safdcas://www.test321.cn#asdasf');


}

function SetEncodedUserInfo() {
    uriObj = new uri.URI("http://user@www.hw.com:77/path/path23?query#qwer");
    uriObj.encodedUserInfo = "testuri007"
    assertEQ(uriObj.encodedUserInfo, 'testuri007');
    assertEQ(uriObj.toString(), 'http://testuri007@www.hw.com:77/path/path23?query#qwer');

    uriObj = new uri.URI("http://www.test.com:77/path/path23?query#qwer");
    uriObj.encodedUserInfo = "testuri008"
    assertEQ(uriObj.encodedUserInfo, 'testuri008');
    assertEQ(uriObj.toString(), 'http://testuri008@www.test.com:77/path/path23?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedUserInfo = "sji<+s大"
    assertEQ(uriObj.encodedUserInfo, 'userinfo');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/path/path23?query#qwer');
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedUserInfo = "sji<+s大"
    assertEQ(uriObj.encodedUserInfo, null);
    assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/path/path23?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedUserInfo = ""
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.toString(), 'http://www.test.com:77/path/path23?query#qwer');
}

function SetEncodedPathTest() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedPath = ""
    assertEQ(uriObj.encodedPath, null);
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedPath = "pathtest/pp"
    assertEQ(uriObj.encodedPath, 'pathtest/pp');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/pathtest/pp?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/path/path23?query#qwer");
    uriObj.encodedPath = "pathtest/p%E5%A4%A7p"
    assertEQ(uriObj.encodedPath, 'pathtest/p%E5%A4%A7p');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/pathtest/p%E5%A4%A7p?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77?query#qwer");
    uriObj.encodedPath = "/testuri015/p大p/"
    assertEQ(uriObj.encodedPath, '/testuri015/p大p/');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri015/p%E5%A4%A7p/?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77");
    uriObj.encodedPath = "testuri016/p%E5%A4%A7/"
    assertEQ(uriObj.encodedPath, 'testuri016/p%E5%A4%A7/');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri016/p%E5%A4%A7/');
}

function SetEncodedQuery() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedQuery = "/testuri018/p大p/=sasf"
    assertEQ(uriObj.encodedQuery, '/testuri018/p大p/=sasf');
    let res = 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?/testuri018/p%E5%A4%A7p/=sasf#qwer';
    assertEQ(uriObj.toString(), res);
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedQuery = "key=value&key1=value1"
    assertEQ(uriObj.encodedQuery, 'key=value&key1=value1');
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?key=value&key1=value1#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedQuery = '';
    assertEQ(uriObj.encodedQuery, null);
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedQuery = 'kkey=sal';
    assertEQ(uriObj.encodedQuery, "kkey=sal");
    assertEQ(uriObj.ssp, "//userinfo@www.test.com:77/testuri017/p大p/?kkey=sal");
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?kkey=sal#qwer');
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedQuery = 'kkey=sal&jj=oi';
    assertEQ(uriObj.encodedQuery, "kkey=sal&jj=oi");
    assertEQ(uriObj.ssp, "/userinfo@www.test.com:77/testuri017/p大p/?kkey=sal&jj=oi");
    assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?kkey=sal&jj=oi#qwer');
}

function SetEncodedFragment() {
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedFragment = 'kkey=sal&jj=oi';
    assertEQ(uriObj.encodedFragment, "kkey=sal&jj=oi");
    assertEQ(uriObj.ssp, "/userinfo@www.test.com:77/testuri017/p大p/?query");
    assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#kkey=sal&jj=oi');
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedFragment = '';
    assertEQ(uriObj.encodedFragment, null);
    assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query');
    uriObj = new uri.URI("http:/userinfo@www.test.com:77/testuri017/p大p/?query#qwer");
    uriObj.encodedFragment = '#jia片段';
    assertEQ(uriObj.encodedFragment, "qwer");
    assertEQ(uriObj.toString(), 'http:/userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#qwer');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query");
    uriObj.encodedFragment = 'jia片段f';
    assertEQ(uriObj.encodedFragment, "jia片段f");
    assertEQ(uriObj.toString(), 'http://userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#jia%E7%89%87%E6%AE%B5f');
    uriObj = new uri.URI("http:userinfo@www.test.com:77/testuri017/p大p/?query");
    uriObj.encodedFragment = 'jia片段f';
    assertEQ(uriObj.encodedFragment, "jia片段f");
    assertEQ(uriObj.toString(), 'http:userinfo@www.test.com:77/testuri017/p%E5%A4%A7p/?query#jia%E7%89%87%E6%AE%B5f');
}

function SetEncodedAuthority() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedAuthority = 'www.test11.cn:88';
    assertEQ(uriObj.encodedUserInfo, null);
    assertEQ(uriObj.host, "www.test11.cn");
    assertEQ(uriObj.port, "88");
    assertEQ(uriObj.toString(), 'http://www.test11.cn:88/testuri017/p%E5%A4%A7p/?query#asd');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedAuthority = 'uusc@www.testuc.com:741';
    assertEQ(uriObj.encodedUserInfo, "uusc");
    assertEQ(uriObj.host, "www.testuc.com");
    assertEQ(uriObj.port, "741");
    assertEQ(uriObj.encodedAuthority, "uusc@www.testuc.com:741");
    assertEQ(uriObj.toString(), 'http://uusc@www.testuc.com:741/testuri017/p%E5%A4%A7p/?query#asd');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedAuthority = '';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, null);
    assertEQ(uriObj.port, "-1");
    assertEQ(uriObj.encodedAuthority, null);
    assertEQ(uriObj.encodedSSP, '/testuri017/p大p/?query');
    assertEQ(uriObj.toString(), 'http:/testuri017/p%E5%A4%A7p/?query#asd');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedAuthority = 'usc@';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, null);
    assertEQ(uriObj.port, "-1");
    assertEQ(uriObj.encodedAuthority, "usc@");
    assertEQ(uriObj.encodedSSP, '//usc@/testuri017/p大p/?query');
    assertEQ(uriObj.toString(), 'http://usc@/testuri017/p%E5%A4%A7p/?query#asd');
}

function SetEncodedSsp() {
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedSSP = 'usc@asjf:99/path1/ss?key=1';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, null);
    assertEQ(uriObj.port, "-1");
    assertEQ(uriObj.authority, null);
    assertEQ(uriObj.path, null);
    assertEQ(uriObj.encodedSSP, 'usc@asjf:99/path1/ss?key=1');
    assertEQ(uriObj.toString(), 'http:usc@asjf:99/path1/ss?key=1#asd');
    uriObj = new uri.URI("http://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedSSP = '/usc@asjf:99/path1/ss?key=1';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, null);
    assertEQ(uriObj.path, "/usc@asjf:99/path1/ss");
    assertEQ(uriObj.authority, null);
    assertEQ(uriObj.encodedSSP, '/usc@asjf:99/path1/ss?key=1');
    assertEQ(uriObj.toString(), 'http:/usc@asjf:99/path1/ss?key=1#asd');
    uriObj = new uri.URI("safdcas://userinfo@www.test.com:77/testuri017/p大p/?query#asd");
    uriObj.encodedSSP = '//usc@www.test321.cn:996?key=1';
    assertEQ(uriObj.userInfo, "usc");
    assertEQ(uriObj.host, "www.test321.cn");
    assertEQ(uriObj.path, null);
    assertEQ(uriObj.encodedAuthority, 'usc@www.test321.cn:996');
    assertEQ(uriObj.encodedSSP, '//usc@www.test321.cn:996?key=1');
    assertEQ(uriObj.toString(), 'safdcas://usc@www.test321.cn:996?key=1#asd');
    uriObj = new uri.URI("safdcas://userinfo@www.test.com:77/testuri017/p大p/?query#asdasf");
    uriObj.encodedSSP = '//www.test321.cn';
    assertEQ(uriObj.userInfo, null);
    assertEQ(uriObj.host, "www.test321.cn");
    assertEQ(uriObj.path, null);
    assertEQ(uriObj.authority, 'www.test321.cn');
    assertEQ(uriObj.encodedSSP, '//www.test321.cn');
    assertEQ(uriObj.toString(), 'safdcas://www.test321.cn#asdasf');
}

function UriSetNullAttribute() {
    let gaogao = new uri.URI('http://gg:gaogao@www.baidu.com:99/path/path?query#fagment');
    assertEQ(gaogao.scheme, "http");
    assertEQ(gaogao.path, "/path/path");
    assertEQ(gaogao.authority, "gg:gaogao@www.baidu.com:99");
    assertEQ(gaogao.userInfo, "gg:gaogao");
    assertEQ(gaogao.query, "query");
    assertEQ(gaogao.fragment, "fagment");
    assertEQ(gaogao.encodedUserInfo, "gg:gaogao");
    assertEQ(gaogao.encodedPath, "/path/path");
    assertEQ(gaogao.encodedQuery, "query");
    assertEQ(gaogao.encodedFragment, "fagment");
    assertEQ(gaogao.encodedAuthority, "gg:gaogao@www.baidu.com:99");

    gaogao.scheme = null;
    gaogao.path = null;
    gaogao.authority = null;
    gaogao.userInfo = null;
    gaogao.query = null;
    gaogao.fragment = null;
    gaogao.encodedUserInfo = null;
    gaogao.encodedPath = null;
    gaogao.encodedQuery = null;
    gaogao.encodedFragment = null;
    gaogao.encodedAuthority = null;


    assertEQ(gaogao.scheme, "http");
    assertEQ(gaogao.path, "/path/path");
    assertEQ(gaogao.authority, "gg:gaogao@www.baidu.com:99");
    assertEQ(gaogao.userInfo, "gg:gaogao");
    assertEQ(gaogao.query, "query");
    assertEQ(gaogao.fragment, "fagment");
    assertEQ(gaogao.encodedUserInfo, "gg:gaogao");
    assertEQ(gaogao.encodedPath, "/path/path");
    assertEQ(gaogao.encodedQuery, "query");
    assertEQ(gaogao.encodedFragment, "fagment");
    assertEQ(gaogao.encodedAuthority, "gg:gaogao@www.baidu.com:99");

    // NOTE(zhangziye):#25267 Unable to use different getter/setter signature 
    // assertEQ(gaogao.ssp, "//gg:gaogao@www.baidu.com:99/path/path?query");
    // assertEQ(gaogao.encodedSSP, "//gg:gaogao@www.baidu.com:99/path/path?query");
    // gaogao.ssp = null;
    // gaogao.encodedSSP = null;
    // assertEQ(gaogao.ssp, "//gg:gaogao@www.baidu.com:99/path/path?query");
    // assertEQ(gaogao.encodedSSP, "//gg:gaogao@www.baidu.com:99/path/path?query");
}

function main(): int {
    let suite = new ArkTestsuite("URI UT tests");

    suite.addTest("SetSchemeTest", SetSchemeTest);
    suite.addTest("SetUserInfoTest", SetUserInfoTest);
    suite.addTest("SetPathTest", SetPathTest);
    suite.addTest("SetQueryTest", SetQueryTest);
    suite.addTest("SetFragmentTest", SetFragmentTest);
    suite.addTest("SetAuthorityTest", SetAuthorityTest);
    suite.addTest("SetSspTest", SetSspTest);
    suite.addTest("SetEncodedUserInfo", SetEncodedUserInfo);
    suite.addTest("SetEncodedPathTest", SetEncodedPathTest);
    suite.addTest("SetEncodedQuery", SetEncodedQuery);
    suite.addTest("SetEncodedFragment", SetEncodedFragment);
    suite.addTest("SetEncodedAuthority", SetEncodedAuthority);
    suite.addTest("SetEncodedSsp", SetEncodedSsp);
    suite.addTest("UriSetNullAttribute", UriSetNullAttribute);
    //passed
    return suite.run();
}
