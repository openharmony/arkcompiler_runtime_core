/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

//! CHECKER      Try Catch Resolving JIT Optimize with default catch on the same level
//! RUN          force_jit: true, options: "--compiler-hotness-threshold=20 --compiler-profiling-threshold=0 --compiler-enable-osr=false --compiler-regex=MultipleCatch::__noinline__multipleCatch", entry: "ETSGLOBAL::main"
//! EVENT        /Compilation,MultipleCatch::__noinline__multipleCatch,.*,COMPILED/
//! METHOD       "MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",3
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"

//! CHECKER      Try Catch Resolving AOT Optimize with default catch on the same level
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex=MultipleCatch::__noinline__multipleCatch"
//! METHOD       "MultipleCatch::__noinline__multipleCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",3
//! PASS_AFTER   "TryCatchResolving"
//! INST_COUNT   "Throw",3

class Exception0 extends Exception {}
class Exception1 extends Exception {}
class Exception2 extends Exception {}

class MultipleCatch {
  __noinline__multipleCatch() : int {
    let res : int = 0;
    for (let i : int = 0; i < 16; i++) {
      let k : int = i % 3;
      try {
        switch (k) {
          case 0:
            throw new Exception0();
          case 1:
            throw new Exception1();
          default:
            throw new Exception2();
        }
      } catch (e: Exception0) {
        res += 1;
      } catch (e: Exception1) {
        res += 2;
      } catch (e) {
        res += 3;
      }
    }
    return res;
  }
}

let mc = new MultipleCatch();

function main(): int {
    let res :int = 0;
    for (let i:int = 0; i < 10; ++i) {
        res += mc.__noinline__multipleCatch();
    }
    return res - 310;
}
