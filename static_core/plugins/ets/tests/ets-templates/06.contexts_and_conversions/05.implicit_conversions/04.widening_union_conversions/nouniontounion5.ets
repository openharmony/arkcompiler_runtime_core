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
desc: Casting conversions floatnounion to union.
tags: [compile-only, positive]
---*/

let floatNumber: number = 3.14;
type StringOrNumber = string | number;
let unionValue: StringOrNumber = floatNumber;
let expectedValue: StringOrNumber = 3.14;

function main(): void {
  if (unionValue === expectedValue) {
    console.log("Test passed! Non-union type successfully converted to union type.");
  } else {
    console.error(`Test failed! Expected ${expectedValue}, but got ${unionValue}`);
  }
}