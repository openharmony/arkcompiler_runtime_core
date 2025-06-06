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

//! CHECKER      Check JIT for ZeroCheck after ChecksElimination
//! RUN          force_jit: true, options: "--compiler-hotness-threshold=0 --compiler-profiling-threshold=0 --compiler-enable-osr=false", entry: "ets_overflow.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_overflow.ETSGLOBAL::__noinline__overflow,.*,COMPILED/
//! METHOD       "ets_overflow.ETSGLOBAL::__noinline__overflow"
//! PASS_AFTER   "ChecksElimination"
//! INST         "ZeroCheck"

//! CHECKER      Check AOT for ZeroCheck after ChecksElimination
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex=ets_overflow.ETSGLOBAL::__noinline__overflow"
//! METHOD       "ets_overflow.ETSGLOBAL::__noinline__overflow"
//! PASS_AFTER   "ChecksElimination"
//! INST         "ZeroCheck"

function __noinline__overflow(): int {
  try {
    let i0: int = 2147483647;
    for (let i: int = 0; i < 1000; i++) {
      if (i % i0 == 0) {
        i0 += 2147483647;
      } else {
        i0 += 2;
      }
    }
  } catch (expected: ArithmeticError) {
    return 0;
  }
  return 1;
}

function main(): int {
  let res :int = 0;
  for (let i:int = 0; i < 25; ++i) {
    res += __noinline__overflow();
  }
  console.log(res);
  return res;
}

