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

//! DEFINE_FRONTEND_OPTIONS            DEFAULT ARGS
//! RUN_FRONTEND                       options: ""

//! CHECKER       AOT IR Builder, check Hybrid strings disabled
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*hybridStringTest[0-9]*' --compiler-hybrid-strings-optimization=false"
//! RUN           entry: "ets_hybrid_strings.ETSGLOBAL::main"

//! CHECKER       AOT IR Builder, check Hybrid strings enabled
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*hybridStringTest[0-9]*'"
//! RUN           entry: "ets_hybrid_strings.ETSGLOBAL::main"
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest00"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",5
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",5
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest01"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",3
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest02"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",3
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest03"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",8
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",16
//! INST_COUNT    "Intrinsic.StdCoreSbToString",8
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest04"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",6
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",6
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest05"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",5
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! INST_COUNT    "Intrinsic.StdCoreSbToString",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat3",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest06"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",4
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",4
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest07"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",8
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest08"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",7
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",5
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest09"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",9
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",9
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest10"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",5
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",10
//! INST_COUNT    "Intrinsic.StdCoreSbToString",5
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest11"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",8
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",16
//! INST_COUNT    "Intrinsic.StdCoreSbToString",8
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat4",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest12"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",8
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",16
//! INST_COUNT    "Intrinsic.StdCoreSbToString",8
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat4",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest13"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",7
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",7
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest14"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",8
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest15"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",6
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",6
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest16"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",8
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",16
//! INST_COUNT    "Intrinsic.StdCoreSbToString",8
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat4",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat3",2
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest17"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",14
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",14
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",28
//! INST_COUNT    "Intrinsic.StdCoreSbToString",14
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",5
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest18"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",4
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! INST_COUNT    "Intrinsic.StdCoreSbToString",4
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat3",2
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest19"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",2
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat3",1

//! CHECKER       JIT IR Builder, check Hybrid strings enabled
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*hybridStringTest[0-9]*'", entry: "ets_hybrid_strings.ETSGLOBAL::main"
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest00"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",5
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",5
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest01"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",3
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest02"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",3
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",6
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest03"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",8
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",16
//! INST_COUNT    "Intrinsic.StdCoreSbToString",8
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest04"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",6
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",6
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest05"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",5
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! INST_COUNT    "Intrinsic.StdCoreSbToString",4
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat3",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest06"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",4
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",4
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest07"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",8
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest08"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",7
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",5
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest09"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",9
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",9
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest10"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",5
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",10
//! INST_COUNT    "Intrinsic.StdCoreSbToString",5
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest11"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",8
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",16
//! INST_COUNT    "Intrinsic.StdCoreSbToString",8
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat4",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest12"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",8
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",16
//! INST_COUNT    "Intrinsic.StdCoreSbToString",8
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat4",1
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest13"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",7
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",7
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest14"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",8
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest15"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",6
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_NOT      "StringBuilder::concatStrings"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",6
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest16"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",8
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",8
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",16
//! INST_COUNT    "Intrinsic.StdCoreSbToString",8
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat4",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat3",2
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest17"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",14
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",14
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",28
//! INST_COUNT    "Intrinsic.StdCoreSbToString",14
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",5
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest18"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",4
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",8
//! INST_COUNT    "Intrinsic.StdCoreSbToString",4
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat3",2
//!
//! METHOD        "ets_hybrid_strings.ETSGLOBAL::hybridStringTest19"
//! PASS_AFTER    "Inline"
//! INST_COUNT    "StringBuilder::concatStrings",2
//! PASS_AFTER    "HybridStringsOptimization"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//! INST_NOT      "StringBuilder::concatStrings"
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "Intrinsic.StdCoreStringConcat3",1

function __noinline__consume(arg: string): void {}

function hybridStringTest00(arg: string): string {
    let var1: string = 'var1'
    let var2: string = 'var2'
    const const1: string = 'const1'
    const const2: string = 'const2'

    let tmp0 = var2 + const1
    let tmp1 = const2 + var1
    let tmp2 = tmp1 + tmp0

    return tmp0 + tmp2 + arg // keep tree strings due to wide dataflow
}

function hybridStringTest01(arg: string): string {
    let var1: string = 'var1'

    var1 += arg

    for (let i = 0; i < 10; i++) {
        var1 += arg
    }

    var1 += arg

    return var1 // switch to SB due to long chain concatenation
}

function hybridStringTest02(arg: string): string {
    let var1: string = 'var1'

    var1 += arg

    while (var1.length < 10) {
        var1 += arg
    }

    var1 += arg

    return var1 // switch to SB due to long chain concatenation
}

function hybridStringTest03(arg: string): string {
    let var1: string = 'var1'

    var1 += arg
    var1 += arg
    var1 += arg
    var1 += arg
    var1 += arg
    var1 += arg
    var1 += arg
    var1 += arg

    return var1 // switch to SB due to long chain concatenation
}

