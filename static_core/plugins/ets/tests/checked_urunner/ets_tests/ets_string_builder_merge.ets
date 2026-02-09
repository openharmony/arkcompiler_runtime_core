/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
//! RUN_FRONTEND  options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=false"

//! CHECKER       AOT IR Builder, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*[Cc]hainConcat[0-9]*' --compiler-inlining=false"
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! INST_COUNT    "Intrinsic.StdCoreSbToString",5
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",7
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",7
//! INST_COUNT    "Intrinsic.StdCoreSbToString",7
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",7
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",9
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",9
//! INST_COUNT    "Intrinsic.StdCoreSbToString",9
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",9
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",14
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",14
//! INST_COUNT    "Intrinsic.StdCoreSbToString",14
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",12
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supportedChainConcat5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",2
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supportedChainConcat6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",37
//! INST_COUNT    "Intrinsic.StdCoreSbAppendBool",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendShort",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendInt",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendLong",2
//! INST_COUNT    "std.core.StringBuilder::append",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",22
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",2
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedAppendChainConcat7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedConstructorChainConcat8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedUsageChainConcat9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat10"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat11"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",3
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat12"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat13"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partiallySupportedChainConcat14"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supportedChainConcat15"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",1
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partiallySupportedChainConcat16"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedUsageChainConcat17"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedUsageChainConcat18"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! RUN           entry: "ets_string_builder_merge.ETSGLOBAL::main"

//! CHECKER       JIT IR Builder, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*[Cc]hainConcat[0-9]+.*' --compiler-inlining=false",
//!               entry: "ets_string_builder_merge.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",5
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! INST_COUNT    "Intrinsic.StdCoreSbToString",5
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",5
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",7
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",7
//! INST_COUNT    "Intrinsic.StdCoreSbToString",7
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",7
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",9
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",9
//! INST_COUNT    "Intrinsic.StdCoreSbToString",9
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",9
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",14
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",14
//! INST_COUNT    "Intrinsic.StdCoreSbToString",14
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",12
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supportedChainConcat5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",2
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supportedChainConcat6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",37
//! INST_COUNT    "Intrinsic.StdCoreSbAppendBool",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendChar",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendByte",4
//! INST_COUNT    "Intrinsic.StdCoreSbAppendShort",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendInt",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendLong",2
//! INST_COUNT    "std.core.StringBuilder::append",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",22
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",2
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString4",2
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedAppendChainConcat7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedConstructorChainConcat8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedUsageChainConcat9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat10"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat11"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",3
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat12"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat13"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partiallySupportedChainConcat14"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supportedChainConcat15"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString3",1
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partiallySupportedChainConcat16"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString",3
//! INST_COUNT    "Intrinsic.StdCoreSbToString",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString2",1
//! INST_COUNT    "Intrinsic.StdCoreStringConcat2",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",0
//! PASS_AFTER    "Codegen"
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedUsageChainConcat17"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedUsageChainConcat18"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1
//! INST_COUNT    "String::concat",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    "Intrinsic.StdCoreSbAppendString ",2
//! INST_COUNT    "Intrinsic.StdCoreSbToString",1

// Each '+=' operation on strings in tests below is translated into StringBuilder + 2 append calls by frontend

function chainConcat0(a: String, n: int, b: String, m: int): String {
    let str: String = '';
    str += '_';
    for (let i = 0; i < n; ++i)
        str += a;
    str += '_';
    for (let i = 0; i < m; ++i)
        str += b;
    str += '_';
    return str;                                 // applied, 5 SBs merged into 1
}

function chainConcat1(a: String, n: int, b: String, m: int, c: String, l: int): String {
    let str: String = '';
    str += '_';
    for (let i = 0; i < n; ++i)
        str += a;
    str += '_';
    for (let i = 0; i < m; ++i)
        str += b;
    str += '_';
    for (let i = 0; i < l; ++i)
        str += c;
    str += '_';
    return str;                                 // applied, 7 SBs merged into 1
}

