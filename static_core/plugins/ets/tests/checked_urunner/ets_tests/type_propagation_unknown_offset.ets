/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

//! CHECKER         Check type of read at unknown index
//! RUN             force_jit: true, options: "--compiler-regex='.*__noinline__readAfterWrite.*'",
//!                 entry: "type_propagation_unknown_offset.ETSGLOBAL::readAfterWrite"
//! METHOD          "type_propagation_unknown_offset.ETSGLOBAL::__noinline__readAfterWrite"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "IsInstance"

//! CHECKER         Check type of read at unknown index (AOT)
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*__noinline__readAfterWrite.*'"
//! METHOD          "type_propagation_unknown_offset.ETSGLOBAL::__noinline__readAfterWrite"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "IsInstance"
//! RUN             options: "", entry: "type_propagation_unknown_offset.ETSGLOBAL::readAfterWrite"

function __noinline__readAfterWrite(idx1: int, idx2: int): boolean {
    const arr: FixedArray<Int|Double> = [1, 2];
    arr[idx1] = 1.0;
    // cannot optimize away instanceof (obviously)
    return arr[idx2] instanceof Int;
}

function readAfterWrite() {
    arktest.assertEQ(__noinline__readAfterWrite(0, 1), true);
    arktest.assertEQ(__noinline__readAfterWrite(0, 0), false);
}

//! CHECKER         Check type of read at unknown index in a loop
//! RUN             force_jit: true, options: "--compiler-regex='.*__noinline__loopReadUnknown.*'",
//!                 entry: "type_propagation_unknown_offset.ETSGLOBAL::loopReadUnknown"
//! METHOD          "type_propagation_unknown_offset.ETSGLOBAL::__noinline__loopReadUnknown"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "IsInstance"

//! CHECKER         Check type of read at unknown index in a loop (AOT)
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*__noinline__loopReadUnknown.*'"
//! METHOD          "type_propagation_unknown_offset.ETSGLOBAL::__noinline__loopReadUnknown"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "IsInstance"
//! RUN             options: "", entry: "type_propagation_unknown_offset.ETSGLOBAL::loopReadUnknown"

function __noinline__loopReadUnknown(val: Int|Double): boolean {
    const arr: FixedArray<Int|Double> = [1, 4.0, val];
    let favoriteElem: Int|Double = arr[0];
    for (let i = 0; i < arr.length; i++) {
        if (arr[i] === 4) {
            favoriteElem = arr[i];
        }
    }
    // cannot optimize away instanceof (obviously)
    return favoriteElem instanceof Double;
}

function loopReadUnknown() {
    arktest.assertEQ(__noinline__loopReadUnknown(3), true);
    arktest.assertEQ(__noinline__loopReadUnknown(4), false);
}

//! DISABLED_CHECKER         Check write to aliasing array in function parameter
//! RUN             force_jit: true, options: "--compiler-regex='.*__noinline__aliasingWrite.*'",
//!                 entry: "type_propagation_unknown_offset.ETSGLOBAL::aliasingWrite"
//! METHOD          "type_propagation_unknown_offset.ETSGLOBAL::__noinline__aliasingWrite"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "IsInstance"

//! DISABLED_CHECKER         Check write to aliasing array in function parameter (AOT)
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*__noinline__aliasingWrite.*'"
//! METHOD          "type_propagation_unknown_offset.ETSGLOBAL::__noinline__aliasingWrite"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST            "IsInstance"
//! RUN             options: "", entry: "type_propagation_unknown_offset.ETSGLOBAL::aliasingWrite"

function __noinline__aliasingWrite(arr1: FixedArray<Int|Double>, arr2: FixedArray<Int|Double>): boolean {
    arr1[0] = 1.0;
    // cannot optimize away instanceof (obviously)
    return arr2[0] instanceof Int;
}

function aliasingWrite() {
    arktest.assertEQ(__noinline__aliasingWrite([1, 2], [3, 4]), true);
    arktest.assertEQ(__noinline__aliasingWrite([1, 2], [1.0, 2]), false);
    const arr: FixedArray<Int|Double> = [1, 2];
    arktest.assertEQ(__noinline__aliasingWrite(arr, arr), false);
}

//! CHECKER         Check write to unknown index with possible optimization
//! RUN             force_jit: true, options: "--compiler-regex='.*__noinline__positiveStore.*'",
//!                 entry: "type_propagation_unknown_offset.ETSGLOBAL::positiveStore"
//! METHOD          "type_propagation_unknown_offset.ETSGLOBAL::__noinline__positiveStore"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "IsInstance"

//! CHECKER         Check write to unknown index with possible optimization (AOT)
//! SKIP_IF         @architecture == "arm32"
//! RUN_PAOC        options: "--compiler-regex='.*__noinline__positiveStore.*'"
//! METHOD          "type_propagation_unknown_offset.ETSGLOBAL::__noinline__positiveStore"
//! PASS_BEFORE     "Peepholes"
//! INST            "IsInstance"
//! PASS_AFTER      "Peepholes"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_NOT        "IsInstance"
//! RUN             options: "", entry: "type_propagation_unknown_offset.ETSGLOBAL::positiveStore"

function __noinline__positiveStore(idx: int, val: Int): boolean {
    const arr: FixedArray<Object> = [1, 2, 3];
    arr[idx] = val;
    // optimize it out!
    return arr[0] instanceof Double;
}

function positiveStore() {
    arktest.assertEQ(__noinline__positiveStore(0, 1), false);
    arktest.assertEQ(__noinline__positiveStore(1, 2), false);
}
