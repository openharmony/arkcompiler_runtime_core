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
desc: Casting conversions float to double.
---*/

function isFloat(value: number): boolean {
  return value >= -3.4e38 && value <= 3.4e38;
}

function floatToDouble(floatValue: number): number {
  if (!isFloat(floatValue)) {
    throw new Error("no float type");
  }
  return floatValue;
}

function main(): void {

  let floatValue: number = 3.14159;
  let doubleValue: number = floatToDouble(floatValue);

  console.log(`raw value (float): ${floatValue}`);
  console.log(`Converted value (double): ${doubleValue}`);

  if (typeof doubleValue === "number") {
    console.log("sucess！");
  } else {
    console.log("false！");
  }
}