/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: >-
    If used for import, a separate module is initialized only once with the details listed in Compilation Unit Initialization.
    The initialization process is performed in the following steps
    • If the separate module has variable or constant declarations (see Variable and Constant Declarations), then their
      initializers are executed to ensure that they all have valid initial values
    • If the separate module has top-level statements (see Top-Level Statements), then they are also executed.
files:
- ./modules/statements.ets
---*/

import {i} from "./modules/statements.ets";
import {arr} from "./modules/statements.ets";
import {str} from "./modules/statements.ets";
import {n} from "./modules/statements.ets";

import {cn} from "./modules/statements.ets";

function main(): void {
  assertEQ( i, 17 );
  assertEQ( str, "updated str" );
  assertEQ( n, 11.1 );
  assertEQ( cn, 2.5 );
}
