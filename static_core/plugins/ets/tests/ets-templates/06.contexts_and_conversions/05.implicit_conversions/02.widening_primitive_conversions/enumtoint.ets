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
desc: Casting conversions enum to int.
---*/

enum Color {
  Red,
  Green,
  Blue
}

let redIntValue: number = Color.Red;
let greenIntValue: number = Color.Green;
let blueIntValue: number = Color.Blue;

let expectedRedValue: number = 0;
let expectedGreenValue: number = 1;
let expectedBlueValue: number = 2;

function main(): void {
  if (redIntValue === expectedRedValue) {
    console.log("Test passed! Red enum value successfully converted to int.");
  } else {
    console.error(`Test failed! Expected ${expectedRedValue}, but got ${redIntValue}`);
  }

  if (greenIntValue === expectedGreenValue) {
    console.log("Test passed! Green enum value successfully converted to int.");
  } else {
    console.error(`Test failed! Expected ${expectedGreenValue}, but got ${greenIntValue}`);
  }

  if (blueIntValue === expectedBlueValue) {
    console.log("Test passed! Blue enum value successfully converted to int.");
  } else {
    console.error(`Test failed! Expected ${expectedBlueValue}, but got ${blueIntValue}`);
  }
}