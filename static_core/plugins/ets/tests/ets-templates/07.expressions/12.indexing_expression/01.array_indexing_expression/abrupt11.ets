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
    For an array, if the index expression value is less than zero, greater
    than or equal to the array's length, then ArrayIndexOutOfBoundsError is thrown.
---*/

let arr: number[] = [1, 2, 3]
let index: int = 3

function main(): int {
    try {
        assertTrue( arr[index] == 0 )
    } catch (e: ArrayIndexOutOfBoundsError) {
        return 0
    }
    return 1
}
