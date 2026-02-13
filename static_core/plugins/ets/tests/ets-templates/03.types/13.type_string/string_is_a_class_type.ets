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
    As a result, reference type semantics of type string
    highlights that this type is a class type.
---*/

function main(): int {
  let testSuite = new arktest.ArkTestsuite("string_is_a_class_type");

  testSuite.addTest("parameterless_constructor_1", () => {
    let a = new string
    let b: string = "efg"
    const c: string = "ijk"
    arktest.assertEQ(a.length, 0)
    arktest.assertTrue(a instanceof string)
    arktest.assertEQ(a, "")
    arktest.assertEQ("abc" + a, "abc")
    arktest.assertEQ(a + "abc", "abc")
    arktest.assertEQ(b + a, "efg")
    arktest.assertEQ(a + b, "efg")
    arktest.assertEQ(c + a, "ijk")
    arktest.assertEQ(a + c, "ijk")
  });

  testSuite.addTest("parameterless_constructor_2", () => {
    let a: string = new string
    let b: String = "efg"
    const c: String = "ijk"
    arktest.assertEQ(a.length, 0)
    arktest.assertTrue(a instanceof string)
    arktest.assertEQ(a, "")
    arktest.assertEQ("abc" + a, "abc")
    arktest.assertEQ(a + "abc", "abc")
    arktest.assertEQ(b + a, "efg")
    arktest.assertEQ(a + b, "efg")
    arktest.assertEQ(c + a, "ijk")
    arktest.assertEQ(a + c, "ijk")
  });

  testSuite.addTest("parameterless_constructor_3", () => {
    let a = new String()
    let b: string = "efg"
    const c: string = "ijk"
    arktest.assertEQ(a.length, 0)
    arktest.assertTrue(a instanceof string)
    arktest.assertEQ(a, "")
    arktest.assertEQ("abc" + a, "abc")
    arktest.assertEQ(a + "abc", "abc")
    arktest.assertEQ(b + a, "efg")
    arktest.assertEQ(a + b, "efg")
    arktest.assertEQ(c + a, "ijk")
    arktest.assertEQ(a + c, "ijk")
  });

  testSuite.addTest("parameterless_constructor_4", () => {
    let a: string = new String()
    let b: string = "efg"
    const c: string = "ijk"
    arktest.assertEQ(a.length, 0)
    arktest.assertTrue(a instanceof string)
    arktest.assertEQ(a, "")
    arktest.assertEQ("abc" + a, "abc")
    arktest.assertEQ(a + "abc", "abc")
    arktest.assertEQ(b + a, "efg")
    arktest.assertEQ(a + b, "efg")
    arktest.assertEQ(c + a, "ijk")
    arktest.assertEQ(a + c, "ijk")
  });

  testSuite.addTest("constructor_with_string_1", () => {
    let a = new string("123")
    let b: string = "efg"
    const c: string = "ijk"
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof string)
    arktest.assertEQ(a, "123")
    arktest.assertEQ("abc" + a, "abc123")
    arktest.assertEQ(a + "abc", "123abc")
    arktest.assertEQ(b + a, "efg123")
    arktest.assertEQ(a + b, "123efg")
    arktest.assertEQ(c + a, "ijk123")
    arktest.assertEQ(a + c, "123ijk")
  });

  testSuite.addTest("constructor_with_string_2", () => {
    let a = new String("123")
    let b: string = "efg"
    const c: string = "ijk"
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof string)
    arktest.assertEQ(a, "123")
    arktest.assertEQ("abc" + a, "abc123")
    arktest.assertEQ(a + "abc", "123abc")
    arktest.assertEQ(b + a, "efg123")
    arktest.assertEQ(a + b, "123efg")
    arktest.assertEQ(c + a, "ijk123")
    arktest.assertEQ(a + c, "123ijk")
  });

  return testSuite.run();
}
