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
desc: >-
    Test function references across packages.
    Covers ETSemitter GenAnnotationFunctionalReference (FUNCTIONAL_REFERENCE).
files: [lib_funcs.ets]
name: test_import
arktsconfig: arktsconfig-import.json
es2panda_options: ['read-metadata']
expected_out:
    runtime:
        - "10"
        - "21"
        - "-3"
---*/

import { MathOps, applyOp } from '@panda/lib/lib_funcs'

let fn: (x: int) => int = MathOps.twice
console.log(fn(5))
console.log(applyOp(7, MathOps.triple))
console.log(applyOp(3, MathOps.negate))
