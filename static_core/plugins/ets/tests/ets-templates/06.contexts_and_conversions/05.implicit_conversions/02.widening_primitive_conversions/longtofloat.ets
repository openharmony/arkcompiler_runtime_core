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
desc: Casting conversions long to float.
---*/

function isLong(value: bigint): boolean {
  return value >= BigInt("-9223372036854775808") && value <= BigInt("9223372036854775807");
}

function longToFloat(longValue: bigint): number {
  if (!isLong(longValue)) {
    throw new Error("no long type");
  }
  return Number(longValue);
}

function main(): void {
  let longValue: bigint = BigInt("9223372036854775807");
  let floatValue: number = longToFloat(longValue);
  console.log(`raw value (long): ${longValue}`);
  console.log(`Converted value (float): ${floatValue}`);

  if (typeof floatValue === "number") {
    console.log("sucess！");
  } else {
    console.log("false！");
  }
}
