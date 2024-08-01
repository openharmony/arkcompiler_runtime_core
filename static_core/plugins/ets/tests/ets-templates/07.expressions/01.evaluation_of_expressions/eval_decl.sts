/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
desc: >-
  An expression may occur:
  - inside a declaration of type (class or interface), i.e. in a field initializer,
  static initializer, constructor declaration, method declaration or annotation;
---*/

type Incrementor = (i: int) => int;
let sg: int;
let ig: int = new Int(1).unboxed();

class A {
  public fld: int = 42 + (sg = "abc".length);
  private f: Incrementor;

  public constructor(f: Incrementor) {
      this.f = f;
  }

  public meth(): int {
      return this.f(sg);
  }

  public static i: int;

  static {
    A.i = 12 + ig;
  }
}

function inc(p: int): int {
    return p + 1;
}

function main(): int {
    let a: A = new A(inc);
    if (a.fld != 45 || A.i != 13 || a.meth() != 4 || sg != 3) return 1;
    return 0;
}
