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

//! CHECKER     StringBuilder length field access AOT unoptimized
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*StringCalculation00' --compiler-inlining=false --compiler-check-final=false --compiler-simplify-string-builder=false"
//!
//! METHOD      "ets_stringbuilder_length.ETSGLOBAL::StringCalculation00"
//! PASS_AFTER  "OptimizeStringConcat"
//! PASS_AFTER_NEXT "Cleanup"
//! INST_COUNT  /StringBuilder::<ctor>/,3
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,3
//! INST_COUNT  /LenArray/,4
//! INST_NOT    /LoadObject \d+ std.core.StringBuilder.length/
//!
//! RUN         entry: "ets_stringbuilder_length.ETSGLOBAL::StringCalculation00", result: 0

//! CHECKER     StringBuilder length field access AOT optimized
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*StringCalculation00' --compiler-inlining=false --compiler-check-final=false"
//!
//! METHOD      "ets_stringbuilder_length.ETSGLOBAL::StringCalculation00"
//! PASS_AFTER  "OptimizeStringConcat"
//! INST_COUNT  /StringBuilder::<ctor>/,3
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,3
//! INST_COUNT  /LenArray/,4
//! INST_NOT    /LoadObject \d+ std.core.StringBuilder.length/
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//! INST_COUNT  /LenArray/,2
//! INST        /LoadObject \d+ std.core.StringBuilder.length/
//!
//! RUN         entry: "ets_stringbuilder_length.ETSGLOBAL::StringCalculation00", result: 0

function GenerateFakeRandomInteger(): Int32Array {
  let resource: Int32Array = new Int32Array([12, 43, 56, 76, 89, 54, 45, 32, 35, 47, 46, 44, 21, 37, 84]);
  return resource;
}

function StringCalculation00() {
  let count: int = 3000000 / 1000;
  let str1: string = "h";
  let res: string = "11";

  let resources: Int32Array = GenerateFakeRandomInteger();
  for (let i: int = 0; i < count; i++) {
    if (resources[i % res.length % 15].toInt() > resources[(i + resources[i % 15].toInt()) % res.length % 15].toInt()) {
      res += str1 + i;
    } else {
      res += str1;
    }
  }

  console.log(res.length)
  return res.length.toInt() - 6650
}


//! CHECKER     StringBuilder without length field AOT
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*StringCalculation01' --compiler-inlining=false --compiler-check-final=false"
//!
//! METHOD      "ets_stringbuilder_length.ETSGLOBAL::StringCalculation01"
//! PASS_AFTER  "OptimizeStringConcat"
//! INST_COUNT  /StringBuilder::<ctor>/,3
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,3
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//!
//! RUN         entry: "ets_stringbuilder_length.ETSGLOBAL::StringCalculation00", result: 0

function StringCalculation01() {
  let count: int = 3000000 / 1000;
  let str1: string = "h";
  let res: string = "11";

  let resources: Int32Array = GenerateFakeRandomInteger();
  for (let i: int = 0; i < count; i++) {
    if (resources[i % 15].toInt() > resources[(i + resources[i % 15].toInt()) % 15].toInt()) {
      res += str1 + i;
    } else {
      res += str1;
    }
  }

  return res.length
}

//! CHECKER     StringBuilder length field access at nested unoptimized loop AOT
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*StringCalculation02' --compiler-inlining=false --compiler-check-final=false --compiler-simplify-string-builder=false"
//! RUN         entry: "ets_stringbuilder_length.ETSGLOBAL::StringCalculation02", result: 0

//! CHECKER     StringBuilder length field access at nested loop AOT
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*StringCalculation02' --compiler-inlining=false --compiler-check-final=false"
//!
//! METHOD      "ets_stringbuilder_length.ETSGLOBAL::StringCalculation02"
//! PASS_AFTER  "OptimizeStringConcat"
//! INST_COUNT  /StringBuilder::<ctor>/,5
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,5
//! INST_COUNT  /LenArray/,4
//! INST_NOT    /LoadObject \d+ std.core.StringBuilder.length/
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,2
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,3
//! INST_COUNT  /LenArray/,2
//! INST        /LoadObject \d+ std.core.StringBuilder.length/
//!
//! RUN         entry: "ets_stringbuilder_length.ETSGLOBAL::StringCalculation02", result: 0

function StringCalculation02() {
    let outterCount: int = 30;
    let innerCount: int = 100;
    let str1: string = "h";
    let str2: string = "j";
    let res: string = "11";
  
    let resources: Int32Array = GenerateFakeRandomInteger();
    for (let i: int = 0; i < outterCount; i++) {
        res += str2 + i;
        for (let j: int = 0; j < innerCount; j++) {
          if (resources[j % res.length % 15].toInt() > resources[(j + resources[i % 15].toInt()) % res.length % 15].toInt()) {
            res += str1 + j;
          } else {
            res += str1;
          }
        }
    }
  
    console.log(res.length)
    return res.length.toInt() - 5761
}

//! CHECKER     Concat loop with length access unoptimized loop AOT
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*StringCalculation03|.*reuse_concat_loop1' --compiler-inlining=false --compiler-check-final=false --compiler-simplify-string-builder=false"
//! RUN         entry: "ets_stringbuilder_length.ETSGLOBAL::StringCalculation03", result: 0

//! CHECKER     Concat loop with length access loop AOT CHA inline
//! SKIP_IF     @architecture == "arm32"
//! RUN_PAOC    options: "--compiler-regex='.*StringCalculation03|.*reuse_concat_loop1' --compiler-inlining=true --paoc-use-cha=true --compiler-check-final=false"
//!
//! METHOD      "ets_stringbuilder_length.ETSGLOBAL::reuse_concat_loop1"
//! PASS_AFTER  "OptimizeStringConcat"
//! INST_COUNT  /StringBuilder::<ctor>/,2
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,2
//! INST_COUNT  /LenArray/,1
//! PASS_AFTER  "SimplifyStringBuilder"
//! INST_COUNT  /StringBuilder::<ctor>/,1
//! INST_COUNT  /Intrinsic.StdCoreSbToString/,1
//! INST_NOT    /LenArray/
//! INST_COUNT  /LoadObject \d+ std.core.StringBuilder.length/,1
//!
//! RUN         entry: "ets_stringbuilder_length.ETSGLOBAL::StringCalculation03", result: 0

function reuse_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.length;                      // partially applied
        str += a;                               // two SBs merged into one
    }
    return str;
}

function StringCalculation03() {
    arktest.assertEQ(reuse_concat_loop1("a", 4), "0a2a4a6a", "Wrong result at reuse_concat_loop1");
}