function hybridStringTest04(arg: string): string {
    let var1: string = 'var1'

    var1 += arg

    if (true) {
        var1 += arg
    }

    try {
        var1 += arg
    }
    catch (e) {
        var1 += arg
    }
    var1 += arg
    __noinline__consume(var1)
    var1 += arg

    return var1 // keep tree strings due to intermadiate string usage and try/catch
}

function hybridStringTest05(arg: string): int {
    let var1: string = 'var1'
    let var2: string = 'var2'
    const const1: string = 'const1'
    const const2: string = 'const2'

    let tmp0 = var2 + const1
    let tmp1 = const2 + var1
    let tmp2 = tmp1 + arg
    let tmp3 = tmp0 + tmp2

    return tmp3.charCodeAt(0) // switch to SB due to flattening
}

function hybridStringTest06(arg: string): string {
    let var1: string = 'var1'
    let var2: string = 'var2'
    const const1: string = 'const1'
    const const2: string = 'const2'

    let tmp0 = var2 + const1
    __noinline__consume(tmp0)
    let tmp1 = const2 + var1
    __noinline__consume(tmp1)
    let tmp2 = tmp1 + arg
    __noinline__consume(tmp2)
    let tmp3 = tmp0 + tmp2
    __noinline__consume(tmp3)

    return tmp3 // keep tree strings due to wide dataflow and intermadiate string usage
}

function hybridStringTest07(arg: string): string {
    let var1: string = 'var1'
    let var2: string = 'var2'
    const const1: string = 'const1'
    const const2: string = 'const2'

    let tmp0 = var2 + const1
    __noinline__consume(tmp0 + arg)

    let tmp1 = const2 + var1
    try {
        __noinline__consume(tmp1 + arg)
    }
    catch (e) {}

    let tmp2 = tmp1 + arg
    let tmp3 = tmp0 + tmp2
    try {
        __noinline__consume(tmp2 + arg)
    }
    catch (e) {
        __noinline__consume(tmp3 + arg)
    }

    return tmp3 // keep tree strings due to wide dataflow, intermadiate string usage and try/catch
}

function hybridStringTest08(arg: string): string {
    let var1: string = 'var1'
    let var2: string = 'var2'
    const const1: string = 'const1'
    const const2: string = 'const2'

    let tmp0 = var2 + const1
    __noinline__consume(tmp0 + arg)

    let tmp1 = const2 + var1
    if (true) {
        __noinline__consume(tmp1 + arg)
    }

    let tmp2 = tmp1 + arg
    let tmp3 = tmp0 + tmp2
    if (true) {
        __noinline__consume(tmp2 + arg)
    }

    return tmp3 // keep tree strings due intermadiate string usage
}

function hybridStringTest09(arg: string): string {
    let var1: string = 'var1'
    let var2: string = 'var2'
    const const1: string = 'const1'
    const const2: string = 'const2'

    let result: string = ''
    let tmp0 = var2 + const1
    result += tmp0 + arg;

    let tmp1 = const2 + var1
    if (true) {
        result += tmp1 + arg
    }

    let tmp2 = tmp1 + arg
    if (true) {
        result += tmp2 + arg
    }

    return result // keep tree strings due to wide dataflow and intermadiate string usage
}

function hybridStringTest10(arg: string): string {
    let var1: string = 'var1'
    let var2: string = 'var2'
    const const1: string = 'const1'
    const const2: string = 'const2'

    let tmp0 = var2 + const1
    let tmp1 = tmp0 + var1
    let tmp2 = tmp1 + arg
    let tmp3 = tmp2 + const2

    tmp3 += tmp0

    return tmp3 // switch to SB due to long chain concatenation
}

function hybridStringTest11(arg: string): string {
    let var1: string = 'var1'

    var1 += arg
    var1 += arg
    var1 += arg
    var1 += arg
    var1 += arg
    var1 += var1
    var1 += arg
    var1 += arg

    return var1 // switch to SB due to long chain concatenation
}

function hybridStringTest12(arg: string): string {
    let var1: string = 'var1'

    var1 += arg
    var1 += arg
    var1 += arg
    let tmp = var1
    var1 += arg
    var1 += arg
    var1 += tmp
    var1 += arg
    var1 += arg

    return var1 // switch to SB due to long chain concatenation
}

function hybridStringTest13(arg: string): string {
    let var1: string = 'var1'
    let var2: string = 'var2'
    const const1: string = 'const1'
    const const2: string = 'const2'

    let tmp0 = var2 + const1
    let tmp1 = tmp0 + var1
    let tmp2 = tmp1 + arg
    let tmp3 = tmp2 + const2

    tmp3 += tmp0
    tmp3 += tmp1
    tmp3 += tmp2

    return tmp3 // keep tree strings due to wide dataflow and intermadiate string usage
}

