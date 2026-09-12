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
desc: Mixed class interface generic constraint is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for methodS(C)
    - >-
        Semantic error ESE8745: Type parameter 'S' can not be inferred for the call to 'methodS'.
        Specify the type argument explicitly.
    - >-
        Semantic error ESE0046: Type 'C' is not compatible with type 'S' at index 1
---*/

'use static'

/*
 * TC-GC-013: 混合类型-类继承类+接口
 * Type argument 'C' should be a subtype of 'A' constraint
 * No matching call signature for methodS
 * 预期结果: CTE编译错误
 * 关联测试点: 混合-类+接口
 */

class A {
  public value: string = ''
}

interface B {
  getName(): string
}

class C implements B {
  public value: string = ''
  public getName(): string {
    return 'name'
  }
}

function methodS<S extends A>(a: S): string {
  return a.toString()
}

function main(): void {
  let c: C = new C()
  methodS(c)
}
