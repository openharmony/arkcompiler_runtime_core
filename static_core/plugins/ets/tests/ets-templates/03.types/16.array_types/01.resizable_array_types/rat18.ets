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
desc: Test creating a 3D array(number[][][]).
---*/

function main(): void {
  type ThreeDArray = number[][][];

  let arrThree: ThreeDArray = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]];
  let a = arrThree[0][1][0];
  assertEQ(a, 3);
}