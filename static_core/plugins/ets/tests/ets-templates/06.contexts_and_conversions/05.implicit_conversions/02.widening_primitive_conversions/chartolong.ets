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
desc: Casting conversions char to long.
---*/

let ch: string = 'A';
let longValue: bigint = BigInt(ch.charCodeAt(0));
let expectedLongValue: bigint = BigInt(65);

function main(): void {
  if (longValue === expectedLongValue) {
    console.log("Test passed! Char successfully converted to long.");
  } else {
    console.error(`Test failed! Expected ${expectedLongValue}, but got ${longValue}`);
  }
}
