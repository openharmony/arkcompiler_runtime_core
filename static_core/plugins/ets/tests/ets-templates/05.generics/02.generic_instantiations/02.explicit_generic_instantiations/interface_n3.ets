/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: In an implicit instantiation, type arguments are not specified explicitly. Such
  type arguments are inferred from the context the generic is referred in.
name: 05.generics/02.generic_instantiations/02.explicit_generic_instantiations/interface_n2
tags:
- compile-only
- negative
---*/

interface A<T>{
    a: T
}

interface B<T, U> extends A<T>{
    b: U
}

function main() {
    let i: I<number, string> = {a: 1, b: "abc"}
    assertEQ(typeof i.a, 'number')
    assertEQ(typeof i.b, 'string')
}
