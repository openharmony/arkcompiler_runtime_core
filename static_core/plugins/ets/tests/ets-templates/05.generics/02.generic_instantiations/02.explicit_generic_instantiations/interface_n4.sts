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
tags:
- compile-only
- negative
---*/

interface A {}
interface B extends A {}

interface I<T extends U, U extends A> {
    a: T
    b: U
}

function main() {
    let i1: I<A, A> = new I<A, A>(new A(), new A())
    let i2: I<B, A> = new I<B, A>(new B(), new A())
    let i3: I<B, B> = new I<B, B>(new B(), new B())
}
