/*---
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
    Type string is a subtype of Object.
---*/

function check_conditional_statement(v: Object): boolean {
  if (v instanceof string) {
    arktest.assertEQ(v.length, 3)
    arktest.assertEQ(v, "123")
  }
  else {
    arktest.assertTrue(false,
      'v should be instance of string')
  }
  return true
}

function main(): int {
  let testSuite = new arktest.ArkTestsuite("string_is_a_subtype_of_object");

  testSuite.addTest("subtype_of_object", () => {
    let a: string = "123"
    arktest.assertTrue(a instanceof string)
    arktest.assertTrue(a instanceof Object)
    let b: Object = a
    arktest.assertTrue(b instanceof string)
    arktest.assertTrue(b instanceof Object)
  });

  testSuite.addTest("smart_type_variable_declaration_1", () => {
    let a: Object = new String("123")
    let b: string = "abc"
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof string)
    arktest.assertEQ(a, "123")
    arktest.assertEQ(b + a, "abc123")
    arktest.assertEQ(a + b, "123abc")
  });

  testSuite.addTest("smart_type_variable_declaration_2", () => {
    let base_str: string = "123"

    for (;;) {
      let a: Object = base_str
      let b: string = "abc"
      arktest.assertEQ(a.length, 3)
      arktest.assertTrue(a instanceof string)
      arktest.assertEQ(a, "123")
      arktest.assertEQ(b + a, "abc123")
      arktest.assertEQ(a + b, "123abc")
      break
    }
  });

  testSuite.addTest("smart_type_variable_assignment", () => {
    let base_str: string = "123"
    let a: Object
    let b: string = "abc"

    while (true) {
      a = base_str
      arktest.assertEQ(a.length, 3)
      arktest.assertTrue(a instanceof string)
      arktest.assertEQ(a, "123")
      arktest.assertEQ(b + a, "abc123")
      arktest.assertEQ(a + b, "123abc")
      break
    }
  });

  testSuite.addTest("smart_type_conditional_statement", () => {
    arktest.assertTrue(check_conditional_statement("123"))
  });

  return testSuite.run();
}
