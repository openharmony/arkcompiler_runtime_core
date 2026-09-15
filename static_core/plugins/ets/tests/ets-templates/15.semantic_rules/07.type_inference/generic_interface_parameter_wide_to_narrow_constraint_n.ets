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
desc: Generic interface parameter wide to narrow constraint is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for processT(Processor<S>)
    - >-
        Semantic error ESE8745: Type parameter 'T' can not be inferred for the call to 'processT'.
        Specify the type argument explicitly.
    - >-
        Semantic error ESE0046: Type 'Processor<S>' is not compatible with type 'Processor<T>' at
        index 1
---*/

'use static'

/*
 * TC-GI-002: 接口方法参数泛型-宽传窄
 * Type argument 'S' should be a subtype of 'B' constraint
 * No matching call signature for processT
 * 预期结果: CTE编译错误
 * 关联测试点: 泛型接口-参数
 */

interface Processor<T> {
  process(input: T): void
}

class A {
  public value: string = ''
}

class B extends A {
  public name: string = ''
}

class ProcessorA implements Processor<A> {
  public process(input: A): void {
    console.log(input.value)
  }
}

function methodS<S extends A>(p: Processor<S>): void {
  processT(p)
}

function processT<T extends B>(p: Processor<T>): void {
}

function main(): void {
  let p: Processor<A> = new ProcessorA()
  methodS(p)
}
