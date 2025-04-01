/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: Issue 17344. Missed int to double conversion in spread parameters
tags: []
---*/

function min(...values: number[]): number {
    return values[0]
}

function caller(val0: short, val1: short, val2: short, val3: short): number {
    return min(val0, val1, val2, val3)
}

function main(): void {
    assertEQ(-1, caller(-1 as short, 2 as short, 3 as short, 4 as short))
}
