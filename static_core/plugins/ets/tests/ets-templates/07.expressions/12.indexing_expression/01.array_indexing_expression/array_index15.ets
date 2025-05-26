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
desc: Test the array index expression to array destructuring.
---*/

function main(): void {
  let colors: Array<string> = new Array<string>("Red", "Green", "Blue");
  let firstColor: string = colors[0];
  let secondColor: string = colors[1];
  let thirdColor: string = colors[2];
  assertEQ(firstColor, "Red");
  assertEQ(secondColor, "Green");
  assertEQ(thirdColor, "Blue");
}