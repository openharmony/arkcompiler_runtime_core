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

function is_active(fn: () => void): void {
    fn()
}

function with_param(fn: () => void): void {
    fn()
}

function main(): void {
    let count = 0
    with_param() {
        try {
            is_active() {
                count++
            }
        }
        catch (ex){
            is_active() { assertTrue( false,  "Not expected execution here") }
        }
        finally {
            is_active() { count++ }
        }
    }
    assertEQ( count, 2,  "Expected 2, actual " + count)
}