function chainConcat2(a: String, n: int, b: String, m: int, c: String, l: int, d: String, k: int): String {
    let str: String = '';
    str += '_';
    for (let i = 0; i < n; ++i)
        str += a;
    str += '_';
    for (let i = 0; i < m; ++i)
        str += b;
    str += '_';
    for (let i = 0; i < l; ++i)
        str += c;
    str += '_';
    for (let i = 0; i < k; ++i)
        str += d;
    str += '_';
    return str;                                 // applied, 9 SBs merged into 1
}

function chainConcat3(a: String, n: int, b: String, m: int, c: String, l: int, d: String, k: int): String {
    let str0: String = '';                      // two SBs chains '0' and '1'
    let str1: String = '';                      // for each string accumulator variable
    str0 += '_';                                //  0
    str1 += '_';                                //  |     1
    for (let i = 0; i < n; ++i) {               //  |     |
        str0 += a;                              //  0     |
        str1 += a;                              //  |     1
    }                                           //  |     |
    str0 += '_';                                //  0     |
    str1 += '_';                                //  |     1
    for (let i = 0; i < m; ++i) {               //  |    /
        str0 += b;                              //  0   /
        str1 += b;                              //  |  1
    }                                           //  | /
                                                //  |/
    str0 += str1;                               //  0 <- 'chain 1' connects to 'chain 0' here
    str0 += '_';                                //  0
                                                //  |
    for (let i = 0; i < l; ++i)                 //  |
        str0 += c;                              //  0
    str0 += '_';                                //  0
    for (let i = 0; i < k; ++i)                 //  |
        str0 += d;                              //  0
    str0 += '_';                                //  0
    return str0;    // applied, 14 SBs (10 from 'chain 0', and 4 from 'chain 1') merged into 2 SBs
}

// Set of SB based tests

const empty = '';
const space = ' ';

function supportedChainConcat5(): String {
    let sb0 = new StringBuilder();
    sb0.append(0)
       .append('1')
       .append(c'2');

    let sb1 = new StringBuilder();
    sb1.append(sb0.toString())
       .append(space)
       .append(0)
       .append('1')
       .append(c'2');

    return sb1.toString();                  // applied
}

function supportedChainConcat6(): String {
    let sb0 = new StringBuilder();
    let obj: Object = {};
    sb0.append(space)
       .append('abcΣΨΩ0123456789')
       .append(space)
       .append(true)
       .append(space)
       .append(false)
       .append(space)
       .append(c'A' as char)
       .append(space)
       .append(57 as byte)
       .append(space)
       .append(127 as byte)
       .append(space)
       .append(32767 as short)
       .append(space)
       .append(128934675 as int)
       .append(space)
       .append(701234987654321 as long)
       .append(space)
       .append(empty)
       .append('string')
       .append(space)
       .append(obj)
       .append(space)
       .append(1.0f as Float)
       .append(space)
       .append(1.0 as Double)
       .append(space)
       .append(1.0 as Number)
       .append(space);

    let sb1 = new StringBuilder();
    sb1.append(sb0.toString())
       .append(space)
       .append('abcΣΨΩ0123456789')
       .append(space)
       .append(true)
       .append(space)
       .append(false)
       .append(space)
       .append(c'A' as char)
       .append(space)
       .append(57 as byte)
       .append(space)
       .append(127 as byte)
       .append(space)
       .append(32767 as short)
       .append(space)
       .append(128934675 as int)
       .append(space)
       .append(701234987654321 as long)
       .append(space)
       .append(empty)
       .append('string')
       .append(space)
       .append(obj)
       .append(space)
       .append(1.0f as Float)
       .append(space)
       .append(1.0 as Double)
       .append(space)
       .append(1.0 as Number)
       .append(space);

    return sb1.toString();                  // applied
}

function unsupportedAppendChainConcat7(): String {
    let sb0 = new StringBuilder();
    sb0.append(0);

    let sb1 = new StringBuilder();
    sb1.append(0)
       .append(space)
       .append(sb0.toString());

    return sb1.toString();                  // not applied, sb0.toString() must be appended first
}

