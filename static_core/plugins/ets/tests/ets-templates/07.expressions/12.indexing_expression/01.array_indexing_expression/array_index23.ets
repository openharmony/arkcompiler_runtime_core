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
desc: Test the array index using an enumeration type as an index.
tags: [compile-only, positive]
---*/

function main(): void {
  enum Color { Red, Green, Blue }
  let colorValues: Array <number> = new Array <number>(10, 20, 30);
  colorValues[Color.Green];
  assertEQ(colorValues[Color.Green], 20);
}