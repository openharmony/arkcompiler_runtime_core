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
desc: Generic class method wide to narrow constraint is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for handle(S)
    - >-
        Semantic error ESE8745: Type parameter 'T' can not be inferred for the call to 'handle'.
        Specify the type argument explicitly.
    - >-
        Semantic error ESE0046: Type 'S' is not compatible with type 'T' at index 1
---*/

'use static'

/*
 * TC-GC-002: 类方法泛型-宽传窄
 * Type argument 'S' should be a subtype of 'B' constraint
 * No matching call signature for handle
 * 预期结果: CTE编译错误
 * 关联测试点: 泛型类-方法
 */

class Manager {
  public process<S extends A>(s: S): void {
    this.handle(s)
  }

  public handle<T extends B>(t: T): void {
    console.log(t.toString())
  }
}

class A {
  public value: string = ''
}

class B extends A {
  public name: string = ''
}

function main(): void {
  let m: Manager = new Manager()
  let a: A = new A()
  m.process(a)
}
