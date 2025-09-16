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

function testAttributeNull() {
  let gaogao = new uri.URI('/16.9.5.4:99');
  arktest.assertEQ(gaogao.scheme, null);
  arktest.assertEQ(gaogao.port, "-1");
  arktest.assertEQ(gaogao.authority, null);
  arktest.assertEQ(gaogao.userInfo, null);
  arktest.assertEQ(gaogao.query, null);
  arktest.assertEQ(gaogao.fragment, null);
  arktest.assertEQ(gaogao.encodedAuthority, null);
  arktest.assertEQ(gaogao.encodedUserInfo, null);
  arktest.assertEQ(gaogao.encodedQuery, null);
  arktest.assertEQ(gaogao.encodedFragment, null);

  gaogao = new uri.URI('http://user@49.10pe8.54.12:80');
  arktest.assertEQ(gaogao.port, "-1");
  arktest.assertEQ(gaogao.userInfo, null);
  arktest.assertEQ(gaogao.host, null);
  arktest.assertEQ(gaogao.path, null);
  arktest.assertEQ(gaogao.query, null);
  arktest.assertEQ(gaogao.fragment, null);
  arktest.assertEQ(gaogao.encodedUserInfo, null);
  arktest.assertEQ(gaogao.encodedPath, null);
  arktest.assertEQ(gaogao.encodedQuery, null);
  arktest.assertEQ(gaogao.encodedFragment, null);
}

function main(): int {
  let suite = new arktest.ArkTestsuite("URI UT tests");
  suite.addTest("testAttributeNull", testAttributeNull);
  //passed
  return suite.run();
}
