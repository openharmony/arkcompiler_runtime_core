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
desc: Test whether the elements of the 3D array can be accessed normally(string[][][]).
---*/

function main(): void {
  type ThreeDArray = string[][][];

  let arrThree: ThreeDArray = [[["aa", "bb"], ["cc", "dd"]], [["ee", "ff"], ["gg", "hh"], ["ii", "jj"]]];
  let a = arrThree[1][1][0];
  assertEQ(a, "gg");
}