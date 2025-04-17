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

function ConstructorTest001() {
  let uri = new uri.UriEntry("http://username:password@www.baidu.com:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@www.baidu.com:99");
  assertEQ(uri.getSsp(), "//username:password@www.baidu.com:99/path/path?query");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "www.baidu.com");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest002() {
  let uri = new uri.UriEntry("http://username:password@[1080::8:800:200C:417A]:99/path/66path1?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@[1080::8:800:200C:417A]:99");
  assertEQ(uri.getSsp(), "//username:password@[1080::8:800:200C:417A]:99/path/66path1?query");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "[1080::8:800:200C:417A]");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/66path1");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest003() {
  let uri = new uri.UriEntry("http://username:password@[::]:88/path/path66?foooo#gaogao");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@[::]:88");
  assertEQ(uri.getSsp(), "//username:password@[::]:88/path/path66?foooo");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "[::]");
  assertEQ(uri.getPort(), 88);
  assertEQ(uri.getPath(), "/path/path66");
  assertEQ(uri.getQuery(), "foooo");
  assertEQ(uri.getFragment(), "gaogao");
}

function ConstructorTest004() {
  let uri = new uri.UriEntry("http://username:password@[1:0:0:1:2:1:2:1]:99/path/66path1?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@[1:0:0:1:2:1:2:1]:99");
  assertEQ(uri.getSsp(), "//username:password@[1:0:0:1:2:1:2:1]:99/path/66path1?query");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "[1:0:0:1:2:1:2:1]");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/66path1");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest005() {
  let uri = new uri.UriEntry("http://username:password@[::FFFF:129.144.52.38]:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@[::FFFF:129.144.52.38]:99");
  assertEQ(uri.getSsp(), "//username:password@[::FFFF:129.144.52.38]:99/path/path?query");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "[::FFFF:129.144.52.38]");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest006() {
  let uri = new uri.UriEntry("http://username:password@[::192.9.5.5]:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@[::192.9.5.5]:99");
  assertEQ(uri.getSsp(), "//username:password@[::192.9.5.5]:99/path/path?query");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "[::192.9.5.5]");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest007() {
  let uri = new uri.UriEntry("http://username:password@[22::22:2:2%ss]:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@[22::22:2:2%ss]:99");
  assertEQ(uri.getSsp(), "//username:password@[22::22:2:2%ss]:99/path/path?query");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "[22::22:2:2%ss]");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest008() {
  let uri = new uri.UriEntry("http://username:password@[fe80:0000:0001:0000:0440:44ff:1233:5678]:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@[fe80:0000:0001:0000:0440:44ff:1233:5678]:99");
  assertEQ(uri.getSsp(), "//username:password@[fe80:0000:0001:0000:0440:44ff:1233:5678]:99/path/path?query");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "[fe80:0000:0001:0000:0440:44ff:1233:5678]");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest009() {
  let uri = new uri.UriEntry("http://username:password@[fe80::0001:0000]:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@[fe80::0001:0000]:99");
  assertEQ(uri.getSsp(), "//username:password@[fe80::0001:0000]:99/path/path?query");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "[fe80::0001:0000]");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest010() {
  let uri = new uri.UriEntry("http://username:password@www.baidu.com:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@www.baidu.com:99");
  assertEQ(uri.getSsp(), "//username:password@www.baidu.com:99/path/path?query");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "www.baidu.com");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest011() {
  let uri = new uri.UriEntry("http://username:password@199.98.55.44:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "username:password@199.98.55.44:99");
  assertEQ(uri.getSsp(), "//username:password@199.98.55.44:99/path/path?query");
  assertEQ(uri.getUserinfo(), "username:password");
  assertEQ(uri.getHost(), "199.98.55.44");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest012() {
  let uri = new uri.UriEntry("http://16.9.5.4:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "16.9.5.4:99");
  assertEQ(uri.getSsp(), "//16.9.5.4:99/path/path?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "16.9.5.4");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest013() {
  let uri = new uri.UriEntry("http://[::168:169:333]:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "[::168:169:333]:99");
  assertEQ(uri.getSsp(), "//[::168:169:333]:99/path/path?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "[::168:169:333]");
  assertEQ(uri.getPort(), 99);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest014() {
  let uri = new uri.UriEntry("http://user@49.10hh8.54.12:80/path/path?query#qwer");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "user@49.10hh8.54.12:80");
  assertEQ(uri.getSsp(), "//user@49.10hh8.54.12:80/path/path?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "qwer");
}

function ConstructorTest015() {
  let uri = new uri.UriEntry("http://user@www.baidu.com/path/path?query#qwer");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "user@www.baidu.com");
  assertEQ(uri.getSsp(), "//user@www.baidu.com/path/path?query");
  assertEQ(uri.getUserinfo(), "user");
  assertEQ(uri.getHost(), "www.baidu.com");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "qwer");
}

function ConstructorTest016() {
  let uri = new uri.UriEntry("ftp://user@www.1hw.1com:77/path/path?query#qwer");
  assertEQ(uri.getScheme(), "ftp");
  assertEQ(uri.getAuthority(), "user@www.1hw.1com:77");
  assertEQ(uri.getSsp(), "//user@www.1hw.1com:77/path/path?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "qwer");
}

function ConstructorTest017() {
  let uri = new uri.UriEntry("http://user@hosthost/path/path?query#qwer");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "user@hosthost");
  assertEQ(uri.getSsp(), "//user@hosthost/path/path?query");
  assertEQ(uri.getUserinfo(), "user");
  assertEQ(uri.getHost(), "hosthost");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "qwer");
}

function ConstructorTest018() {
  let uri = new uri.UriEntry("http://user@[::]/path/path?query#qwer");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "user@[::]");
  assertEQ(uri.getSsp(), "//user@[::]/path/path?query");
  assertEQ(uri.getUserinfo(), "user");
  assertEQ(uri.getHost(), "[::]");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "qwer");
}

function ConstructorTest019() {
  let uri = new uri.UriEntry("http://[::192:0:5]/path/path?query#qwer");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "[::192:0:5]");
  assertEQ(uri.getSsp(), "//[::192:0:5]/path/path?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "[::192:0:5]");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "qwer");
}

function ConstructorTest020() {
  let uri = new uri.UriEntry("http://[::192:0:5]/path/path?query#qwer");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "[::192:0:5]");
  assertEQ(uri.getSsp(), "//[::192:0:5]/path/path?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "[::192:0:5]");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "qwer");
}

function ConstructorTest021() {
  let uri = new uri.UriEntry("http:/&username:password@[1080::8:800:200C:417A]:99/path/66path1?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "");
  assertEQ(uri.getSsp(), "/&username:password@[1080::8:800:200C:417A]:99/path/66path1?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "/&username:password@[1080::8:800:200C:417A]:99/path/66path1");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest022() {
  let uri = new uri.UriEntry("http:/[username:password@[::FFFF:129.144.52.38]:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "");
  assertEQ(uri.getSsp(), "/[username:password@[::FFFF:129.144.52.38]:99/path/path?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "/[username:password@[::FFFF:129.144.52.38]:99/path/path");
  assertEQ(uri.getQuery(), "query");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest023() {
  let uri = new uri.UriEntry("http:username:password@[1080::8:800:200C:417A]:99/path/66path1?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "");
  assertEQ(uri.getSsp(), "username:password@[1080::8:800:200C:417A]:99/path/66path1?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "");
  assertEQ(uri.getQuery(), "");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest024() {
  let uri = new uri.UriEntry("http:^$username:password@[::192.9.5.5]:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "");
  assertEQ(uri.getSsp(), "^$username:password@[::192.9.5.5]:99/path/path?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "");
  assertEQ(uri.getQuery(), "");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest025() {
  let uri = new uri.UriEntry("http:[?]username:password@[fe80::0001:0000]:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "http");
  assertEQ(uri.getAuthority(), "");
  assertEQ(uri.getSsp(), "[?]username:password@[fe80::0001:0000]:99/path/path?query");
  assertEQ(uri.getUserinfo(), "");
  assertEQ(uri.getHost(), "");
  assertEQ(uri.getPort(), -1);
  assertEQ(uri.getPath(), "");
  assertEQ(uri.getQuery(), "");
  assertEQ(uri.getFragment(), "fagment");
}

function ConstructorTest026() {
  let uri = new uri.UriEntry("");
  assertEQ(uri.getErrStr(), "Uri is empty.");
}

function ConstructorTest027() {
  let uri = new uri.UriEntry("#asd;");
  assertEQ(uri.getErrStr(), "#It can't be the first.");
}

function ConstructorTest028() {
  let uri = new uri.UriEntry("?sa^d:s#asd;");
  assertEQ(uri.getErrStr(), "Query does not conform to the rule.");
}

function ConstructorTest029() {
  let uri = new uri.UriEntry("?sad:s#a^sd;");
  assertEQ(uri.getErrStr(), "Fragment does not conform to the rule.");
}

function ConstructorTest030() {
  let uri = new uri.UriEntry("4http:/username:password@www.baidu.com:99/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "Scheme the first character must be a letter.");
}

function ConstructorTest031() {
  let uri = new uri.UriEntry("ht*tp:/username:password@www.baidu.com:99/path/path?query#fagment");
  assertEQ(uri.getScheme(), "ht*tp");
}

function ConstructorTest032() {
  let uri = new uri.UriEntry("/usern]ame/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "SpecialPath does not conform to the rule.");
}


function main(): int {
  let suite = new ArkTestsuite("URI UT tests");
  suite.addTest("ConstructorTest001", ConstructorTest001);
  suite.addTest("ConstructorTest002", ConstructorTest002);
  suite.addTest("ConstructorTest003", ConstructorTest003);
  suite.addTest("ConstructorTest004", ConstructorTest004);
  suite.addTest("ConstructorTest005", ConstructorTest005);
  suite.addTest("ConstructorTest006", ConstructorTest006);
  suite.addTest("ConstructorTest007", ConstructorTest007);
  suite.addTest("ConstructorTest008", ConstructorTest008);
  suite.addTest("ConstructorTest009", ConstructorTest009);
  suite.addTest("ConstructorTest010", ConstructorTest010);
  suite.addTest("ConstructorTest011", ConstructorTest011);
  suite.addTest("ConstructorTest012", ConstructorTest012);
  suite.addTest("ConstructorTest013", ConstructorTest013);
  suite.addTest("ConstructorTest014", ConstructorTest014);
  suite.addTest("ConstructorTest015", ConstructorTest015);
  suite.addTest("ConstructorTest016", ConstructorTest016);
  suite.addTest("ConstructorTest017", ConstructorTest017);
  suite.addTest("ConstructorTest018", ConstructorTest018);
  suite.addTest("ConstructorTest019", ConstructorTest019);
  suite.addTest("ConstructorTest020", ConstructorTest020);
  suite.addTest("ConstructorTest021", ConstructorTest021);
  suite.addTest("ConstructorTest022", ConstructorTest022);
  suite.addTest("ConstructorTest023", ConstructorTest023);
  suite.addTest("ConstructorTest024", ConstructorTest024);
  suite.addTest("ConstructorTest025", ConstructorTest025);
  suite.addTest("ConstructorTest026", ConstructorTest026);
  suite.addTest("ConstructorTest027", ConstructorTest027);
  suite.addTest("ConstructorTest028", ConstructorTest028);
  suite.addTest("ConstructorTest029", ConstructorTest029);
  suite.addTest("ConstructorTest030", ConstructorTest030);
  suite.addTest("ConstructorTest031", ConstructorTest031);
  suite.addTest("ConstructorTest032", ConstructorTest032);
  //passed
  return suite.run();
}