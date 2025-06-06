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

//! CHECKER      Try Catch Resolving JIT IfImm is replaced by DeoptimizeIf and check DeoptimizeIf branch
//! RUN          force_jit: true, options: "--compiler-hotness-threshold=5 --compiler-profiling-threshold=0 --compiler-enable-osr=false --compiler-regex=ets_try_catch12.IfCatch::__noinline__ifCatch", entry: "ets_try_catch12.ETSGLOBAL::main"
//! EVENT        /Compilation,ets_try_catch12.IfCatch::__noinline__ifCatch,.*,COMPILED/
//! METHOD       "ets_try_catch12.IfCatch::__noinline__ifCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"
//! INST         "DeoptimizeIf"

//! CHECKER      Try Catch Resolving AOT IfImm is not replaced by DeoptimizeIf
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex=ets_try_catch12.IfCatch::__noinline__ifCatch"
//! METHOD       "ets_try_catch12.IfCatch::__noinline__ifCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_COUNT   "Throw",2
//! INST_NOT     "DeoptimizeIf"

//! CHECKER      Try Catch Resolving AOT+PGO both throws are optimized (profile is written at the end)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PGO_PROF entry: "ets_try_catch12.ETSGLOBAL::main"
//! RUN_PGO_PAOC options: "--compiler-regex=ets_try_catch12.IfCatch::__noinline__ifCatch"
//! METHOD       "ets_try_catch12.IfCatch::__noinline__ifCatch"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw",2
//! PASS_AFTER   "TryCatchResolving"
//! INST_NOT     "Throw"
//! INST_NOT     "DeoptimizeIf"

class Exception0 extends Exception {}
class Exception1 extends Exception {}

class IfCatch {

  __noinline__inc(a: int, i: int): int {
    return a + i;
  }

  __noinline__ifCatch(k: int) : int {
    let res : int = 0;
    for (let i : int = 0; i < 16; i++) {
      try {
        if (k == 0) {
          res = this.__noinline__inc(res, 1);
          throw new Exception0();
        } else {
          res = this.__noinline__inc(res, 2);
          throw new Exception1();
        }
      } catch (e: Exception0) {
        res = this.__noinline__inc(res, 1);
      } catch (e) {
        res = this.__noinline__inc(res, 2);
      }
    }
    return res;
  }
}

let ic = new IfCatch();

function main(): int {
  let res :int = 0;
  for (let i:int = 1; i < 20; ++i) {
    res += ic.__noinline__ifCatch(i);
  }
  res += ic.__noinline__ifCatch(0);
  console.log(res);
  return res - 1248;
}
