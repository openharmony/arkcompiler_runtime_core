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
desc: Generic interface inheritance constraint is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for useB(T)
    - >-
        Semantic error ESE8745: Type parameter 'T' can not be inferred for the call to 'useB'.
        Specify the type argument explicitly.
    - >-
        Semantic error ESE0046: Type 'T' is not compatible with type 'T' at index 1
---*/

'use static'

/*
 * TC-GI-003: 接口继承链泛型约束
 * Type argument 'T' should be a subtype of 'B<Double>' constraint
 * No matching call signature for useB
 * 预期结果: CTE编译错误
 * 关联测试点: 泛型接口-继承
 */

interface A<T> {
  getValue(): T
}

interface B<T> extends A<T> {
  getName(): T
}

class Data {
  public value: number = 0
}

class BImpl implements B<number> {
  public getValue(): number {
    return 42
  }
  public getName(): number {
    return 100
  }
}

function process<T extends A<number>>(input: T): void {
  useB(input)
}

function useB<T extends B<number>>(input: T): void {
  let v: number = input.getValue()
  console.log(v.toString())
}

function main(): void {
  let b: B<number> = new BImpl()
  process(b)
}
