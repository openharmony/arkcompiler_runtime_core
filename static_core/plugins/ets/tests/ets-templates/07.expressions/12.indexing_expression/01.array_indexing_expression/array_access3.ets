/*---
Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
    Array access expression. A numeric types conversion (§6.2.1) is performed on index
    expression to make sure the resultant type is int. A compile-time error occurs otherwise.
---*/

let arr: int[] = [1, 2, 3];

function main(): void {
  let x: Byte = new Byte(0 as byte)
  let y: Short = new Short(1 as short)
  let z: Int = new Int(2 as int)

  assert arr[x] == 1 && arr[y] == 2 && arr[z] == 3
}
