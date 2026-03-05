/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
desc: A compile-time error occurs if a native function has a body.
tags: [compile-only]
---*/

native function foo(a: Object, b?: string, ): int|float; // native with optional parameter
export native function foo2(b: string, a: int = 1): int; // native with default parameter and export directive
native function foo3(func: (i: int) => int): string // native with lambda parameter

function main() {

    let v = foo(new Object());
    let z: int = foo2('');
    const myLambda = (i: int) => 1
    foo3(myLambda)

}
