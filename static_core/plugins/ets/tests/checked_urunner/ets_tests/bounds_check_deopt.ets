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

//! CHECKER      Do not deoptimize for a branch-local upper bounds check
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*branchLocalUpperNoDeopt.*' --compiler-inlining=false",
//!              entry: "bounds_check_deopt.ETSGLOBAL::testBranchLocalUpperNoDeopt", result: 39
//! METHOD       "bounds_check_deopt.ETSGLOBAL::__noinline__branchLocalUpperNoDeopt"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "BoundsCheck", 1
//! PASS_AFTER   "ChecksElimination"
//! INST_NOT     "DeoptimizeIf"
//! EVENT        /Compilation,bounds_check_deopt.ETSGLOBAL::__noinline__branchLocalUpperNoDeopt,.*,COMPILED/
//! EVENT_NOT    /Deoptimization.*/
function __noinline__branchLocalUpperNoDeopt(arr: FixedArray<int>): int {
    let sum: int = 0;
    for (let i: int = 0; i < arr.length; ++i) {
        if (i < arr.length - 4) {
            sum += arr[i + 4];
        }
    }
    return sum;
}

function testBranchLocalUpperNoDeopt(): int {
    let arr: FixedArray<int> = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
    return __noinline__branchLocalUpperNoDeopt(arr);
}

//! CHECKER      Merge branch-local bounds checks in one block
//! RUN_PAOC     options: "--compiler-regex='.*branchLocalBlockMerge.*' --compiler-inlining=false"
//! METHOD       "bounds_check_deopt.ETSGLOBAL::__noinline__branchLocalBlockMerge"
//! PASS_BEFORE  "ChecksElimination"
//! INST_COUNT   "BoundsCheck", 2
//! PASS_AFTER   "ChecksElimination"
//! INST         "DeoptimizeIf"
//! INST_NOT     "BoundsCheck"
function __noinline__branchLocalBlockMerge(arr: FixedArray<int>, size: int, take: boolean): int {
    let sum: int = 0;
    for (let i: int = 0; i < size; ++i) {
        if (take) {
            sum += arr[i];
            sum += arr[i + 1];
        }
    }
    return sum;
}

//! CHECKER      Do not deoptimize for merged branch-local bounds checks in bounds
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*branchLocalBlockMerge.*' --compiler-inlining=false",
//!              entry: "bounds_check_deopt.ETSGLOBAL::testBranchLocalBlockMergeInBounds", result: 9
//! EVENT        /Compilation,bounds_check_deopt.ETSGLOBAL::__noinline__branchLocalBlockMerge,.*,COMPILED/
//! EVENT_NOT    /Deoptimization.*/
function testBranchLocalBlockMergeInBounds(): int {
    let arr: FixedArray<int> = [0, 1, 2, 3];
    return __noinline__branchLocalBlockMerge(arr, 3, true);
}

//! CHECKER      Deoptimize for merged branch-local bounds checks out of bounds
//! RUN          force_jit: true,
//!              options: "--compiler-regex='.*branchLocalBlockMerge.*' --compiler-inlining=false",
//!              entry: "bounds_check_deopt.ETSGLOBAL::testBranchLocalBlockMergeOutOfBounds", result: 0
//! EVENT        /Compilation,bounds_check_deopt.ETSGLOBAL::__noinline__branchLocalBlockMerge,.*,COMPILED/
//! EVENT        /DeoptimizationReason,bounds_check_deopt.ETSGLOBAL::__noinline__branchLocalBlockMerge,BOUNDS_CHECK/
function testBranchLocalBlockMergeOutOfBounds(): int {
    let arr: FixedArray<int> = [0, 1, 2, 3];
    try {
        __noinline__branchLocalBlockMerge(arr, 4, true);
    } catch (e) {
        if (e instanceof ArrayIndexOutOfBoundsError) {
            return 0;
        }
    }
    return 1;
}
