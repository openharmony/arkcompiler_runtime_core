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

//! CHECKER      Build boolean StObjByName emitted for a union property (JIT)
//! SKIP_IF      @architecture == "arm32"
//! RUN          force_jit: true, options: "--compiler-regex='.*__noinline__storeBoolean.*'",
//!              entry: "ets_stobj_by_name_bool.ETSGLOBAL::main"
//! METHOD       "ets_stobj_by_name_bool.ETSGLOBAL::__noinline__storeBoolean"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStObjByNameI8/

//! CHECKER      Build boolean StObjByName emitted for a union property (AOT)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*__noinline__storeBoolean.*'"
//! METHOD       "ets_stobj_by_name_bool.ETSGLOBAL::__noinline__storeBoolean"
//! PASS_AFTER   "IrBuilder"
//! INST         /Intrinsic.CompilerEtsStObjByNameI8/
//! RUN          entry: "ets_stobj_by_name_bool.ETSGLOBAL::main"

class A {
    public value: boolean = false
}

class B {
    public value: boolean = false
}

function __noinline__storeBoolean(obj: A | B, value: boolean): void {
    obj.value = value
}

function main(): void {
    const a = new A()
    const b = new B()
    __noinline__storeBoolean(a, true)
    __noinline__storeBoolean(b, true)
    arktest.assertTrue(a.value)
    arktest.assertTrue(b.value)
}
