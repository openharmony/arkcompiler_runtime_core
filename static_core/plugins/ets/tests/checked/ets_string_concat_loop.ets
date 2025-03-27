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

//! CHECKER       AOT IR Builder, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! RUN           entry: "ets_string_concat_loop.ETSGLOBAL::main"
//! RUN_PAOC      options: "--compiler-regex='.*concat_loop[0-9]*' --compiler-inlining=false"
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /head, loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /head, loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::equivalent_concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop8"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,10
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "Codegen"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::nested_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 3/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 3/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 2/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,14
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,3
//! INST_COUNT    /Intrinsic.StdCoreStringConcat4/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,6
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,5
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,10
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,5
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,4
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,5
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,5
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop10"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop11"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switch_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,8
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switch_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,8
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::args_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2

//! CHECKER       JIT IR Builder, check String concatenation loop
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-regex='.*concat_loop[0-9]+.*' --compiler-inlining=false", entry: "ets_string_concat_loop.ETSGLOBAL::main"
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /head, loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /head, loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::equivalent_concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::concat_loop8"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,10
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! PASS_AFTER    "ChecksElimination"
//! IN_BLOCK      /loop/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "Codegen"
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::nested_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 3/
//! INST          /StringBuilder::<ctor>/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST          /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST          /StringBuilder::<ctor>/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 3/
//! INST          /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 2/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! PASS_AFTER    "SimplifyStringBuilder"
//! INST          /Intrinsic.StdCoreSbToString/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,14
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,3
//! INST_COUNT    /Intrinsic.StdCoreStringConcat4/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,6
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//! INST_NOT      /Intrinsic.StdCoreSbAppendString/
//! INST_NOT      /Intrinsic.StdCoreSbAppendInt/
//! INST_NOT      /Intrinsic.StdCoreSbToString/
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::try_catch_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,5
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,10
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,5
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreStringConcat2/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,4
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,5
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,5
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop6"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop7"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,2
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop8"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop9"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop10"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::if_concat_loop11"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /prehead/
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,1
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1, depth 1, bc: 0x00000017/
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switch_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,8
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::switch_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,6
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,8
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,6
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,1
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString /,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop2"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop3"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,4
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_NOT      /StringBuilder::<ctor>/
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop4"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop5"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,3
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString2/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//!
//! METHOD        "ets_string_concat_loop.ETSGLOBAL::args_concat_loop0"
//! PASS_BEFORE   "BranchElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2
//! PASS_AFTER    "ChecksElimination"
//! INST_COUNT    /StringBuilder::<ctor>/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendString/,2
//! INST_COUNT    /Intrinsic.StdCoreSbAppendInt/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendBool/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendChar/,1
//! INST_COUNT    /Intrinsic.StdCoreSbAppendDouble/,1
//! INST_COUNT    /Intrinsic.StdCoreSbToString/,2

//! CHECKER       Invalid save states JIT (#I932LA/15833)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-inlining=false --compiler-regex='.*bugfix_concat_loop1'", entry: "ets_string_concat_loop.ETSGLOBAL::main"
//!
//! METHOD        "bugfix_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1/
//! INST          /StringBuilder::<ctor>/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /StringBuilder::<ctor>/

//! CHECKER       Invalid save states AOT (#I932LA/15833)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*bugfix_concat_loop1'"
//!
//! METHOD        "bugfix_concat_loop1"
//! PASS_BEFORE   "BranchElimination"
//! IN_BLOCK      /loop 1/
//! INST          /StringBuilder::<ctor>/
//! PASS_AFTER    "SimplifyStringBuilder"
//! IN_BLOCK      /loop 1/
//! INST_NOT      /StringBuilder::<ctor>/

function concat_loop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        str = str + a;                              // applied
    return str;
}

function concat_loop1(a: String, n: int): String {
    let str: String = "";
    while (--n >= 0)
        str += a;                                   // applied
    return str;
}

function concat_loop2(a: String, n: int): String {
    let str: String = "";
    do
        str += a;                                   // applied
    while (--n > 0);
    return str;
}

function concat_loop3(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i, str += a);          // applied
    return str;
}

function concat_loop4(strings: String[]): String {
    let str: String = "";
    for (let i = 0; i < strings.length; ++i) {
        str += strings[i];                          // applied
    }
    return str;
}

function concat_loop5(strings: String[]): String {
    let str: String = "";
    for (let i = 0; i < strings.length; ++i) {
        str += strings[0] + strings[i];             // applied
    }
    return str;
}

