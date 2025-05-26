/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Casting conversions enum to double.
---*/

enum Status {
  Active = 10,
  Inactive = 20,
  Pending = 30
}

let activeDoubleValue: number = Status.Active;
let inactiveDoubleValue: number = Status.Inactive;
let pendingDoubleValue: number = Status.Pending;

let expectedActiveValue: number = 10.0;
let expectedInactiveValue: number = 20.0;
let expectedPendingValue: number = 30.0;

function main(): void {
  if (activeDoubleValue === expectedActiveValue) {
    console.log("Test passed! Active enum value successfully converted to double.");
  } else {
    console.error(`Test failed! Expected ${expectedActiveValue}, but got ${activeDoubleValue}`);
  }

  if (inactiveDoubleValue === expectedInactiveValue) {
    console.log("Test passed! Inactive enum value successfully converted to double.");
  } else {
    console.error(`Test failed! Expected ${expectedInactiveValue}, but got ${inactiveDoubleValue}`);
  }

  if (pendingDoubleValue === expectedPendingValue) {
    console.log("Test passed! Pending enum value successfully converted to double.");
  } else {
    console.error(`Test failed! Expected ${expectedPendingValue}, but got ${pendingDoubleValue}`);
  }
}