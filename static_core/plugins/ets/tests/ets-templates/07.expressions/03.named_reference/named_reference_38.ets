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

/*---
desc: Cross-module reference type alias.
files:
- ./named_reference_export10.ets
---*/

import {UserID} from './named_reference_export10.ets';

function main(): void {
  const id1: UserID = "user-001";
  const id2: UserID = 1001;

  assertEQ(id1, "user-001");
  assertEQ(id2, 1001);
}