function concat_loop7(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                                   // applied
        str += b;                                   // applied
    }
    return str;
}

function equivalent_concat_loop7(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        let sb0 = new StringBuilder();              // applied
        sb0.append(str);
        sb0.append(a);
        let tmp1 = sb0.toString();

        let sb1 = new StringBuilder();              // applied
        sb1.append(tmp1);
        sb1.append(b);
        str = sb1.toString();
    }
    return str;
}

function concat_loop8(a: String, b: String, n: int): String {
    let a1b: String = "";
    let b2a: String = "";
    for (let i = 0; i < n; ++i) {
        a1b += a + 1;                               // applied
        a1b += b;                                   // applied

        b2a += b + 2;                               // applied
        b2a += a;                                   // applied
    }
    return a1b + "." + b2a;
}

function nested_concat_loop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        for (let j = 0; j < n; ++j)
            for (let k = 0; k < n; ++k)
                str = str + a;                      // applied, instructions moved out of outermost loop
    return str;
}

function try_catch_concat_loop0(a: String, n: int): String {
    let str_try: String = "";
    let str_catch: String = "";
    let str_finally: String = "";
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                                   // not applied due to loop containing try-catch-finally blocks
                                                    // replaced by concat intrinsic
        try {
            str_try += a;                           // not applied due to being inside try-block
                                                    // replaced by concat intrinsic
        }
        catch(ex) {
            str_catch += a;                         // not applied due to being inside catch-block
                                                    // replaced by concat intrinsic
        }
        finally {
            str_finally += a;                       // not applied due to being inside finally-block
                                                    // replaced by concat intrinsic
        }
    }
    return str + str_try + str_catch + str_finally; // replaced by concat intrinsic
}

function try_catch_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        try {
            str = str + a;                          // not applied
                                                    // replaced by concat intrinsic
        }
        catch(ex) {
            str = str + a;                          // not applied
                                                    // replaced by concat intrinsic
        }
    return str;
}

function try_catch_concat_loop2(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        try {
            str = str + a;                          // not applied
                                                    // replaced by concat intrinsic
        }
        finally {
            str = str + a;                          // partially applied
                                                    // SB merged with the one inside try-block
        }
    return str;
}

function try_catch_concat_loop3(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        try {
            for (let j = 0; j < n; ++j) {
                str += a;                       // not applied due to loop containing try-catch-finally blocks
                                                // replaced by concat intrinsic
            }
        }
        catch(ex) {
            for (let j = 0; j < n; ++j) {
                str += a;                       // not applied
                                                // replaced by concat intrinsic
            }
        }
    return str;
}

function try_catch_concat_loop4(a: String, n: int): String {
    let str: String = "";
    try {
        for (let i = 0; i < n; ++i)
            str += a;                           // not applied due to loop containing try-catch-finally blocks
                                                // replaced by concat intrinsic
    }
    catch(ex) {}
    return str;
}

function try_catch_concat_loop5(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                   // not applied due to loop containing try-catch-finally blocks
                                    // replaced by concat intrinsic
        try {
            str += a;               // partially applied
                                    // SBs inside try, catch, and finally blocks merged into one
        }
        catch(ex) {
            str += a;               // partially applied
                                    // SBs inside try, catch, and finally blocks merged into one
        }
        finally {
            str += a;               // partially applied
                                    // SBs inside try, catch, and finally blocks merged into one
        }
    }
    return str;
}

function if_concat_loop0(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
    }
    return str;
}

function if_concat_loop2(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop3(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop4(b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop5(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2)
            str += 1;                           // applied
    }
    return str;
}

function if_concat_loop6(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
    }
    return str;
}

function if_concat_loop7(n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
        else
            str += 2;                           // applied
    }
    return str;
}

function if_concat_loop8(b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop9(b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
        str += b;                               // applied
    }
    return str;
}

function if_concat_loop10(n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2)
            str += 1;                           // applied
    }
    return str;
}

function if_concat_loop11(n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        if (i < n/2) {
        }
        else
            str += 2;                           // applied
    }
    return str;
}

function switch_concat_loop0(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        switch (i) {
            case 0:
                str += 0;                       // applied
                break;
            case 1:
                str += 1;                       // applied
                break;
            case 2:
                str += 2;                       // applied
                break;
            default:
                str += i;                       // applied
                break;
        }
        str += b;                               // applied
    }
    return str;
}

function switch_concat_loop1(a: String, b: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // applied
        switch (i) {
            case 0:
                str += 0;                       // applied
            case 1:
                str += 1;                       // applied
            case 2:
                str += 2;                       // applied
            default:
                str += i;                       // applied
        }
        str += b;                               // applied
    }
    return str;
}

