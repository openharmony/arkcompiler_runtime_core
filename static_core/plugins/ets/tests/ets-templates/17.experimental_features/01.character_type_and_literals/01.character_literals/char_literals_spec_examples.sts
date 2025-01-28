/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Char literals. Spec examples of char literals.
---*/

function main() {
    let c1 = c'a'
    let c2 = c'\n'
    let c3 = c'\x7F'
    let c4 = c'\u0000'

    let c1b = new Char(c1)
    let c2b = new Char(c2)
    let c3b = new Char(c3)
    let c4b = new Char(c4)

    assertEQ( c1b.unboxed(), c1 )
    assertEQ( c2b.unboxed(), c2 )
    assertEQ( c3b.unboxed(), c3 )
    assertEQ( c4b.unboxed(), c4 )
}
