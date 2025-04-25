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

function ConstructorTest033() {
  let uri = new uri.UriEntry("/username/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "");
}

function ConstructorTest034() {
  let uri = new uri.UriEntry("http:/userna^me:password@www.baidu.com:99/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "");
}

function ConstructorTest035() {
  let uri = new uri.UriEntry("http://?query#fagment");
  assertEQ(uri.getScheme(), "http");
}

function ConstructorTest036() {
  let uri = new uri.UriEntry("http:/username:password@www.baidu.com:99/pa^th/path?query#fagment");
  assertEQ(uri.getErrStr(), "");
}

function ConstructorTest037() {
  let uri = new uri.UriEntry("http:/username:password@www.baidu.com:9^9/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "");
}

function ConstructorTest038() {
  let uri = new uri.UriEntry("http:/username:password@[1:0:0:1:2:1:2:1]:9^9/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "");
}

function ConstructorTest039() {
  let uri = new uri.UriEntry("http:/username:password@[1:0:0:1:2:1:2:1/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "");
}

function ConstructorTest040() {
  let uri = new uri.UriEntry("http://gg:gaogao@[1:0:0:sfvs1:2:1:2:1]:99/path1?query#fagment");
  assertEQ(uri.getErrStr(), "Ipv6 does not conform to the rule.");
}
function ConstructorTest041() {
  let uri = new uri.UriEntry("http、:/username:password@www.baidu.com:99/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "Scheme does not conform to the rule.");
}

function ConstructorTest042() {
  let uri = new uri.UriEntry("http://[ffff::1]:、99/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "Port does not conform to the rule.");
}

function ConstructorTest043() {
  let uri = new uri.UriEntry("http://[ffff::1:99/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "IPv6 is missing a closing bracket.");
}

function ConstructorTest044() {
  let uri = new uri.UriEntry("http://ffff::1]:99/path/path?query#fagment");
  assertEQ(uri.getErrStr(), "Host does not conform to the rule.");
}

 

function main(): int {
  let suite = new ArkTestsuite("URI UT tests");
  suite.addTest("ConstructorTest033", ConstructorTest033);
  suite.addTest("ConstructorTest034", ConstructorTest034);
  suite.addTest("ConstructorTest035", ConstructorTest035);
  suite.addTest("ConstructorTest036", ConstructorTest036);
  suite.addTest("ConstructorTest037", ConstructorTest037);
  suite.addTest("ConstructorTest038", ConstructorTest038);
  suite.addTest("ConstructorTest039", ConstructorTest039);
  suite.addTest("ConstructorTest040", ConstructorTest040);
  suite.addTest("ConstructorTest041", ConstructorTest041);
  suite.addTest("ConstructorTest042", ConstructorTest042);
  suite.addTest("ConstructorTest043", ConstructorTest043);
  suite.addTest("ConstructorTest044", ConstructorTest044);

  //passed
  return suite.run();
}