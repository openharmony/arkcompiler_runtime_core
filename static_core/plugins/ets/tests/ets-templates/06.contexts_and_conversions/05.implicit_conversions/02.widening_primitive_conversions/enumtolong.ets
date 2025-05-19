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
desc: Casting conversions enum to long.
---*/

enum Color {
  Red,
  Green,
  Blue
}

let redLongValue: bigint = BigInt(Color.Red);
let greenLongValue: bigint = BigInt(Color.Green);
let blueLongValue: bigint = BigInt(Color.Blue);

let expectedRedValue: bigint = 0n;
let expectedGreenValue: bigint = 1n;
let expectedBlueValue: bigint = 2n;

function main(): void {
  if (redLongValue === expectedRedValue) {
    console.log("Test passed! Red enum value successfully converted to long.");
  } else {
    console.error(`Test failed! Expected ${expectedRedValue}, but got ${redLongValue}`);
  }

  if (greenLongValue === expectedGreenValue) {
    console.log("Test passed! Green enum value successfully converted to long.");
  } else {
    console.error(`Test failed! Expected ${expectedGreenValue}, but got ${greenLongValue}`);
  }

  if (blueLongValue === expectedBlueValue) {
    console.log("Test passed! Blue enum value successfully converted to long.");
  } else {
    console.error(`Test failed! Expected ${expectedBlueValue}, but got ${blueLongValue}`);
  }
}
