/*---
Copyright (c) 2024-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License" );
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

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

    assertEQ( foo(true), 2 );
    assertEQ( foo(new Boolean()), 3 );
    assertEQ( foo(c'A'), 4 );
    assertEQ( foo(new Char()), 5 );
    assertEQ( foo(1 as short), 6 );
    assertEQ( foo(new Short()), 7 );
    assertEQ( foo(1 as int), 8 );
    assertEQ( foo(new Int()), 9 );
    assertEQ( foo(1 as long), 10 );
    assertEQ( foo(new Long()), 11 );
    assertEQ( foo(1.1 as float), 12 );
    assertEQ( foo(new Float()), 13 );
    assertEQ( foo(1.2 as double), 14 );
    assertEQ( foo(new Number()), 15 );
    assertEQ( foo("knock-knock"), 16 );
    assertEQ( foo(new Object()), 17 );
    assertEQ( foo(new Number[2]), 18 );
    assertEQ( foo(tpl), 19 );
    assertEQ( foo<Number, String>(new Number(), new String()), 20 );
    assertEQ( foo( () => { return false; } ), 21 );
    assertEQ( foo(1, 1.2, "who`s there?" ), 22 );
}
