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
desc: Generic interface multiple instantiations is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0181: Implements generic interface 'A<I,T>' with different instantiations.
---*/

'use static'

interface A<I extends A<I, T>, T> { foo(x: I): T }
interface C<T> extends A<C<T>, T> { foo(x: C<T>): T }
interface B<R> extends A<B<R>, R> { foo(x: B<R>): R }
interface Foo extends C<Int>, B<Double> {}
