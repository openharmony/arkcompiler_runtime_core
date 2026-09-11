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
desc: Exported variables have explicit declared types.
---*/

'use static'

// 导出变量时指定了显式类型，应编译成功
export let a: int = 42
export let b: string = 'hello'
export let c: boolean = true

function main() {
    arktest.assertEQ(a, 42)
    arktest.assertEQ(b, 'hello')
    arktest.assertTrue(c)
}