function unsupportedConstructorChainConcat8(): String {
    let sb0 = new StringBuilder();
    sb0.append(0);

    let sb1 = new StringBuilder('0');
    sb1.append(sb0.toString());

    return sb1.toString();                  // not applied, sb1 must be constructed via default constructor
}

function unsupportedUsageChainConcat9(): String {
    let sb0 = new StringBuilder();
    sb0.append(0);

    let sb1 = new StringBuilder();
    sb1.append(sb0.toString());

    sb0.append(space);

    return sb1.toString();                  // not applied, sb0 must not be used after sb0.toString() appended
}

function unsupportedChainConcat10(cond: boolean): String {
    let sb0 = new StringBuilder();
    sb0.append(0)
       .append('1')
       .append(c'2');

    let sb1 = new StringBuilder();
    if (cond)
        sb1.append(sb0.toString());
    else
        sb1.append('210');

    sb1.append(space)
       .append(0)
       .append('1')
       .append(c'2');

    return sb1.toString();                  // not applied, sb1 constructor and sb1.append(sb0.toString())
                                            // are not in the same BB
}

function unsupportedChainConcat11(cond: boolean): String {
    let sb0 = new StringBuilder('0');
    let sb1 = new StringBuilder('1');

    let sb = cond ? sb1 : sb0;

    let sb2 = new StringBuilder();
    sb2.append(sb.toString())
       .append(space)
       .append(2);

    return sb2.toString();                  // not applied, sb is Phi instruction
}

function __noinline__consume(obj: Object): void {}

function unsupportedChainConcat12(cond: boolean): String {
    let sb0 = new StringBuilder('0');
    let sb1 = new StringBuilder();
    let sb = cond ? sb1 : sb0;

    sb1.append(sb0.toString())
       .append(space)
       .append(1);

    __noinline__consume(sb);

    return sb1.toString();                  // not applied, sb0 must not be used after sb0.toString() appended
}

let sb0Global = new StringBuilder('0');
function unsupportedChainConcat13(): String {
    let sb1 = new StringBuilder();
    sb1.append(sb0Global.toString())
       .append(space)
       .append(1);

    return sb1.toString();                  // not applied, sb0 must be local instance
}

function partiallySupportedChainConcat14(a: String, b: String, c: String, d: String): String {
    let ab = a + b;
    let cd = c + d;                         // not applied, due to 'cd' appended to result is not the first append
    return ab + cd;                         // applied, 3 SBs merged into 1 SB + concat2
}

function supportedChainConcat15(a: String, b: String, c: String, d: String): String {
    let str = a + b;
    str += c;
    str += d;                                       // applied, 3 SBs merged into 1 SB
    return str;
}

function partiallySupportedChainConcat16(a: String, b: String, c: String, d: String): String {
    let abcd = a + b;
    abcd += c + d;                    // not applied, due to 'c+d' appended to result is not the first append
    return abcd;                      // applied, 3 SBs merged into 1 SB + concat2
}

function unsupportedUsageChainConcat17(a: String): String {
    let sb = new StringBuilder();
    sb.append(a);
    let result = sb.toString();
    sb.append(a);
    return result;                    // not applied, sb must not be used after sb.toString() appended
}

function unsupportedUsageChainConcat18(a: String): String {
    let sb = new StringBuilder();
    sb.append(a);
    let result = sb.toString();
    sb.append(a);
    return result.concat();           // not applied, sb must not be used after sb.toString() appended
}

//! CHECKER       AOT IR Builder, check String Builders merging (chainConcat19 only, with --compiler-inlining=true)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*chainConcat19' --compiler-inlining=true"
//! RUN           entry: "ets_string_builder_merge.ETSGLOBAL::main"

