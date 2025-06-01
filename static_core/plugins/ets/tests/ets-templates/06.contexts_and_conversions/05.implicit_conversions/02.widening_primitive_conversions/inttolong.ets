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
desc: Casting conversions int to long.
---*/

function isInt(value: number): boolean {
  return value >= -2147483648 && value <= 2147483647;
}

function isLong(value: bigint): boolean {
  return value >= BigInt("-9223372036854775808") && value <= BigInt("9223372036854775807");
}

function intToLong(intValue: number): bigint {
  if (!isInt(intValue)) {
    throw new Error("no int type");
  }
  return BigInt(intValue);
}

function main(): void {
  let intValue: number = 2147483647;
  let longValue: bigint = intToLong(intValue);
  console.log(`raw value (int): ${intValue}`);
  console.log(`Converted value (long): ${longValue}`);

  if (isLong(longValue)) {
    console.log("sucess！");
  } else {
    console.log("false！");
  }
}
