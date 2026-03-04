/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
  desc: The number of elements is known as array length, and can be accessed by using the length property
---*/

function main(): int {
    let myTestsuite = new arktest.ArkTestsuite("Access to readonly array property 'length'");
    
    myTestsuite.addTest("Common case", () => {
      let arr: readonly short[] = [1, 2, 3];
      arktest.assertEQ(arr.length, 3);
    });

    myTestsuite.addTest("Empty array", () => {
      let arr: ReadonlyArray<boolean> = [];
      arktest.assertEQ(arr.length, 0);
    });
    
    myTestsuite.addTest("Array of array", () => {
      let arr: ReadonlyArray<string[]> = [["1", "2", "3"], ["1", "2", "3", "4"]];
      arktest.assertEQ(arr.length, 2);
    });

    return myTestsuite.run();
}
