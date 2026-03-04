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
    Checking that union of generic custom types can be used as
    type for lambda function parameters
---*/

abstract class A<U, V> {}

abstract class B1<U, V> extends A<U, V> {}

class B2<U, V> extends A<U, V> {
  b2_v: U | V
  constructor (p: U | V) {
    this.b2_v = p
  }
}

class C1<U, V> extends B1<U, V> {
  c1_v: U | V
  constructor (p: U | V) {
    this.c1_v = p
  }
}

class C2<U, V> extends B1<U, V> {
  c2_v: U | V
  constructor (p: U | V) {
    this.c2_v = p
  }
}

class Other<T> {
  other_v: T
  constructor (p: T) {
    this.other_v = p
  }
}

function main(): void {
  let check = (v: C1<int, null> | C2<number, string> | B2<bigint, undefined> |
                Other<string> | bigint,
                v_id: int) => {
    if (v_id == 1) {
      arktest.assertTrue(v instanceof C1)
      let a: C1<int, null> = v as C1<int, null>
      arktest.assertEQ(a.c1_v, 15)
    }
    else if (v_id == 2) {
      arktest.assertTrue(v instanceof C2)
      let a: C2<number, string> = v as C2<number, string>
      arktest.assertEQ(a.c2_v, "abc")
    }
    else if (v_id == 3) {
      arktest.assertTrue(v instanceof B2)
      let a: B2<bigint, undefined> = v as B2<bigint, undefined>
      arktest.assertEQ(a.b2_v, 5n)
    }
    else if (v_id == 4) {
      arktest.assertTrue(v instanceof Other)
      let a: Other<string> = v as Other<string>
      arktest.assertEQ(a.other_v, "xyz")
    }
    else if (v_id == 5) {
      arktest.assertTrue(v instanceof bigint)
      arktest.assertEQ(v, 17n)
    }
  }

  check(new C1<int, null>(15), 1)
  check(new C2<number, string>("abc"), 2)
  check(new B2<bigint, undefined>(5n), 3)
  check(new Other<string>("xyz"), 4)
  check(17n, 5)
}