function reuse_concat_loop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str;                             // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function reuse_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.length;                      // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function reuse_concat_loop2(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.getLength();                 // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function reuse_concat_loop3(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // not applied
        str += str;                             // unsupported string concatenation pattern
    }
    return str;
}

function reuse_concat_loop4(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // partially applied
        str += str.length;                      // two SBs merged into one
    }
    return str;
}

function reuse_concat_loop5(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += a;                               // partially applied
        str += str.getLength();                 // two SBs merged into one
    }
    return str;
}

function args_concat_loop0(x: double, y: boolean, ...p: char[]): string
{
    let res: string = "" + x + y + p.length;    // not replaced by intrinsic due to concatenating float, bool, int
    for (let ch of p) res += ch;                // not applied due to exit block not dominated by preheader
    return res;
}

function bugfix_concat_loop0(): String {
    let res = ""
    for (let i = 0; i < 1; ++i)
    {
        if (i < 0) {
            res += "0"
        }
        else
        {
            res += "12345"
            res += "678"
        }
    }
    return res
}

function bugfix_concat_loop1(size: int, charset: string): string {
    let res: string = new string()

    switch (charset) {
        case "ISO-8859-1":
            for (let i = 0; i < size; ++i)
                res += string.fromCharCode(0x43 as number)
        break
        case "UTF-16":
            for (let i = 0; i < size; ++i)
                res += string.fromCharCode(0xD799 as number)
        break
    }

    return res
}

function main() {
    assertEQ(concat_loop0("a", 4), "aaaa", "Wrong result at concat_loop0");
    assertEQ(concat_loop1("a", 4), "aaaa", "Wrong result at concat_loop1");
    assertEQ(concat_loop2("a", 4), "aaaa", "Wrong result at concat_loop2");
    assertEQ(concat_loop3("a", 4), "aaaa", "Wrong result at concat_loop3");
    assertEQ(concat_loop4(["a", "b", "c", "d"]), "abcd", "Wrong result at concat_loop4");
    assertEQ(concat_loop5(["a", "b", "c", "d"]), "aaabacad", "Wrong result at concat_loop5");
    assertEQ(concat_loop7("a", "b", 4), "abababab", "Wrong result at concat_loop7");
    assertEQ(equivalent_concat_loop7("a", "b", 4), "abababab", "Wrong result at equivalent_concat_loop7");
    assertEQ(concat_loop8("a", "b", 4), "a1ba1ba1ba1b.b2ab2ab2ab2a", "Wrong result at concat_loop8");
    assertEQ(nested_concat_loop0("a", 2), "aaaaaaaa", "Wrong result at nested_concat_loop0");
    assertEQ(try_catch_concat_loop0("a", 4), "aaaaaaaaaaaa", "Wrong result at try_catch_concat_loop0");
    assertEQ(try_catch_concat_loop1("a", 4), "aaaa", "Wrong result at try_catch_concat_loop1");
    assertEQ(try_catch_concat_loop2("a", 4), "aaaaaaaa", "Wrong result at try_catch_concat_loop2");
    assertEQ(try_catch_concat_loop3("a", 2), "aaaa", "Wrong result at try_catch_concat_loop3");
    assertEQ(try_catch_concat_loop4("a", 4), "aaaa", "Wrong result at try_catch_concat_loop4");
    assertEQ(try_catch_concat_loop5("a", 4), "aaaaaaaaaaaa", "Wrong result at try_catch_concat_loop5");
    assertEQ(if_concat_loop0("a", "b", 4), "a1ba1ba2ba2b", "Wrong result at if_concat_loop0");
    assertEQ(if_concat_loop1("a", 4), "a1a1a2a2", "Wrong result at if_concat_loop1");
    assertEQ(if_concat_loop2("a", "b", 4), "a1ba1babab", "Wrong result at if_concat_loop2");
    assertEQ(if_concat_loop3("a", "b", 4), "ababa2ba2b", "Wrong result at if_concat_loop3");
    assertEQ(if_concat_loop4("b", 4), "1b1b2b2b", "Wrong result at if_concat_loop4");
    assertEQ(if_concat_loop5("a", 4), "a1a1aa", "Wrong result at if_concat_loop5");
    assertEQ(if_concat_loop6("a", 4), "aaa2a2", "Wrong result at if_concat_loop6");
    assertEQ(if_concat_loop7(4), "1122", "Wrong result at if_concat_loop7");
    assertEQ(if_concat_loop8("b", 4), "bb2b2b", "Wrong result at if_concat_loop8");
    assertEQ(if_concat_loop9("b", 4), "1b1bbb", "Wrong result at if_concat_loop9");
    assertEQ(if_concat_loop10(4), "11", "Wrong result at if_concat_loop10");
    assertEQ(if_concat_loop11(4), "22", "Wrong result at if_concat_loop11");
    assertEQ(switch_concat_loop0("a", "b", 4), "a0ba1ba2ba3b", "Wrong result at switch_concat_loop0");
    assertEQ(switch_concat_loop1("a", "b", 4), "a0120ba121ba22ba3b", "Wrong result at switch_concat_loop1");
    assertEQ(reuse_concat_loop0("a", 4), "aaaaaaaaaaaaaaa", "Wrong result at reuse_concat_loop0");
    assertEQ(reuse_concat_loop1("a", 4), "0a2a4a6a", "Wrong result at reuse_concat_loop1");
    assertEQ(reuse_concat_loop2("a", 4), "0a2a4a6a", "Wrong result at reuse_concat_loop2");
    assertEQ(reuse_concat_loop3("a", 4), "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "Wrong result at reuse_concat_loop3");
    assertEQ(reuse_concat_loop4("a", 4), "a1a3a5a7", "Wrong result at reuse_concat_loop4");
    assertEQ(reuse_concat_loop5("a", 4), "a1a3a5a7", "Wrong result at reuse_concat_loop5");
    assertEQ(args_concat_loop0(1.5, false, c'A', c'B'), "1.5false2AB", "Wrong result at args_concat_loop0");
    assertEQ(bugfix_concat_loop0(), "12345678", "Wrong result at bugfix_concat_loop0");
    assertEQ(bugfix_concat_loop1(4, "ISO-8859-1"), "CCCC", "Wrong result at bugfix_concat_loop1");
    assertEQ(bugfix_concat_loop1(4, "UTF-16"), "힙힙힙힙", "Wrong result at bugfix_concat_loop1");
}

//! CHECKER       Invalid save states AOT (I9NI8P/16707)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*bugfix_concat_loop2_test[0-9]+' --compiler-inlining=true"

//! CHECKER       Invalid save states JIT (I9NI8P/16707)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, entry: "ets_string_concat_loop.ETSGLOBAL::bugfix_concat_loop2"

const initial = "initial";
const expected = "expected";
const empty = "";

function bugfix_concat_loop2_test0(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        b += empty;
        c = b;
    }
    return c;
}

