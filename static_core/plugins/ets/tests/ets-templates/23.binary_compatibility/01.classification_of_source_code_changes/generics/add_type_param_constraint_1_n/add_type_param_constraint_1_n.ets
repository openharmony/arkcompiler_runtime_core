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
    After reverting Any bytecode support, adding an Object constraint to an
    unconstrained type parameter T (whose default constraint is Any) changes the
    effective type from Any to Object, but both produce the same bytecode signature,
    so this change no longer affects binary compatibility.
files: ['./add_type_param_constraint_1_n.libr.ets']
---*/

import * as LIB from './add_type_param_constraint_1_n.libr.ets'

function main(): void {
    let c = new LIB.C<string>("test")
    arktest.assertEQ(c.get(), "test")
}
