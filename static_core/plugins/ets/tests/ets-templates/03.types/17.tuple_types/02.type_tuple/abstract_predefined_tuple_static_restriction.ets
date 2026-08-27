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
desc: The predefined abstract Tuple type cannot be directly instantiated.
tags: [compile-only, negative]
params: 'TEST-UNIT-03-0074; GAP-03-0106; GRC-03-0135; CI-03-17-02-001-01'
assertion: 'Compilation rejects direct construction of the abstract predefined Tuple type.'
---*/

let concrete: [int, string] = [1, 'tuple']
let abstractTuple: Tuple = concrete
let invalidAbstractTuple: Tuple = new Tuple()
