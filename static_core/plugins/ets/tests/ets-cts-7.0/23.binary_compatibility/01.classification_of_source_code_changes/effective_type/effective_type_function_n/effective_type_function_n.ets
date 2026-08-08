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
    After reverting Any bytecode support, Any and Object have the same effective
    type at the bytecode level. Changing a parameter from Object to Any no longer
    affects binary compatibility.
files: ['./effective_type_function_n.libr.ets']
---*/

import { a } from './effective_type_function_n.libr.ets';

function main(): void {
    let obj: Object = new Error("abc")
    let x = a<Error>(obj)
    arktest.assertTrue(x instanceof Error)
    arktest.assertEQ(x.message, 'abc')
}
