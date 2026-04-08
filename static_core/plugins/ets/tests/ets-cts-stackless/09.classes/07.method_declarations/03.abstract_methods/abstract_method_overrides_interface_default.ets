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
desc: >-
    An abstract method can override non-abstract methods inherited from
    base classes or base interfaces.
---*/

class C {
  foo(): int {
    return 1
  }
}

interface I {
  foo(): int {  // default implementation
    return 2
  }
}

abstract class X extends C implements I {
  abstract foo(): int  // Here abstract foo() overrides both foo()
                       // coming from class C and interface I
}

class Y extends X {
  foo(): int {
    return 11
  }
}

function main(): void {
  let y: Y = new Y();
  arktest.assertEQ(y.foo(), 11);

  let x: X = y;
  arktest.assertEQ(x.foo(), 11);

  let c: C = y;
  arktest.assertEQ(c.foo(), 11);

  let i: I = y;
  arktest.assertEQ(i.foo(), 11);
}
