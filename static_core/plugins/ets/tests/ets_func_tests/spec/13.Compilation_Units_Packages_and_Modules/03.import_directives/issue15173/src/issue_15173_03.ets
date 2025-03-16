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
desc: 13.compilation_units\13.03import_directives\issue_15173_02
name: issue_15173_03.ets
tags:
files: ['./issue_15173_01.ets']
---*/

import {_foo} from './issue_15173_01'
import {_bar} from './issue_15173_01'

function main(): void {
  let foo = new _foo();
  let bar = new _bar();
  assertEQ( foo.test(), "inside a method")
  assertEQ( bar.test(), "inside a method")
}
