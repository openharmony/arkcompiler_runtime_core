/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
desc: Direct indexed access through a value whose static type is Tuple is rejected at compile time.
tags: [compile-only, negative]
params: 'TEST-UNIT-03-0076; GAP-03-0108; GRC-03-0137; CI-03-17-02-006-01'
assertion: 'Compilation rejects direct element access through a value whose static type is Tuple.'
---*/

let concrete: [int, string] = [1, 'tuple']
let abstractTuple: Tuple = concrete
let element: Any = abstractTuple[0]
