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
desc: ES2PANDA_FAIL, TypeError, C type does not exist.
tags: [compile-only, negative]
files:
- ./declaration_module.ets
---*/

// Module describes C, but C not exported
import * as m from "./declaration_module.ets"

let c = new m.C // compile-time error as C is not exported