function hybridStringTest14(arg: string): string {
    let var1: string = 'var1'

    var1 += arg
    let tmp0 = var1
    var1 += arg
    let tmp1 = var1
    var1 += arg
    let tmp2 = var1
    var1 += arg
    let tmp3 = var1
    var1 += tmp3
    var1 += tmp2
    var1 += tmp1
    var1 += tmp0

    return var1 // keep tree strings due to intermadiate string usage
}

function hybridStringTest15(arg: string): string {
    let var1: string = 'var1'

    var1 += arg

    if (true) {
        var1 += arg
    }

    var1 += arg

    try {
        var1 += arg
    }
    catch (e) {
        var1 += arg
    }

    var1 += arg

    return var1 // keep tree strings due to intermadiate string usage and try/catch
}

function hybridStringTest16(arg: string, b: boolean): string {
    let var1: string = 'var1'

    var1 += arg

    if (true) {
        var1 += arg
    }

    var1 += arg

    if (b) {
        var1 += arg
        var1 += 't'
    }
    else {
        var1 += arg
        var1 += 'f'
    }

    var1 += arg

    return var1 // switch to SB due to long chain concatenation
}

function hybridStringTest17(arg: string, b: boolean, j: int): string {
    let var1: string = 'var1'

    var1 += arg
    const zero: number = 0
    const one: number = 1

    if (true) {
        for (let i: number = 0; i < 10; i++) {
            var1 += arg
            if ((i & one) == zero) {
                var1 += 'i'
            }
        }
    }

    var1 += arg

    if (b) {
        for (let i: number = 0; i < 10; i++) {
            var1 += arg
            if ((i & one) == zero) {
                var1 += 'i'
            }
        }
        var1 += 't'
    }
    else {
        for (let i = 0; i < 10; i++) {
            var1 += arg
            if ((i & one) != zero) {
                var1 += 'i'
            }
        }
        var1 += 'f'
    }

    switch (j)
    {
        case 0: var1 += '0'; break;
        case 1: var1 += '1'; break;
        default: var1 += 'def'; break;
    }

    var1 += arg

    return var1 // switch to SB due to long chain concatenation
}

function hybridStringTest18(arg: string): string {
    let var1: string = 'var1'
    let var2: string = 'var2'
    const const1: string = 'const1'

    let result: string = ''
    let tmp0 = var2 + const1
    result += tmp0 + arg;
    result += var1;

    return result; // switch to SB due to long chain concatenation
}

function hybridStringTest19(arg: string): string {
    let var1: string = 'var1'
    const const1: string = 'const1'

    return var1 + arg + const1; // switch to SB due to long chain concatenation
}

function main(): void {
    arktest.assertEQ(hybridStringTest00('a'), 'var2const1const2var1var2const1a');
    arktest.assertEQ(hybridStringTest01('a'), 'var1aaaaaaaaaaaa');
    arktest.assertEQ(hybridStringTest02('a'), 'var1aaaaaaa');
    arktest.assertEQ(hybridStringTest03('a'), 'var1aaaaaaaa');
    arktest.assertEQ(hybridStringTest04('a'), 'var1aaaaa');
    arktest.assertEQ(hybridStringTest05('a'), 118);
    arktest.assertEQ(hybridStringTest06('a'), 'var2const1const2var1a');
    arktest.assertEQ(hybridStringTest07('a'), 'var2const1const2var1a');
    arktest.assertEQ(hybridStringTest08('a'), 'var2const1const2var1a');
    arktest.assertEQ(hybridStringTest09('a'), 'var2const1aconst2var1aconst2var1aa');
    arktest.assertEQ(hybridStringTest10('a'), 'var2const1var1aconst2var2const1');
    arktest.assertEQ(hybridStringTest11('a'), 'var1aaaaavar1aaaaaaa');
    arktest.assertEQ(hybridStringTest12('a'), 'var1aaaaavar1aaaaa');
    arktest.assertEQ(hybridStringTest13('a'), 'var2const1var1aconst2var2const1var2const1var1var2const1var1a');
    arktest.assertEQ(hybridStringTest14('a'), 'var1aaaavar1aaaavar1aaavar1aavar1a');
    arktest.assertEQ(hybridStringTest15('a'), 'var1aaaaa');

    arktest.assertEQ(hybridStringTest16('a', true), 'var1aaaata');
    arktest.assertEQ(hybridStringTest16('a', false), 'var1aaaafa');

    arktest.assertEQ(hybridStringTest17('a', false, 0), 'var1aaiaaiaaiaaiaaiaaaaiaaiaaiaaiaaif0a');
    arktest.assertEQ(hybridStringTest17('a', true, 1), 'var1aaiaaiaaiaaiaaiaaaiaaiaaiaaiaaiat1a');
    arktest.assertEQ(hybridStringTest17('a', true, 2), 'var1aaiaaiaaiaaiaaiaaaiaaiaaiaaiaaiatdefa');

    arktest.assertEQ(hybridStringTest18('arg'), 'var2const1argvar1');
    arktest.assertEQ(hybridStringTest19('arg'), 'var1argconst1');
}
