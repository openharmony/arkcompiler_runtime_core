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
import {uri} from "@ohos.uri"

function IsAbsoluteTest001() {
  let uri = new uri.UriEntry("f/tp://username:password@www.baidu.com:88/path?query#fagment");
  let res: boolean = uri.isAbsolute();
  assertFalse(res);
}

function IsAbsoluteTest002() {
  let uri = new uri.UriEntry("ftp://username:password@www.baidu.com:88/path?query#fagment");
  let res: boolean = uri.isAbsolute();
  assertTrue(res);
}

function IsAbsoluteTest003() {
  let uri = new uri.UriEntry("htt/p://username:password@www.baidu.com:88/path?query#fagment");
  let res: boolean = uri.isAbsolute();
  assertFalse(res);
}

function IsRelativeTest001() {
  let uri = new uri.UriEntry("https://www.example.com/aaa");
  let res = uri.isRelative();
  assertFalse(res);
}

function IsRelativeTest002() {
  let uri = new uri.UriEntry("/bbb");
  let res = uri.isRelative();
  assertTrue(res);
}

function IsOpaqueTest001() {
  let uri = new uri.UriEntry("aaa:user@example.com");
  let res = uri.isOpaque();
  assertTrue(res);
}

function IsOpaqueTest002() {
  let uri = new uri.UriEntry("content://com.example/bbb");
  let res = uri.isOpaque();
  assertFalse(res);
}

function IsHierarchicalTest001() {
  let uri = new uri.UriEntry("https://www.example.com/path/to/resource");
  let res = uri.isHierarchical();
  assertTrue(res);
}

function IsHierarchicalTest002() {
  let uri = new uri.UriEntry("/path/to/resource");
  let res = uri.isHierarchical();
  assertTrue(res);
}

function IsHierarchicalTest003() {
  let uri = new uri.UriEntry("tel:123456789");
  let res = uri.isHierarchical();
  assertFalse(res);
}

function AddQueryValueTest001() {
  let uri = new uri.UriEntry("https://username:password@host:8080/file?aaa=1#myfragment");
  let temp = uri.addQueryValue("bbb", "2");
  assertEQ(temp, "https://username:password@host:8080/file?aaa=1&bbb=2#myfragment");
}

function AddQueryValueTest002() {
  let uri = new uri.UriEntry("mao:user@example.com");
  let temp = uri.addQueryValue("bb", "cc");
  assertEQ(temp, "mao:?bb=cc");
}

function ClearQueryTest001() {
  let uri = new uri.UriEntry("https://username:password@host:8080/file?aaa=1#myfragment");
  let temp = uri.clearQuery();
  assertEQ(temp, "https://username:password@host:8080/file#myfragment");
}


function main(): int {
  let suite = new ArkTestsuite("URI UT tests");


  suite.addTest("IsAbsoluteTest001", IsAbsoluteTest001);
  suite.addTest("IsAbsoluteTest002", IsAbsoluteTest002);
  suite.addTest("IsAbsoluteTest003", IsAbsoluteTest003);

  suite.addTest("IsRelativeTest001", IsRelativeTest001);
  suite.addTest("IsRelativeTest002", IsRelativeTest002);

  suite.addTest("IsOpaqueTest001", IsOpaqueTest001);
  suite.addTest("IsOpaqueTest002", IsOpaqueTest002);

  suite.addTest("IsHierarchicalTest001", IsHierarchicalTest001);
  suite.addTest("IsHierarchicalTest002", IsHierarchicalTest002);
  suite.addTest("IsHierarchicalTest003", IsHierarchicalTest003);

  suite.addTest("AddQueryValueTest001", AddQueryValueTest001);
  suite.addTest("AddQueryValueTest002", AddQueryValueTest002);

  suite.addTest("ClearQueryTest001", ClearQueryTest001);
  //passed
  return suite.run();
}