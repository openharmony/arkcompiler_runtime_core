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
desc: Verify whether this cannot be used as a left-hand side value.
tags: [compile-only, negative]
expected_out:
    - "Semantic error ESE0025: Invalid left-hand side of assignment expression"
    - "Syntax error ESY0072: Invalid left-hand side in assignment expression."
---*/

class MyClass {
  static foo(input: MyClass): MyClass {  return input }
  method(): void {
    this = MyClass.foo(this)
  }
}