function chainConcat19(): void {
    const _ = '_';
    const a = 'a';
    const b = 'b';
    const c = 'c';

    let sbA = new StringBuilder();
    sbA.append(_)
    sbA.append(a)
    let _a = sbA.toString()

    let sbAb = new StringBuilder();
    sbAb.append(_a)
    sbAb.append(b)
    let _ab = sbAb.toString()

    let sbAbc = new StringBuilder();
    sbAbc.append(_ab)
    sbAbc.append(c)
    let _abc = sbAbc.toString()

    arktest.assertEQ(_a, '_a');
    arktest.assertEQ(_ab, '_ab');
    arktest.assertEQ(_abc, '_abc');
}

function main() {
    arktest.assertEQ(chainConcat0('a', 4, 'b', 3), '_aaaa_bbb_', 
                     'Wrong result at chainConcat0');
    arktest.assertEQ(chainConcat1('a', 4, 'b', 3, 'c', 5), '_aaaa_bbb_ccccc_',
                     'Wrong result at chainConcat1');
    arktest.assertEQ(chainConcat2('a', 4, 'b', 3, 'c', 5, 'd', 2), '_aaaa_bbb_ccccc_dd_',
                     'Wrong result at chainConcat2');
    arktest.assertEQ(chainConcat3('a', 4, 'b', 3, 'c', 5, 'd', 2), '_aaaa_bbb_aaaa_bbb_ccccc_dd_',
                     'Wrong result at chainConcat3');

    arktest.assertEQ(supportedChainConcat5(), '012 012',
                     'Wrong result at supportedChainConcat5');
    arktest.assertEQ(supportedChainConcat6(),
' abcΣΨΩ0123456789 true false A 57 127 32767 128934675 701234987654321 string [object Object] 1 1 1  ' +
'abcΣΨΩ0123456789 true false A 57 127 32767 128934675 701234987654321 string [object Object] 1 1 1 ',
                     'Wrong result at supportedChainConcat6');

    arktest.assertEQ(unsupportedAppendChainConcat7(), '0 0',
                     'Wrong result at unsupportedAppendChainConcat7');
    arktest.assertEQ(unsupportedConstructorChainConcat8(), '00',
                     'Wrong result at unsupportedConstructorChainConcat8');
    arktest.assertEQ(unsupportedUsageChainConcat9(), '0',
                     'Wrong result at unsupportedUsageChainConcat9');

    arktest.assertEQ(unsupportedChainConcat10(false), '210 012',
                     'Wrong result at unsupportedChainConcat10(false)');
    arktest.assertEQ(unsupportedChainConcat10(true), '012 012',
                     'Wrong result at unsupportedChainConcat10(true)');

    arktest.assertEQ(unsupportedChainConcat11(false), '0 2',
                     'Wrong result at unsupportedChainConcat11(false)');
    arktest.assertEQ(unsupportedChainConcat11(true), '1 2',
                     'Wrong result at unsupportedChainConcat11(true)');

    arktest.assertEQ(unsupportedChainConcat12(false), '0 1',
                     'Wrong result at unsupportedChainConcat12(false)');
    arktest.assertEQ(unsupportedChainConcat12(true), '0 1',
                     'Wrong result at unsupportedChainConcat12(true)');

    arktest.assertEQ(unsupportedChainConcat13(), '0 1',
                     'Wrong result at unsupportedChainConcat13()');

    arktest.assertEQ(partiallySupportedChainConcat14('ab', 'c', 'd', 'e'), 'abcde',
                     'Wrong result at partiallySupportedChainConcat14');
    arktest.assertEQ(supportedChainConcat15('ab', 'c', 'd', 'e'), 'abcde',
                     'Wrong result at supportedChainConcat15');
    arktest.assertEQ(partiallySupportedChainConcat16('ab', 'c', 'd', 'e'), 'abcde',
                     'Wrong result at partiallySupportedChainConcat16');
    arktest.assertEQ(unsupportedUsageChainConcat17('abc'), 'abc',
                     'Wrong result at unsupportedUsageChainConcat17');
    arktest.assertEQ(unsupportedUsageChainConcat18('abc'), 'abc',
                     'Wrong result at unsupportedUsageChainConcat18');

    chainConcat19();
}

