
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
  Checking the implementation of interface readonly field (2 tests: init field in constructor and direct init field).
---*/

class C {
  readonly data: int;
  constructor(a: int) {
    this.data = a
  } 
}

interface I {
  readonly field: C
}

// init field in constructor
class A0 implements I {
  readonly field: C
  constructor () {
    this.field = new C(5)
  }
}

function runTest0() {
  let a: A0 = new A0
  arktest.assertTrue(a.field instanceof C, "Expected C but was " + (typeof a.field))
  arktest.assertTrue(a.field.data instanceof int, "Expected int but was " + (typeof a.field.data))
  arktest.assertEQ(a.field.data, 5)
}

// direct init field
class A1 implements I {
  readonly field: C = new C(5)
}

function runTest1() {
  let a: A1 = new A1()
  arktest.assertTrue(a.field instanceof C, "Expected C but was " + (typeof a.field))
  arktest.assertTrue(a.field.data instanceof int, "Expected int but was " + (typeof a.field.data))
  arktest.assertEQ(a.field.data, 5)
}

function main(): int {
  const testSuite = new arktest.ArkTestsuite('readonly_field_interface.impl_interface_class_field_init');
  testSuite.addTest("constructor_field_init", runTest0)
  testSuite.addTest("direct_field_init", runTest1)
  return testSuite.run()
}

