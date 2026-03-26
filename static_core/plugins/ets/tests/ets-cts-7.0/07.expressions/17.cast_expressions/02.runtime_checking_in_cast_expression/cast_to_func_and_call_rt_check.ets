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
    Checking cast to function type and its call at once.
---*/

type TypeA = long | number
type TypeB = number | bigint
enum ValueType {A, B}
type CheckValueFuncTypeA = (a: TypeA) => void
type CheckValueFuncTypeB = (a: TypeB) => void
type CheckValueFunc = CheckValueFuncTypeA | CheckValueFuncTypeB

function check_long_as_type_A(a: TypeA) {
  arktest.assertTrue(a instanceof long)
  arktest.assertEQ(a as long, 9223372036854775807)
}

function check_number_as_type_A(a: TypeA) {
  arktest.assertTrue(a instanceof number)
  arktest.assertEQ(a as number, 5.7)
}

function check_number_as_type_B(a: TypeB) {
  arktest.assertTrue(a instanceof number)
  arktest.assertEQ(a as number, 7.8)
}

function check_bigint_as_type_B(a: TypeB) {
  arktest.assertTrue(a instanceof bigint)
  arktest.assertEQ(a as bigint, 5n)
}

function check_func(v: Any, v_type: ValueType, v_check_func: CheckValueFunc): void {
  if (v_type == ValueType.A) {
    arktest.assertTrue(v instanceof TypeA);
    arktest.assertTrue(v_check_func instanceof CheckValueFuncTypeA);
    (v_check_func as CheckValueFuncTypeA)(v as TypeA);
  }
  else if (v_type == ValueType.B) {
    arktest.assertTrue(v instanceof TypeB);
    arktest.assertTrue(v_check_func instanceof CheckValueFuncTypeB);
    (v_check_func as CheckValueFuncTypeB)(v as TypeB);
  }
}

function main(): void {
  check_func(9223372036854775807, ValueType.A, check_long_as_type_A)
  check_func(5.7, ValueType.A, check_number_as_type_A)
  check_func(7.8, ValueType.B, check_number_as_type_B)
  check_func(5n, ValueType.B, check_bigint_as_type_B)
}
