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
    After reverting Any bytecode support, replacing a type parameter name Object
    (which acts as unconstrained T with Any default) with T extends Object changes
    the effective type, but both produce the same bytecode signature, so this change
    no longer affects binary compatibility.
files: ['./add_type_param_constraint_2_n.libr.ets']
---*/

import * as LIB from './add_type_param_constraint_2_n.libr.ets'

function main(): void {
    let c = new LIB.C<string>("test")
    arktest.assertEQ(c.get(), "test")
}
