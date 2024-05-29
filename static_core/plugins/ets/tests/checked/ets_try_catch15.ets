/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

//! CHECKER      Try Catch Resolving JIT IfImm is replaced by DeoptimizeIf 
//! RUN          force_jit: true, options: "--compiler-hotness-threshold=5 --compiler-profiling-threshold=0 --compiler-enable-osr=false --compiler-regex=ETSGLOBAL::__noinline__test", entry: "ETSGLOBAL::main"
//! EVENT        /Compilation,ETSGLOBAL::__noinline__test,.*,COMPILED/
//! METHOD       "ETSGLOBAL::__noinline__test"
//! PASS_AFTER   "Inline"
//! INST_COUNT   "Throw", 3
//! PASS_AFTER   "TryCatchResolving"
//! INST_COUNT   "Throw", 1
//! INST         "DeoptimizeIf"

//! CHECKER      Try Catch Resolving AOT IfImm is not replaced by DeoptimizeIf
//! SKIP_IF       @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex=ETSGLOBAL::__noinline__test"
//! METHOD       "ETSGLOBAL::__noinline__test"
//! PASS_AFTER   "Inline"
//! INST         "Throw"
//! PASS_AFTER   "TryCatchResolving"
//! INST         "Throw"
//! INST_NOT     "DeoptimizeIf"

class Ex extends Exception {}
let ex: Ex = new Ex()

class A {
    t: string = ""
    getType() : string {
        return this.t
    }
    constructor(p: int) throws { if (p == 0) throw ex; else this.t = "int" }
}

function __noinline__test() {
    try {
        assert new A(-1).getType() == "int"
    } catch (e) {
        assert false: "Unexpected exception"
    }
}

function main() {
    for (let i:int = 1; i < 20; ++i) {
       __noinline__test();
    }
}