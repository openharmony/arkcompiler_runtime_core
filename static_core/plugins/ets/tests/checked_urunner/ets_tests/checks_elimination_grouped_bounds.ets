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

//! CHECK_GROUP  PreserveGroupedBounds
//! METHOD       "checks_elimination_grouped_bounds.ETSGLOBAL::__noinline__sumAdjacent"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "BoundsCheck", 2
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "BoundsCheck"
//! INST         /(?<start>\d+)\.i32 +Parameter +arg 1/
//! INST         /(?<zero>\d+)\.i64 +Constant +0x0/
//! INST         /(?<minus_one>\d+)\.i64 +Constant +0xffffffffffffffff/
//! INST         /(?<lower_index>\d+)\.i32 +Add +v#{VAR(:start)}, v#{VAR(:minus_one)}/
//! INST         /Compare LT i32 +v#{VAR(:lower_index)}, v#{VAR(:zero)}/
//! INST         "DeoptimizeIf BOUNDS_CHECK"

//! CHECKER      Preserve upper and lower limits for grouped bounds checks (JIT)
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*__noinline__sumAdjacent.*' --compiler-inlining=false",
//!              entry: "checks_elimination_grouped_bounds.ETSGLOBAL::main", result: 15
//! CHECK_GROUP  PreserveGroupedBounds

//! CHECKER      Preserve upper and lower limits for grouped bounds checks (AOT)
//! RUN_PAOC     options: "--compiler-regex='.*__noinline__sumAdjacent.*' --compiler-inlining=false"
//! CHECK_GROUP  PreserveGroupedBounds
//! RUN          entry: "checks_elimination_grouped_bounds.ETSGLOBAL::main", result: 15

function __noinline__sumAdjacent(arr: ValueArray<int>, start: int): int {
    let sum: int = 0
    for (let i: int = start; i < arr.length; ++i) {
        sum += arr[i]
        sum += arr[i - 1]
    }
    return sum
}

function main(): int {
    let arr: ValueArray<int> = [1, 2, 3, 4]
    return __noinline__sumAdjacent(arr, 1)
}
