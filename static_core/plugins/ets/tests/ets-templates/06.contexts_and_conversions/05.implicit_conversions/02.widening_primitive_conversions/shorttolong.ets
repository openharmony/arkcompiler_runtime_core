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
desc: Casting conversions short to long.
---*/

function isShort(value: number): boolean {
  return value >= -32768 && value <= 32767;
}

function isLong(value: bigint): boolean {
  return value >= BigInt("-9223372036854775808") && value <= BigInt("9223372036854775807");
}

function shortToLong(shortValue: number): bigint {
  if (!isShort(shortValue)) {
    throw new Error("no short type");
  }
  return BigInt(shortValue);
}

function main(): void {
  let shortValue: number = 12345;
  let longValue: bigint = shortToLong(shortValue);
  console.log(`raw value (short): ${shortValue}`);
  console.log(`Converted value (long): ${longValue}`);

  if (isLong(longValue)) {
    console.log("sucess！");
  } else {
    console.log("false！");
  }
}
