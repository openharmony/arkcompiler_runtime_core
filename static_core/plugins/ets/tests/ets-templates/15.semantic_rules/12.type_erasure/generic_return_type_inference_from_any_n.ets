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
desc: Generic return type inference from any is rejected at compile time.
tags: [compile-only, negative]
expected_out:
  compiler:
    - >-
        Semantic error ESE0127: No matching call signature for foo(Any)
    - >-
        Semantic error ESE8745: Type parameter 'E' can not be inferred for the call to 'foo'.
        Specify the type argument explicitly.
---*/

'use static'

function foo<E>(arg: Any): E[] {
  return arg as E[]
}

function bar(arg: Any) {
  const x = foo(arg)  // must be CTE
  console.log(x.length)
}

function main() {
  const arr: string[] = ['']
  bar(arr)
}
