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
    Array access expression. This resultant variable is a component of the array but
    is never considered const even if a const variable is denoted in the array reference
    expression.
---*/

type NullableInt = Int|undefined
const arr: NullableInt[] = new NullableInt[3];

function main(): void {
  arr[0] = new Int(1)
  arr[1] = 2
  arr[2] = undefined

  assertTrue( arr[0] instanceof Int )
  assertTrue( arr[0] instanceof Object )
  assertTrue( arr[0] == 1 )

  assertTrue( arr[1] instanceof Int )
  assertTrue( arr[1] instanceof Object )
  assertTrue( arr[1] == 2 )

  assertTrue( arr[2] == undefined )
}
