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
    Checking that intersection type I1 & I2 is subtype of type Any
    on non generic custom types
---*/

// type I1 = I
// type I2 = A

interface I {
  methI(): string
}

class A {
  methA() {
    return 'methA'
  }
}

class B extends A implements I {
  methI() {
    return 'methIB'
  }
}

class C implements I {
  methI() {
    return 'methIC'
  }
}

type I1 = I
type I2 = A

function check_value(v: Any, v_id: int): void {
  if (v_id == 1) {
    arktest.assertTrue(v instanceof I1)
    arktest.assertTrue(v instanceof I2)
    arktest.assertTrue(v instanceof I)
    arktest.assertTrue(v instanceof A)
    arktest.assertTrue(v instanceof B)
    arktest.assertFalse(v instanceof C)
  }
  else if (v_id == 2) {
    arktest.assertFalse(v instanceof I1)
    arktest.assertTrue(v instanceof I2)
    arktest.assertFalse(v instanceof I)
    arktest.assertTrue(v instanceof A)
    arktest.assertFalse(v instanceof B)
    arktest.assertFalse(v instanceof C)
  }
  else if (v_id == 3) {
    arktest.assertTrue(v instanceof I1)
    arktest.assertFalse(v instanceof I2)
    arktest.assertTrue(v instanceof I)
    arktest.assertFalse(v instanceof A)
    arktest.assertFalse(v instanceof B)
    arktest.assertTrue(v instanceof C)
  }
}

function check(v: I1 | I2, v_id: int): void {
  check_value(v, v_id)
  if (v instanceof I1) {
    check_value(v, v_id)
    if (v instanceof I2) {
      arktest.assertEQ(v_id, 1)
      check_value(v, v_id)
    }
    else {
      arktest.assertEQ(v_id, 3)
    }
  }
  else {
    arktest.assertEQ(v_id, 2)
  }
}

function main(): void {
  check(new B(), 1)
  check(new A(), 2)
  check(new C(), 3)
}
