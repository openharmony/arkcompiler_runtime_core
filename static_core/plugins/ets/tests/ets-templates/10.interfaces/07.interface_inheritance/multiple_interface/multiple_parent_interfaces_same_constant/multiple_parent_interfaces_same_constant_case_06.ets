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
desc: Verify that constants with the same name in a module and a namespace do not conflict.
files:
- ./moduleA.ets
- ./namespaceA.ets
---*/

import { CONSTANT_A } from "./moduleA.ets";
import { NamespaceA } from "./namespaceA.ets";

function main(): void {
    assertEQ(CONSTANT_A, "ValueA");
    assertEQ(NamespaceA.CONSTANT_A, "ValueA");
}