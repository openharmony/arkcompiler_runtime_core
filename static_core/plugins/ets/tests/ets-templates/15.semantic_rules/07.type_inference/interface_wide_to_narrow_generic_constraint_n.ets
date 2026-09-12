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
desc: Interface wide to narrow generic constraint is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for methodT(S)
    - >-
        Semantic error ESE8745: Type parameter 'T' can not be inferred for the call to 'methodT'.
        Specify the type argument explicitly.
    - >-
        Semantic error ESE0046: Type 'S' is not compatible with type 'T' at index 1
---*/

'use static'

/*
 * TC-GC-005: 接口约束-宽约束传窄约束
 * Type argument 'S' should be a subtype of 'B' constraint
 * No matching call signature for methodT
 * 预期结果: CTE编译错误
 * 关联测试点: 接口-宽传窄
 */

interface A {
  getValue(): string
}

interface B extends A {
  getName(): string
}

interface C extends B {
  getId(): int
}

function methodS<S extends A>(a: S): string {
  return methodT(a)
}

function methodT<T extends B>(t: T): string {
  return t.getName()
}

function main(): void {
  let c: C = new CImpl()
  methodS(c)
}

class CImpl implements C {
  public getValue(): string {
    return ''
  }
  public getName(): string {
    return ''
  }
  public getId(): int {
    return 0
  }
}
