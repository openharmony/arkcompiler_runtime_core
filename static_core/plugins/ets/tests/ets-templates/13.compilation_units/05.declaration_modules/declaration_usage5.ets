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
desc: A namespace declaration introduces the qualified name to be used as a qualifier
  for access to each exported entity of a namespace. Next cases tests basic top-level
  declarations.
tags: [compile-only]
files:
- ./declaration_module.ets
---*/

import * as m from "./declaration_module.ets"

function main() {
    let aa = new m.A() // A instance is created
}
