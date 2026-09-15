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
desc: Multilevel generic call wide to narrow constraint is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for methodB(S)
    - >-
        Semantic error ESE8745: Type parameter 'T' can not be inferred for the call to 'methodB'.
        Specify the type argument explicitly.
    - >-
        Semantic error ESE0046: Type 'S' is not compatible with type 'T' at index 1
---*/

'use static'

/*
 * TC-GC-007: 多层泛型调用-宽传窄
 * Type argument 'S' should be a subtype of 'B' constraint
 * No matching call signature for methodB
 * 预期结果: CTE编译错误
 * 关联测试点: 多层-宽传窄
 */

class A {
  public value: string = ''
}

class B extends A {
  public name: string = ''
}

class C extends B {
  public id: int = 0
}

function methodA<S extends A>(a: S): string {
  return methodB(a)
}

function methodB<T extends B>(b: T): string {
  return b.toString()
}

function main(): void {
  let c: C = new C()
  methodA(c)
}
