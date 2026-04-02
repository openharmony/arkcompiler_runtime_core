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
    Checking that types A and B (based on unions of string literal types)
    are subtypes of A | B by checking values of type A | B
    (which includes values from types A & B, A - B, B - A)
---*/

type A = "Done" | "In Progress"
type B = "In Progress" | "On Review"

function check_value_is_from_type_A(v: Any): boolean {
  if (v instanceof string) {
    return v === "Done" || v === "In Progress"
  }
  else {
    return false
  }
}

function check_value(v: A | B, v_id: int): void {
  if (v_id == 1) {
    arktest.assertTrue(v instanceof string)
    arktest.assertEQ(v as A, "In Progress")
    arktest.assertEQ(v as B, "In Progress")
  }
  else if (v_id == 2) {
    arktest.assertTrue(v instanceof string)
    arktest.assertEQ(v as A, "Done")
  }
  else {
    arktest.assertTrue(v instanceof string)
    arktest.assertEQ(v as B, "On Review")
  }
}

function check_as_type_A_inter_B(v: B, v_id: int): void {
  arktest.assertEQ(v_id, 1)
  check_value(v, v_id)
  arktest.assertTrue(v instanceof string)
  arktest.assertEQ(v, "In Progress")
  arktest.assertEQ(v as A, "In Progress")
  arktest.assertEQ(v as B, "In Progress")
}

function check_as_type_A_diff_B(v: A, v_id: int): void {
  arktest.assertEQ(v_id, 2)
  check_value(v, v_id)
  arktest.assertTrue(v instanceof string)
  arktest.assertEQ(v, "Done")
}

function check_as_type_B_diff_A(v: B, v_id: int): void {
  arktest.assertEQ(v_id, 3)
  check_value(v, v_id)
  arktest.assertTrue(v instanceof string)
  arktest.assertEQ(v, "On Review")
}

function check_as_type_A(v: A, v_id: int): void {
  check_value(v, v_id);
  (v === "In Progress") ? check_as_type_A_inter_B(v as B, v_id) : check_as_type_A_diff_B(v, v_id);
}

function check(v: A | B, v_id: int): void {
  check_value(v, v_id);
  check_value_is_from_type_A(v) ? check_as_type_A(v as A, v_id) : check_as_type_B_diff_A(v as B, v_id);
}

function main(): void {
  check("In Progress", 1)
  check("Done", 2)
  check("On Review", 3)
}
