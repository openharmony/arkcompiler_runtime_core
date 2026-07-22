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
    After reverting Any bytecode support, removing a class type parameter changes
    the effective type from T(=Any) to Object, but both produce the same bytecode
    signature, so this change no longer affects binary compatibility.
files: ['./remove_type_parameter_n.libr.ets']
---*/

import * as LIB from './remove_type_parameter_n.libr.ets'

function main(): void {
    let c1 = new LIB.Container<string>("hello");
    arktest.assertEQ(c1.get(), "hello")
}
