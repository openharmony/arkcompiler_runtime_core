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

//! CHECKER JIT IR Builder: Case #1: escompat.Array get/set operations.
//! RUN     force_jit: true, options: "--compiler-regex='.*(test1|array[GS]et).*'", entry: "escompat_array_get_set.ETSGLOBAL::test1"
//! METHOD  "escompat_array_get_set.ETSGLOBAL::arrayGet"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST       /CallVirtual.*escompat\.Array::\$_get/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! METHOD  "escompat_array_get_set.ETSGLOBAL::arraySet"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST       /CallVirtual.*escompat\.Array::\$_set/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArraySet/

//! CHECKER AOT IR Builder: Case #1: IRBuilder pass must NOT inline Array get/set since neither class Array nor get/set method is final.
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   options: "--compiler-regex='.*array[GS]et.*'"
//! METHOD  "escompat_array_get_set.ETSGLOBAL::arrayGet"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST       /CallVirtual.*escompat\.Array::\$_get/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST       /Call((Virtual)|(Static)) .*escompat\.Array::\$_get/
//! METHOD  "escompat_array_get_set.ETSGLOBAL::arraySet"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST       /CallVirtual.*escompat\.Array::\$_set/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST       /Call((Virtual)|(Static)) .*escompat\.Array::\$_set/

//! CHECKER JIT IR Builder: Case #2: escompat.Array get/set operations (with exceptions).
//! RUN        force_jit: true, entry: "escompat_array_get_set.ETSGLOBAL::test2"
//! EVENT      /Compilation,escompat_array_get_set.ETSGLOBAL::__noinline__testRangeError,.*,COMPILED/
//! METHOD  "escompat_array_get_set.ETSGLOBAL::__noinline__testRangeError"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST       /CallVirtual.*escompat\.Array::\$_set/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! EVENT      /Compilation,escompat_array_get_set.ETSGLOBAL::__noinline__testClassCastError,.*,COMPILED/
//! METHOD  "escompat_array_get_set.ETSGLOBAL::__noinline__testClassCastError"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST       /CallVirtual.*escompat\.Array::\$_get/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/

//! CHECKER AOT IR Builder: Case #2: escompat.Array get/set operations (with exceptions).
//! SKIP_IF    @architecture == "arm32"
//! RUN_PAOC   options: ""
//! METHOD  "escompat_array_get_set.ETSGLOBAL::__noinline__testRangeError"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! INST       /CallVirtual.*escompat\.Array::\$_set/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArraySet/
//! METHOD  "escompat_array_get_set.ETSGLOBAL::__noinline__testClassCastError"
//! PASS_AFTER "IrBuilder"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! INST       /CallVirtual.*escompat\.Array::\$_get/
//! PASS_AFTER "Inline"
//! INST_NOT   /Intrinsic.EscompatArrayGet/
//! RUN        entry: "escompat_array_get_set.ETSGLOBAL::test2"

function arraySet(arr: Array<int>): void {
    arr[1] = 2;
    arr[3] = 5;
    arr[8] = 13;
}

function arrayGet(arr: Array<int>): int {
    return arr[1] + arr[3] + arr[8];
}

const CORRECT_EXCEPTION_THROWN = 0;
const WRONG_EXCEPTION_THROWN = 1;
const NO_EXCEPTION_THROWN = 2;
const WRONG_EXPRESSION_RESULT = 3;

function __noinline__testRangeError(arr: Array<int>, index: int): int {
    try {
        arr[index] = 21;
    } catch (e) {
        if (e instanceof RangeError) {
            return CORRECT_EXCEPTION_THROWN;
        }
        return WRONG_EXCEPTION_THROWN;
    }
    return NO_EXCEPTION_THROWN;
}

function __noinline__testClassCastError(arr: Array<int>, index: int): int {
    const MAGIC_NUMBER = 1234567890;
    let dest = MAGIC_NUMBER;
    try {
        dest = arr[index];
    } catch (e) {
        if (e instanceof ClassCastError) {
            return (dest == MAGIC_NUMBER) ? CORRECT_EXCEPTION_THROWN : WRONG_EXPRESSION_RESULT;
        }
        return WRONG_EXCEPTION_THROWN;
    }
    return NO_EXCEPTION_THROWN;
}

function test1(): void {
    const arr = new Array<int>(16);
    arraySet(arr);
    assertEQ(arrayGet(arr), 20);
}

function test2(): void {
    const arr = new Array<int>(16);
    arraySet(arr);

    // Exception case #1: index < 0
    assertEQ(__noinline__testRangeError(arr, -1), CORRECT_EXCEPTION_THROWN);

    // Exception case #2: index >= length
    assertEQ(__noinline__testRangeError(arr, 16), CORRECT_EXCEPTION_THROWN);

    // Exception case #3: undefined -> Int cast error
    assertEQ(__noinline__testClassCastError(arr, 2), CORRECT_EXCEPTION_THROWN);
}
