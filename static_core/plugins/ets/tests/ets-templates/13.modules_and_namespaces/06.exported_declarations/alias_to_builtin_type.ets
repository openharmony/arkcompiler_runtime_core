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
desc: Exported type aliases may resolve to builtin ArkTS types.
---*/

'use static'

// 类型别名指向内置类型，应编译成功（内置类型总是可用的）
export type MyNumber = double
export type MyString = string
export type MyIntArray = int[]

function main() {
    let numberValue: MyNumber = 1.0
    let stringValue: MyString = 'ArkTS'
    let arrayValue: MyIntArray = [1, 2]
    arktest.assertEQ(numberValue, 1.0)
    arktest.assertEQ(stringValue, 'ArkTS')
    arktest.assertEQ(arrayValue.length, 2)
}
