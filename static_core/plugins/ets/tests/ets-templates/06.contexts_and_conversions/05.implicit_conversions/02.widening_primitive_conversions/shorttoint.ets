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
desc: Casting conversions short to int.
---*/

function isShort(value: number): boolean {
  return value >= -32768 && value <= 32767;
}

function isInt(value: number): boolean {
  return value >= -2147483648 && value <= 2147483647;
}

function shortToInt(shortValue: number): number {
  if (!isShort(shortValue)) {
    throw new Error("no short type");
  }
  return shortValue;
}

function main(): void {
  let shortValue: number = 12345;
  let intValue: number = shortToInt(shortValue);
  console.log(`raw value (short): ${shortValue}`);
  console.log(`Converted value (int): ${intValue}`);

  if (isInt(intValue)) {
    console.log("sucess！");
  } else {
    console.log("false！");
  }
}