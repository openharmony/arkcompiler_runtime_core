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
desc: Different return type in mA and mC declared by hiding .
assert: If a method declaration d1 with return type R1 overrides or hides the declaration of another method d2 with return type R2, then d1 must be return-type-substitutable for d2, or a compile-time error occurs.
tags: [compile-only, negative]
---*/

class A {
  public m(): D {
    return new D;
  }
}

class B {

}

class D {

}

class C extends A {
  public m(): B {
    return new D();
  }

}