function bugfix_concat_loop2_test1(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        if (a == 1)
            b += empty;
        else
            c = b;
    }
    return c;
}

function bugfix_concat_loop2_test2(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        if (a == 1)
            b += empty;
        c = b;
    }
    return c;
}

function bugfix_concat_loop2_test3(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        b += empty;
        if (a == 0)
            c = b;
    }
    return c;
}

function bugfix_concat_loop2_test4(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        b += empty;
        if (a == 1) {}
        else
            c = b;
    }
    return c;
}

function bugfix_concat_loop2_test5(): string {
    let c = initial;
    let b = expected;
    for(let a = 0; a < 1; a++) {
        if (a == 1) {}
        else
            b += empty;
        c = b;
    }
    return c;
}

function bugfix_concat_loop2() {
    for(let i = 0; i < 5000; i++) {
        assertEQ(bugfix_concat_loop2_test0(), expected);
        assertEQ(bugfix_concat_loop2_test1(), expected);
        assertEQ(bugfix_concat_loop2_test2(), expected);
        assertEQ(bugfix_concat_loop2_test3(), expected);
        assertEQ(bugfix_concat_loop2_test4(), expected);
        assertEQ(bugfix_concat_loop2_test5(), expected);
    }
}

//! CHECKER       Invalid phi instruction AOT (IARN2S/19226)
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC      options: "--compiler-regex='.*bugfix_concat_loop3' --compiler-loop-unroll=false --compiler-inlining=true"

//! CHECKER       Invalid phi instruction JIT (IARN2S/19226)
//! SKIP_IF       @architecture == "arm32"
//! RUN           force_jit: true, options: "--compiler-loop-unroll=false", entry: "ets_string_concat_loop.ETSGLOBAL::bugfix_concat_loop3"

function bugfix_concat_loop3() {
    let str = "";
    let a2 : string[] = ["ddd"]
    let idx2 : string | boolean;
    for (idx2 of a2) {
        let c2 = idx2
        if (c2 instanceof string) {
            str += c2;
        }
    }

    assertEQ(str, "ddd");
}