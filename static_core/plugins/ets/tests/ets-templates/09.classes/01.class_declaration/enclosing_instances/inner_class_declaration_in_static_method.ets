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
desc: Declaration inner class in static mthod and invoke hin from another method.
assert: An instance of an inner class I whose declaration occurs in a static context has no lexically enclosing instances. However, if I is immediately declared within a static method or static initializer then I does have an enclosing block, which is the innermost block statement exically enclosing the declaration of I.
tags: [compile-only, negative]
---*/

class A {
  public static foo(): void {
    public inner class B {
      public x: int;
    }

    let b: B = new B();
  }

  public static foo(y: int): void {
    let b: B = new B();
  }
}
