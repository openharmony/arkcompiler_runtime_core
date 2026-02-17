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

function checkArray<T>(src: Array<T>, exp: Array<T>) {
    arktest.assertEQ(src.length, exp.length, "reverse(): bad array len")
    for (let i = 0; i < exp.length; i++) {
        arktest.assertEQ(src[i], exp[i], "reverse(): unexpected element: i: " + i)
    }
}

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND            options: "--ets-strings-concat=false"

//! CHECKER       INT: ArrayInt reverse
//! RUN           options: "--compiler-enable-jit=false",
//!               entry: "escompat_array_reverse.ETSGLOBAL::testArrayIntReverse"

//! CHECKER       JIT: ArrayInt reverse
//! RUN           force_jit: true,
//!               options: "--compiler-regex=.*testArrayIntReverse.*",
//!               entry: "escompat_array_reverse.ETSGLOBAL::testArrayIntReverse"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::testArrayIntReverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! INST          /CallStatic.Inlined.*std.core.Array::reverse/
//! INST          /Intrinsic.EscompatArrayReverse/
//! ASM_METHOD    "escompat_array_reverse.ETSGLOBAL::testArrayIntReverse"
//! ASM_INST      "Intrinsic.EscompatArrayReverse"

//! CHECKER       AOT: ArrayInt reverse
//! RUN_PAOC      options: "--compiler-regex=.*testArrayIntReverse.* --compiler-inline-external-methods-aot=false",
//!               entry: "escompat_array_reverse.ETSGLOBAL::testArrayIntReverse"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::testArrayIntReverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! EVENT         /Inline,esc.*reverse.ETSGLOBAL::testArrayIntReverse,std.core.Array::reverse,.*,STATIC,SKIP_EXTERNAL/
//! INST_NOT      /CallStatic.Inlined.*std.core.Array::reverse/
//! INST_NOT      /Intrinsic.EscompatArrayReverse/

//! CHECKER       AOT PGO: ArrayInt reverse
//! RUN_PGO_PROF  entry: "escompat_array_reverse.ETSGLOBAL::testArrayIntReverse"
//! RUN_PGO_PAOC  options: "--compiler-regex=.*testArrayIntReverse.* --compiler-inline-external-methods-aot=true"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::testArrayIntReverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! EVENT         /Inline,esc.*reverse.ETSGLOBAL::testArrayIntReverse,std.core.Array::reverse,.*,STATIC,SUCCESS/
//! INST          /CallStatic.Inlined.*std.core.Array::reverse/
//! INST          /Intrinsic.EscompatArrayReverse/
//! ASM_METHOD    "escompat_array_reverse.ETSGLOBAL::testArrayIntReverse"
//! ASM_INST      "Intrinsic.EscompatArrayReverse"

function testArrayIntReverse() {
    let inp: Array<int> = [1, 2, 3, 4, 5, 6, 7];
    let exp: Array<int> = [7, 6, 5, 4, 3, 2, 1];
    checkArray<int>(inp.reverse(), exp);
}

//! CHECKER       INT: ArrayUnion reverse
//! RUN           options: "--compiler-enable-jit=false",
//!               entry: "escompat_array_reverse.ETSGLOBAL::testArrayUnionReverse"

//! CHECKER       JIT: ArrayUnion reverse
//! RUN           force_jit: true,
//!               options: "--compiler-regex=.*testArrayUnionReverse.*",
//!               entry: "escompat_array_reverse.ETSGLOBAL::testArrayUnionReverse"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::testArrayUnionReverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! INST          /CallStatic.Inlined.*std.core.Array::reverse/
//! INST          /Intrinsic.EscompatArrayReverse/
//! ASM_METHOD    "escompat_array_reverse.ETSGLOBAL::testArrayUnionReverse"
//! ASM_INST      "Intrinsic.EscompatArrayReverse"

//! CHECKER       AOT: ArrayUnion reverse
//! RUN_PAOC      options: "--compiler-regex=.*testArrayUnionReverse.* --compiler-inline-external-methods-aot=false",
//!               entry: "escompat_array_reverse.ETSGLOBAL::testArrayUnionReverse"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::testArrayUnionReverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! EVENT         /Inline,esc.*reverse.ETSGLOBAL::testArray.*Reverse,std.core.Array::reverse,.*,STATIC,SKIP_EXTERNAL/
//! INST_NOT      /CallStatic.Inlined.*std.core.Array::reverse/
//! INST_NOT      /Intrinsic.EscompatArrayReverse/

//! CHECKER       AOT PGO: ArrayUnion reverse
//! RUN_PGO_PROF  entry: "escompat_array_reverse.ETSGLOBAL::testArrayUnionReverse"
//! RUN_PGO_PAOC  options: "--compiler-regex=.*testArrayUnionReverse.* --compiler-inline-external-methods-aot=true"
//! METHOD        "escompat_array_reverse.ETSGLOBAL::testArrayUnionReverse"
//! PASS_AFTER    "IrBuilder"
//! INST_NOT      /Intrinsic.EscompatArrayReverse/
//! INST          /CallStatic .*std.core.Array::reverse/
//! PASS_AFTER    "Inline"
//! EVENT         /Inline,esc.*reverse.ETSGLOBAL::testArrayUnionReverse,std.core.Array::reverse,.*,STATIC,SUCCESS/
//! INST          /CallStatic.Inlined.*std.core.Array::reverse/
//! INST          /Intrinsic.EscompatArrayReverse/
//! ASM_METHOD    "escompat_array_reverse.ETSGLOBAL::testArrayUnionReverse"
//! ASM_INST      "Intrinsic.EscompatArrayReverse"

function testArrayUnionReverse() {
    let inp: Array<char | number | string | boolean> = [1, c'b', "abc", c'4', 5, "6", false];
    let exp: Array<char | number | string | boolean> = [false, "6", 5, c'4', "abc", c'b', 1];
    checkArray<char | number | string | boolean>(inp.reverse(), exp);
}
