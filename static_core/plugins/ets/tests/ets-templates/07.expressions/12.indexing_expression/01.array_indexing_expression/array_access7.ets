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
    For an array indexing, a type of index expression must be of a numeric type.
    if type of index expression is number or other floating-point type and its fractional
    part is different from 0 then it will be runtime error if such situation is identified
    during program execution and compile-time one if detected during compilation.
---*/

let arr: int[] = [1, 2, 3];

function main(): void {
  let x: Float = new Float(1.0)
  let y: Double = new Double(2.0)

  assertTrue( arr[x] == 2 && arr[y] == 3 )
}
