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

{% for c in cases %}

/*---
desc: >-
    Fixed-size arrays preserve the element type during the Type Erasure
test_suffix: "{{c.test_suffix}}"
---*/

let expectedArrayStoreError = (e: Error): boolean => {
  return (e instanceof ArrayStoreError)
}

let expectedCastError = (e: Error): boolean => {
  return (e instanceof ClassCastError)
}

function check_with_object_and_cast_to_origin_type(v: Object): boolean {
    arktest.assertTrue(v instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(v instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(v instanceof {{c.fixed_array_check_wrong_type}})
    let a = v as FixedArray<{{c.element_type}}>
    arktest.expectThrow(() => {
      let b = v as {{c.fixed_array_cast_wrong_type}}
    }, expectedCastError)
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.check_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.check_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.check_val_2}})
    a[0] = {{c.new_val_0}}
    a[1] = {{c.new_val_1}}
    a[2] = {{c.new_val_2}}
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
    return true
}

function check_with_object_and_cast_to_superclass_type(v: Object): boolean {
    arktest.assertTrue(v instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(v instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(v instanceof {{c.fixed_array_check_wrong_type}})
    let a = v as {{c.fixed_array_check_superclass_type}}
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.check_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.check_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.check_val_2}})
    a[0] = {{c.new_val_0}}
    a[1] = {{c.new_val_1}}
    a[2] = {{c.new_val_2}}
{%- if c.new_wrong_val_0 %}
    arktest.expectThrow(() => {
      a[0] = {{c.new_wrong_val_0}}
    }, expectedArrayStoreError)
    arktest.expectThrow(() => {
      a[1] = {{c.new_wrong_val_1}}
    }, expectedArrayStoreError)
    arktest.expectThrow(() => {
      a[2] = {{c.new_wrong_val_2}}
    }, expectedArrayStoreError)
{%- endif %}
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
    return true
}

function check_with_any_and_cast_to_origin_type(v: Any): boolean {
    arktest.assertTrue(v instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(v instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(v instanceof {{c.fixed_array_check_wrong_type}})
    let a = v as FixedArray<{{c.element_type}}>
    arktest.expectThrow(() => {
      let b = v as {{c.fixed_array_cast_wrong_type}}
    }, expectedCastError)
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.check_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.check_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.check_val_2}})
    a[0] = {{c.new_val_0}}
    a[1] = {{c.new_val_1}}
    a[2] = {{c.new_val_2}}
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
    return true
}

function check_with_any_and_cast_to_superclass_type(v: Any): boolean {
    arktest.assertTrue(v instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(v instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(v instanceof {{c.fixed_array_check_wrong_type}})
    let a = v as {{c.fixed_array_check_superclass_type}}
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.check_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.check_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.check_val_2}})
    a[0] = {{c.new_val_0}}
    a[1] = {{c.new_val_1}}
    a[2] = {{c.new_val_2}}
{%- if c.new_wrong_val_0 %}
    arktest.expectThrow(() => {
      a[0] = {{c.new_wrong_val_0}}
    }, expectedArrayStoreError)
    arktest.expectThrow(() => {
      a[1] = {{c.new_wrong_val_1}}
    }, expectedArrayStoreError)
    arktest.expectThrow(() => {
      a[2] = {{c.new_wrong_val_2}}
    }, expectedArrayStoreError)
{%- endif %}
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
    return true
}

