/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Declaring constructor with return statement with expression.
assert: >-
    A return statement may be used in the body of a constructor if it does not include an expression.
    Except for the possibility of explicit constructor invocations, and the prohibition on explicitly
    returning a value, the body of a constructor is like the body of a method.
tags: [compile-only, negative]
---*/

class A {
  public x: int;

  public constructor() {
    this.x = 5;
    return this;
  }
}
