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
desc: Generic class wide to narrow constraint is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for methodT(Container<S>)
    - >-
        Semantic error ESE8745: Type parameter 'T' can not be inferred for the call to 'methodT'.
        Specify the type argument explicitly.
    - >-
        Semantic error ESE0046: Type 'Container<S>' is not compatible with type 'Container<T>' at
        index 1
---*/

'use static'

/*
 * TC-GC-001: 类本身泛型-宽传窄
 * Type argument 'S' should be a subtype of 'B' constraint
 * No matching call signature for methodT
 * 预期结果: CTE编译错误
 * 关联测试点: 泛型类-本身
 */

class Container<S> {
  public value: S

  constructor(value: S) {
    this.value = value
  }
}

class A {
  public value: string = ''
}

class B extends A {
  public name: string = ''
}

function methodS<S extends A>(c: Container<S>): void {
  methodT(c)
}

function methodT<T extends B>(c: Container<T>): void {
  console.log(c.value.toString())
}

function main(): void {
  let c: Container<A> = new Container<A>(new A())
  methodS(c)
}
