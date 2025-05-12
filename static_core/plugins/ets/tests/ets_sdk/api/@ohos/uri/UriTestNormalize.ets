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


function testNormalize001() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path/66./../././mm/.././path1?query#fagment');
  let res = gaogao.normalize();
  assertEQ(res.path, "/path/path1");
  assertEQ(res.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/path/path1?query#fagment");
}

function testNormalize002() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path?query#fagment');
  let res = gaogao.normalize();
  assertEQ(res.path, "/../../path");
  assertEQ(res.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path?query#fagment");
}

function testNormalize003() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path/.././../aa/bb/cc?query#fagment');
  let res = gaogao.normalize();
  assertEQ(res.path, "/../../../aa/bb/cc");
  assertEQ(res.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../../aa/bb/cc?query#fagment");
}

function testNormalize004() {
  let gaogao = new uri.URI('http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../path/.././../aa/bb/cc?query');
  let res = gaogao.normalize();
  assertEQ(res.path, "/../../aa/bb/cc");
  assertEQ(res.toString(), "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../aa/bb/cc?query");
}

function NormalizeTest001() {
  let uri = new uri.UriEntry("http://user@[1:0:0:1:2:1:2:1]:99/path/66./../././mm/.././path1?query#fagment");
  let normalize = uri.normalize();
  assertEQ(normalize, "http://user@[1:0:0:1:2:1:2:1]:99/path/path1?query#fagment");
}

function NormalizeTest002() {
  let uri = new uri.UriEntry("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path?query#fagment");
  let normalize = uri.normalize();
  assertEQ(normalize, "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path?query#fagment");
}

function NormalizeTest003() {
  let uri = new uri.UriEntry("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../path/.././../aa/bb/cc?query#fagment");
  let normalize = uri.normalize();
  assertEQ(normalize, "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99/../../../aa/bb/cc?query#fagment");
}

function NormalizeTest004() {
  let uri = new uri.UriEntry("http://gg:gaogao@[1:0:0:1:2:1:2:1]:99?query#fagment");
  let normalize = uri.normalize();
  assertEQ(normalize, "http://gg:gaogao@[1:0:0:1:2:1:2:1]:99?query#fagment");
}

function main(): int {
  let suite = new ArkTestsuite("URI Normalize tests");
  suite.addTest("testNormalize001", testNormalize001);
  suite.addTest("testNormalize002", testNormalize002);
  suite.addTest("testNormalize003", testNormalize003);
  suite.addTest("testNormalize004", testNormalize004);
  
  suite.addTest("NormalizeTest001", NormalizeTest001);
  suite.addTest("NormalizeTest002", NormalizeTest002);
  suite.addTest("NormalizeTest003", NormalizeTest003);
  suite.addTest("NormalizeTest004", NormalizeTest004);
  //passed
  return suite.run();
}