//! CHECKER       Unsupported concat pattern AOT (IBLDF4/22718)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*bugfixChainConcat20' --compiler-inlining=true"
//! RUN           entry: "ets_string_builder_merge.ETSGLOBAL::main"

//! CHECKER       Unsupported concat pattern JIT (IBLDF4/22718)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true,
//!               options: "--compiler-regex='.*bugfixChainConcat20'",
//!               entry: "ets_string_builder_merge.ETSGLOBAL::bugfixChainConcat20"

function bugfixChainConcat20(): void {
    let a = 'a';
    let b = 'b';
    let abc = a + b + 'c';
    let abcd = abc + 'd';
    let abce = abc + 'e';

    arktest.assertEQ(abc, "abc");
    arktest.assertEQ(abcd, "abcd");
    arktest.assertEQ(abce, "abce");
}

//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN_BCO       options: "--ets-strings-concat=false --bco-compiler --compiler-simplify-string-builder=true"

//! CHECKER       BCO, check String Builders merging
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat0"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",5
//! INST_COUNT    /StringBuilder::append/,5
//! INST_COUNT    /StringBuilder::toString/,5
//! IN_BLOCK      /prehead \(loop 2\)/
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop 2, depth 1/
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /prehead \(loop 1\)/
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1/
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,5
//! INST_COUNT    /StringBuilder::toString/,1
//! IN_BLOCK      /prehead \(loop 2\)/
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,1
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 2, depth 1/
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead \(loop 1\)/
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST_NOT      /StringBuilder::toString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1/
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_COUNT    /StringBuilder::append/,1
//! INST_NOT      /StringBuilder::toString/
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat1"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",7
//! INST_COUNT    /StringBuilder::append/,7
//! INST_COUNT    /StringBuilder::toString/,7
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,7
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat2"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",9
//! INST_COUNT    /StringBuilder::append/,9
//! INST_COUNT    /StringBuilder::toString/,9
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,9
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::chainConcat3"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",14
//! INST_COUNT    /StringBuilder::append/,14
//! INST_COUNT    /StringBuilder::toString/,14
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    /StringBuilder::append/,14
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supportedChainConcat5"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    /StringBuilder::append/,8
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,7
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supportedChainConcat6"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    /StringBuilder::append/,61
//! INST_COUNT    /StringBuilder::toString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,60
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedAppendChainConcat7"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedConstructorChainConcat8"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedUsageChainConcat9"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat10"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat11"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",3
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat12"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedChainConcat13"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partiallySupportedChainConcat14"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::supportedChainConcat15"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::partiallySupportedChainConcat16"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",3
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,3
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",2
//! INST_COUNT    /StringBuilder::append/,3
//! INST_COUNT    /StringBuilder::toString/,2
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedUsageChainConcat17"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//!
//! METHOD        "ets_string_builder_merge.ETSGLOBAL::unsupportedUsageChainConcat18"
//! PASS_AFTER    "BranchElimination"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST_COUNT    "StringBuilder::<ctor>",1
//! INST_COUNT    /StringBuilder::append/,2
//! INST_COUNT    /StringBuilder::toString/,1

//! CHECKER       INTERP-IRTOC, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='ets_string_builder_merge.ETSGLOBAL::main' \
//!                         --compiler-enable-jit=false --interpreter-type=irtoc --compiler-enable-events \
//!                         --compiler-events-path=./events.csv",
//!               entry: "ets_string_builder_merge.ETSGLOBAL::main"

//! CHECKER       INTERP-CPP, check String Builders merging
//! SKIP_IF       @architecture == "arm32"
//! DEFINE_FRONTEND_OPTIONS            RUN_BCO reuse .abc
//! RUN           options: "--compiler-regex='ets_string_builder_merge.ETSGLOBAL::main' \
//!                         --compiler-enable-jit=false --interpreter-type=cpp --compiler-enable-events \
//!                         --compiler-events-path=./events.csv",
//!               entry: "ets_string_builder_merge.ETSGLOBAL::main"
