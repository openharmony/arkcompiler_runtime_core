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
    Checking values of intersection type A & B and difference type A - B
    by checking values of type A in case of usage union of literal types
    with ! and conditional-or expression usage in function call
---*/

type A = "Done" | "In Progress" | undefined
type B = "In Progress" | "On Review" | undefined
type A_diff_B = "Done"
type A_inter_B = "In Progress" | undefined

function check_as_type_A_inter_B(v: A_inter_B, v_id: int): void {
  if (v_id == 2) {
    arktest.assertEQ(v, "In Progress")
  }
  else if (v_id == 3) {
    arktest.assertEQ(v, undefined)
  }
  else {
    arktest.assertTrue(false,
      'v_id should be == 2 or 3')
  }
}

function check_as_type_A_diff_B(v: A_diff_B, v_id: int): void {
  if (v_id == 1) {
    arktest.assertEQ(v, "Done")
  }
  else {
    arktest.assertTrue(false,
      'v_id should be == 1')
  }
}

function check_as_type_A(v: A, v_id: int): void {
  // type A_inter_B = "In Progress" | undefined
  if (!(v == undefined || v === "In Progress")) {
    check_as_type_A_diff_B(v as A_diff_B, v_id)
  }
  else {
    check_as_type_A_inter_B(v as A_inter_B, v_id)
  }
}

function main(): void {
  check_as_type_A("Done", 1)
  check_as_type_A("In Progress", 2)
  check_as_type_A(undefined, 3)
}