function main(): int {
  let testSuite = new arktest.ArkTestsuite("fixed_array_type_erasure");

  testSuite.addTest("check_at_variable_declaration_instanceof", () => {
    let a: FixedArray<{{c.element_type}}> = [{{c.check_val_0}}, {{c.check_val_1}}, {{c.check_val_2}}]
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.check_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.check_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.check_val_2}})
    a[0] = {{c.new_val_0}}
    a[1] = {{c.new_val_1}}
    a[2] = {{c.new_val_2}}
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
  });

  testSuite.addTest("check_at_variable_declaration_cast_to_object", () => {
    let a: FixedArray<{{c.element_type}}> = [{{c.check_val_0}}, {{c.check_val_1}}, {{c.check_val_2}}]
    let act = a as FixedArray<{{c.element_type}}>
    arktest.expectThrow(() => {
      let awt = a as {{c.fixed_array_cast_wrong_type}}
    }, expectedCastError)
    let ob : Object = a as Object
    let x = ob as FixedArray<{{c.element_type}}>
    arktest.expectThrow(() => {
      let xwt = ob as {{c.fixed_array_cast_wrong_type}}
    }, expectedCastError)
    arktest.assertEQ(x.length, 3)
    arktest.assertTrue(x instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(x instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(x instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(x[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[0], {{c.check_val_0}})
    arktest.assertTrue(x[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[1], {{c.check_val_1}})
    arktest.assertTrue(x[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[2], {{c.check_val_2}})
    x[0] = {{c.new_val_0}}
    x[1] = {{c.new_val_1}}
    x[2] = {{c.new_val_2}}
    arktest.assertEQ(x.length, 3)
    arktest.assertTrue(x instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(x instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(x instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(x[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[0], {{c.new_val_0}})
    arktest.assertTrue(x[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[1], {{c.new_val_1}})
    arktest.assertTrue(x[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[2], {{c.new_val_2}})
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
  });

  testSuite.addTest("check_at_variable_declaration_cast_to_any", () => {
    let a: FixedArray<{{c.element_type}}> = [{{c.check_val_0}}, {{c.check_val_1}}, {{c.check_val_2}}]
    let an : Any = a as Any
    let x = an as FixedArray<{{c.element_type}}>
    arktest.expectThrow(() => {
      let xwt = an as {{c.fixed_array_cast_wrong_type}}
    }, expectedCastError)
    arktest.assertEQ(x.length, 3)
    arktest.assertTrue(x instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(x instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(x instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(x[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[0], {{c.check_val_0}})
    arktest.assertTrue(x[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[1], {{c.check_val_1}})
    arktest.assertTrue(x[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[2], {{c.check_val_2}})
    x[0] = {{c.new_val_0}}
    x[1] = {{c.new_val_1}}
    x[2] = {{c.new_val_2}}
    arktest.assertEQ(x.length, 3)
    arktest.assertTrue(x instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(x instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(x instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(x[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[0], {{c.new_val_0}})
    arktest.assertTrue(x[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[1], {{c.new_val_1}})
    arktest.assertTrue(x[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(x[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(x[2], {{c.new_val_2}})
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
  });

  testSuite.addTest("check_with_object_and_cast_to_origin_type", () => {
    let a: FixedArray<{{c.element_type}}> = [{{c.check_val_0}}, {{c.check_val_1}}, {{c.check_val_2}}]
    arktest.assertTrue(check_with_object_and_cast_to_origin_type(a))
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
  });

  testSuite.addTest("check_with_object_and_cast_to_superclass_type", () => {
    let a: FixedArray<{{c.element_type}}> = [{{c.check_val_0}}, {{c.check_val_1}}, {{c.check_val_2}}]
    arktest.assertTrue(check_with_object_and_cast_to_superclass_type(a))
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
  });

  testSuite.addTest("check_with_any_and_cast_to_origin_type", () => {
    let a: FixedArray<{{c.element_type}}> = [{{c.check_val_0}}, {{c.check_val_1}}, {{c.check_val_2}}]
    arktest.assertTrue(check_with_any_and_cast_to_origin_type(a))
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
  });

  testSuite.addTest("check_with_any_and_cast_to_superclass_type", () => {
    let a: FixedArray<{{c.element_type}}> = [{{c.check_val_0}}, {{c.check_val_1}}, {{c.check_val_2}}]
    arktest.assertTrue(check_with_any_and_cast_to_superclass_type(a))
    arktest.assertEQ(a.length, 3)
    arktest.assertTrue(a instanceof {{c.fixed_array_check_type}})
    arktest.assertTrue(a instanceof {{c.fixed_array_check_superclass_type}})
    arktest.assertFalse(a instanceof {{c.fixed_array_check_wrong_type}})
    arktest.assertTrue(a[0] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[0] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[0], {{c.new_val_0}})
    arktest.assertTrue(a[1] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[1] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[1], {{c.new_val_1}})
    arktest.assertTrue(a[2] instanceof {{c.element_check_type}})
    arktest.assertFalse(a[2] instanceof {{c.element_check_wrong_type}})
    arktest.assertEQ(a[2], {{c.new_val_2}})
  });

  return testSuite.run();
}

{% endfor %}
