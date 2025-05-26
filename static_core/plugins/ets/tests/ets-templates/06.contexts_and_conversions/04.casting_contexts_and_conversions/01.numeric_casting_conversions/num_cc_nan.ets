/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
desc: A numeric casting conversion occurs if the target type and the expression type are both numeric or char.
params: from NaN to NaN
---*/

function main(): int {
    let s1: float = Float.NaN
    let d1: float = s1 as float
    if (!Float.isNaN(d1)) {
        return 1
    }

    let s2: float = Float.NaN
    let d2: double = s2 as double
    if (!Double.isNaN(d2)) {
        return 1
    }

    let s3: double = Double.NaN
    let d3: double = s3 as double
    if (!Double.isNaN(d3)) {
        return 1
    }

    let s4: double = Double.NaN
    let d4: float = s4 as float
    if (!Float.isNaN(d4)) {
        return 1
    }

    return 0
}
