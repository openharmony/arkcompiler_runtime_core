/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

/*---
desc: 13.compilation_units\13.03.import_directives\issue_15182_04
name: issue_15182_04.ets
tags:
files: ['./issue_15182_01.ets', './issue_15182_02.ets', './issue_15182_03.ets']
---*/

import { Test1, Test2, Test3 } from "./issue_15182_03.ets"
import { int32 } from "./issue_15182_03.ets"

function main(): void {
  let t1 = new Test1();
  let t2 = new Test2();
  let t3 = new Test3();

  assertTrue( t1 instanceof Test1)
  assertTrue( t2 instanceof Test2)
  assertTrue( t3 instanceof Test3)
}
