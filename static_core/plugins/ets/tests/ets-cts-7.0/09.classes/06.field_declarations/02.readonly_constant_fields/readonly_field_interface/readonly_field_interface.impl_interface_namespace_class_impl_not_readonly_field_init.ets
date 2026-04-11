
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
  Checking the implementation of interface readonly field (namespace, implementation as not readonly, 2 tests: init field in constructor and direct init field).
  If a property of interface is defined as readonly, then the implementation of the property can be readonly or not readonly.
---*/

// init field in constructor
namespace testNS0 {
  class C {
    readonly data: int;
    constructor(a: int) {
      this.data = a
    } 
  }

  interface I {
    readonly field: C
  }

  class A implements I {
    field: C
    constructor () {
      this.field = new C(5)
    }
  }

  export function runTest() {
    let a: A = new A()
    arktest.assertTrue(a.field instanceof C, "Expected C but was " + (typeof a.field))
    arktest.assertTrue(a.field.data instanceof int, "Expected int but was " + (typeof a.field.data))
    arktest.assertEQ(a.field.data, 5)
  }
}

// direct init field
namespace testNS1 {
  class C {
    readonly data: int;
    constructor(a: int) {
      this.data = a
    } 
  }

  interface I {
    readonly field: C
  }

  class A implements I {
    field: C = new C(5)
  }

  export function runTest() {
    let a: A = new A()
    arktest.assertTrue(a.field instanceof C, "Expected C but was " + (typeof a.field))
    arktest.assertTrue(a.field.data instanceof int, "Expected int but was " + (typeof a.field.data))
    arktest.assertEQ(a.field.data, 5)
  }
}

function main(): int {
  const testSuite = new arktest.ArkTestsuite('readonly_field_interface.impl_interface_namespace_class_impl_not_readonly_field_init');
  testSuite.addTest("testNS0_test_constructor_field_init", testNS0.runTest)
  testSuite.addTest("testNS1_test_direct_field_init", testNS1.runTest)
  return testSuite.run()
}

