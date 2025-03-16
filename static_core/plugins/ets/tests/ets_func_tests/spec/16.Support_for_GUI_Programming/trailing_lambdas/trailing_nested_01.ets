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
desc: >-
    17.10 Trailing lambda
tags: []
---*/

function fa(fn: () => void) {
    fn()
}

function nested(fn: () => void) {
    fn()
}

function main(): void {
    let a = 5
    nested() {
        fa() {
            assertEQ( a, 5)
            a = 4
        }
        assertEQ( a, 4)
        a = 3
    }
    assertEQ( a, 3)
}

