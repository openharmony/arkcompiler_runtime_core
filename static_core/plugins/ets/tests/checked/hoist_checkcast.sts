/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

//! CHECKER      Hoist loop invariant CheckCast and replace by deopt, JIT
//! RUN          force_jit: true, options: "--compiler-regex='.*test.*'", entry: "ETSGLOBAL::main"
//! EVENT        /Inline,ETSGLOBAL::test,ETSGLOBAL::set,.*,STATIC,SUCCESS/
//! EVENT        /Compilation,ETSGLOBAL::test,.*,COMPILED/
//! EVENT        /DeoptimizationReason,ETSGLOBAL::test,CHECK_CAST/
//! METHOD       "ETSGLOBAL::test"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "CheckCast", 1
//! INST_COUNT   "RefTypeCheck", 1
//! PASS_AFTER   "LICM"
//! PASS_AFTER_NEXT "ChecksElimination"
//! INST         "CheckCast D"
//! INST         "RefTypeCheck D"
//! PASS_BEFORE  "Codegen"
//! INST_COUNT   "CheckCast", 1
//! INST_COUNT   "RefTypeCheck", 1

//! CHECKER      Hoist loop invariant CheckCast and replace by deopt, AOT
//! SKIP_IF      @architecture == "arm32"
//! RUN_AOT      options: "--compiler-regex='.*test.*'"
//! RUN          entry: "ETSGLOBAL::main"

class A {}
class B {}

function set(arr: A[], i: int, value : A | B) {
    arr[i] = value as A;
}

function test(arr: A[], value: A | B) : int {
    let iters = 0;
    try {
        for (let i: int = 0; i < arr.length; ++i) {
            iters++;
            set(arr, i, value);
        }
    } catch (e: ClassCastError) {
        assert e.toString().match('ClassCastError: B cannot be cast to A') != null
        return -iters;
    }
    return iters;
}

function main(): int {
    let arr = new A[5];
    if (test(arr, new A()) != 5) {
        return 1;
    }
    if (test(arr, new B()) != -1) {
        return 2;
    }
    return 0;
}

//! CHECKER      Hoist loop invariant RefTypeCheck and replace by deopt, JIT
//! RUN          force_jit: true, options: "--compiler-regex='.*test2.*'", entry: "ETSGLOBAL::main2"
//! EVENT        /Compilation,ETSGLOBAL::test2,.*,COMPILED/
//! EVENT        /DeoptimizationReason,ETSGLOBAL::test2,CHECK_CAST/
//! METHOD       "ETSGLOBAL::test2"
//! PASS_AFTER   "IrBuilder"
//! INST_NOT     "CheckCast"
//! INST_COUNT   "RefTypeCheck", 1
//! PASS_AFTER   "LICM"
//! PASS_AFTER_NEXT "ChecksElimination"
//! INST         "RefTypeCheck D"
//! PASS_BEFORE  "Codegen"
//! INST_NOT     "CheckCast"
//! INST_COUNT   "RefTypeCheck", 1

//! CHECKER      Hoist loop invariant RefTypeCheck and replace by deopt, AOT
//! SKIP_IF      @architecture == "arm32"
//! RUN_AOT      options: "--compiler-regex='.*test2.*'"
//! RUN          entry: "ETSGLOBAL::main2"

//! CHECKER      Hoist loop invariant RefTypeCheck and replace by deopt, JIT with CPP interpreter
//! RUN          force_jit: true, options: "--compiler-regex='.*test2.*' --interpreter-type=cpp", entry: "ETSGLOBAL::main2"

//! CHECKER      Check stacktrace for starr_obj with wrong type, CPP interpreter
//! RUN          options: "--compiler-enable-jit=false --interpreter-type=cpp", entry: "ETSGLOBAL::main2"

//! CHECKER      Check stacktrace for starr_obj with wrong type, irtoc interpreter
//! RUN          options: "--compiler-enable-jit=false --interpreter-type=irtoc", entry: "ETSGLOBAL::main2"

function test2(arr: Object[], value: A | B) : int {
    let iters = 0;
    try {
        for (let i: int = 0; i < arr.length; ++i) {
            iters++;
            arr[i] = value
        }
    } catch (e: ArrayStoreError) {
        // NOTE: line number is invalid in interpreter, see #17098
        assert e.toString().match('ArrayStoreError: B cannot be stored in an array of type [LA;') != null
        return -iters;
    }
    return iters;
}

function main2(): int {
    let arr = new A[5];
    if (test2(arr, new A()) != 5) {
        return 1;
    }
    if (test2(arr, new B()) != -1) {
        return 2;
    }
    return 0;
}

