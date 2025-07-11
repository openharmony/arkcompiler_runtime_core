/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
    A function can be declared in, or imported to a scope.
    The semantic check for these functions is as follows:
    • If signatures of functions are overload-equivalent, then a compile-time error occurs.
    • Otherwise, overloading is valid.
    More details can be found in Function Overloading and in Import and Overloading of Function Names.
files:
- ./modules/function_export_all.ets
---*/

import {foo} from "./modules/function_export_all.ets"
import {TPL} from "./modules/function_export_all.ets"

function main(): void {
    let tpl: TPL = [new Number(), new String()];

    arktest.assertEQ( foo(true), 2 );
    arktest.assertEQ( foo(new Boolean()), 2 );
    arktest.assertEQ( foo(c'A'), 4 );
    arktest.assertEQ( foo(new Char()), 4 );
    arktest.assertEQ( foo(1 as short), 6 );
    arktest.assertEQ( foo(new Short()), 6 );
    arktest.assertEQ( foo(1 as int), 8 );
    arktest.assertEQ( foo(new Int()), 8 );
    arktest.assertEQ( foo(1 as long), 10 );
    arktest.assertEQ( foo(new Long()), 10 );
    arktest.assertEQ( foo(1.1 as float), 12 );
    arktest.assertEQ( foo(new Float()), 12 );
    arktest.assertEQ( foo(1.2 as double), 14 );
    arktest.assertEQ( foo(new Number()), 14 );
    arktest.assertEQ( foo("knock-knock"), 16 );
    arktest.assertEQ( foo(new Object()), 17 );
    arktest.assertEQ( foo(new number[2]), 18 );
    arktest.assertEQ( foo(tpl), 19 );
    arktest.assertEQ( foo<Number, String>(new Number(), new String()), 20 );
    arktest.assertEQ( foo( () => { return false; } ), 21 );
    arktest.assertEQ( foo(1, 1.2, "who`s there?" ), 22 );
}
