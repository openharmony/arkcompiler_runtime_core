/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: >-
  A continue statement with label identifier transfers control out of the enclosing loop statement
  which has the same label identifier.Such statement must be in the body of the surrounding
  function or method otherwise compile - time error is raised by the compiler.
---*/

function main(): int {
    let sum: int
    let arr1: int[] = [0, 1, 2]
    let arr2: int[] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
    loop1: for (let y of arr1) {
        for (let x of arr2) {
            sum += x
        }
        if (y % 2 == 0) {
            continue loop1; // the outer loop only with label
        }
    }
    if (sum == 135) return 0;
    return 1;
}
