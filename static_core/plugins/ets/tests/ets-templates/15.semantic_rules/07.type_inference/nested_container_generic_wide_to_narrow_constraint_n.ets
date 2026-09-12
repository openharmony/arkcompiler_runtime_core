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
desc: Nested container generic wide to narrow constraint is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for methodT(Map<String,Array<S>>)
    - >-
        Semantic error ESE8745: Type parameter 'T' can not be inferred for the call to 'methodT'.
        Specify the type argument explicitly.
    - >-
        Semantic error ESE0046: Type 'Map<String,Array<S>>' is not compatible with type
        'Map<String,Array<T>>' at index 1
---*/

'use static'

/*
 * TC-GM-003: 嵌套容器泛型
 * Type argument 'S' should be a subtype of 'B' constraint
 * No matching call signature for methodT
 * 预期结果: CTE编译错误
 * 关联测试点: 容器-嵌套
 */

class A {
  public value: string = ''
}

class B extends A {
  public name: string = ''
}

function methodS<S extends A>(m: Map<string, S[]>): void {
  methodT(m)
}

function methodT<T extends B>(m: Map<string, T[]>): void {
  let arr: T[] | undefined = m.get('key')
  if (arr !== undefined && arr.length > 0) {
    console.log(arr[0].toString())
  }
}

function main(): void {
  let m: Map<string, A[]> = new Map<string, A[]>()
  m.set('key', [new A()])
  methodS(m)
}
